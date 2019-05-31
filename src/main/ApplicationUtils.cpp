// Copyright 2018 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "main/ApplicationUtils.h"
#include "bucket/Bucket.h"
#include "catchup/CatchupConfiguration.h"
#include "history/HistoryArchive.h"
#include "history/HistoryArchiveManager.h"
#include "historywork/GetHistoryArchiveStateWork.h"
#include "ledger/LedgerManager.h"
#include "main/ExternalQueue.h"
#include "main/Maintainer.h"
#include "main/PersistentState.h"
#include "main/StellarCoreVersion.h"
#include "util/Logging.h"
#include "work/WorkManager.h"
#include "transactions/MarkAccountOpFrame.h"

#include <lib/http/HttpClient.h>
#include <locale>

//bitcoin related headers
#include <config/bitcoin-config.h>
#include <chainparams.h>
#include <clientversion.h>
#include <compat.h>
#include <lib/bitcoin/src/fs.h>
#include <interfaces/chain.h>
#include <rpc/server.h>
#include <init.h>
#include <noui.h>
#include <shutdown.h>
#include <util/system.h>
#include <httpserver.h>
#include <httprpc.h>
#include <util/strencodings.h>
#include <walletinitinterface.h>
#include <wallet/rpcwallet.h>
#include <key_io.h>
#include <outputtype.h>
const std::function<std::string(const char*)> G_TRANSLATION_FUN = nullptr; 
namespace stellar
{

bool isNewDb = false;

namespace
{
bool
checkInitialized(Application::pointer app)
{
    try
    {
        // check to see if the state table exists
        app->getPersistentState().getState(PersistentState::kDatabaseSchema);
    }
    catch (...)
    {
        LOG(INFO) << "* ";
        LOG(INFO) << "* The database has not yet been initialized. Try --newdb";
        LOG(INFO) << "* ";
        return false;
    }
    return true;
}
}

int
runWithConfig(Config cfg)
{
    if (cfg.MANUAL_CLOSE)
    {
        // in manual close mode, we set FORCE_SCP
        // so that the node starts fully in sync
        // (this is to avoid to force scp all the time when testing)
        cfg.FORCE_SCP = true;
    }

    LOG(INFO) << "Starting stellar-core " << STELLAR_CORE_VERSION;
    VirtualClock clock(VirtualClock::REAL_TIME);
    Application::pointer app;
    try
    {
        app = Application::create(clock, cfg, false);

        if (!checkInitialized(app))
        {
            return 0;
        }
        else
        {
            if (!app->getHistoryArchiveManager().checkSensibleConfig())
            {
                return 1;
            }
            if (cfg.ARTIFICIALLY_ACCELERATE_TIME_FOR_TESTING)
            {
                LOG(WARNING) << "Artificial acceleration of time enabled "
                             << "(for testing only)";
            }

            app->start();

            app->applyCfgCommands();
        }
    }
    catch (std::exception& e)
    {
        LOG(FATAL) << "Got an exception: " << e.what();
        return 1;
    }
    auto& io = clock.getIOService();
    asio::io_service::work mainWork(io);
    while (!io.stopped())
    {
        clock.crank();
    }
    return 0;
}

void
httpCommand(std::string const& command, unsigned short port)
{
    std::string ret;
    std::ostringstream path;

    path << "/";
    bool gotCommand = false;

    std::locale loc("C");

    for (auto const& c : command)
    {
        if (gotCommand)
        {
            if (std::isalnum(c, loc))
            {
                path << c;
            }
            else
            {
                path << '%' << std::hex << std::setw(2) << std::setfill('0')
                     << (unsigned int)c;
            }
        }
        else
        {
            path << c;
            if (c == '?')
            {
                gotCommand = true;
            }
        }
    }

    int code = http_request("127.0.0.1", path.str(), port, ret);
    if (code == 200)
    {
        LOG(INFO) << ret;
    }
    else
    {
        LOG(INFO) << "http failed(" << code << ") port: " << port
                  << " command: " << command;
    }
}

void
loadXdr(Config cfg, std::string const& bucketFile)
{
    VirtualClock clock;
    cfg.setNoListen();
    Application::pointer app = Application::create(clock, cfg, false);
    if (checkInitialized(app))
    {
        uint256 zero;
        Bucket bucket(bucketFile, zero);
        bucket.apply(*app);
    }
    else
    {
        LOG(INFO) << "Database is not initialized";
    }
}

void
setForceSCPFlag(Config cfg, bool set)
{
    VirtualClock clock;
    cfg.setNoListen();
    Application::pointer app = Application::create(clock, cfg, false);

    if (checkInitialized(app))
    {
        app->getPersistentState().setState(
            PersistentState::kForceSCPOnNextLaunch, (set ? "true" : "false"));
        if (set)
        {
            LOG(INFO) << "* ";
            LOG(INFO) << "* The `force scp` flag has been set in the db.";
            LOG(INFO) << "* ";
            LOG(INFO)
                << "* The next launch will start scp from the account balances";
            LOG(INFO) << "* as they stand in the db now, without waiting to "
                         "hear from";
            LOG(INFO) << "* the network.";
            LOG(INFO) << "* ";
        }
        else
        {
            LOG(INFO) << "* ";
            LOG(INFO) << "* The `force scp` flag has been cleared.";
            LOG(INFO) << "* The next launch will start normally.";
            LOG(INFO) << "* ";
        }
    }
}

void
initializeDatabase(Config cfg)
{
    VirtualClock clock;
    cfg.setNoListen();
    isNewDb = true;
    Application::pointer app = Application::create(clock, cfg, isNewDb);

    // Initialize Bitcoin database
    try
    {
        LOG(INFO) << "* ";
        LOG(INFO) << "* Initializing bitcoin blocks...";
        LOG(INFO) << "* ";

        initializeRunBitcoinDaemon(cfg);
        
        LOG(INFO) << "* ";
        LOG(INFO) << "* Successfully initialized bitcoin blocks";
        LOG(INFO) << "* ";
    }
    catch(std::exception& e)
    {
        LOG(FATAL) << " Error initializing Bitcoin database";
        LOG(FATAL) << e.what();
    }


    LOG(INFO) << "*";
    LOG(INFO) << "* The next launch will catchup from the network afresh.";
    LOG(INFO) << "*";
}

// ********************* Bitcoin ******************************************** 



static bool AppInit(int argc, const char* argv[])
{
    InitInterfaces interfaces;
    interfaces.chain = interfaces::MakeChain();

    bool fRet = false;

    //
    // Parameters
    //
    // If Qt is used, parameters/bitcoin.conf are parsed in qt/bitcoin.cpp's main()
    SetupServerArgs();
    std::string error;
    if (!gArgs.ParseParameters(argc, argv, error)) {
        fprintf(stderr, "Error parsing command line arguments: %s\n", error.c_str());
        return false;
    }

    // Process help and version before taking care about datadir
    if (HelpRequested(gArgs) || gArgs.IsArgSet("-version")) {
        std::string strUsage = PACKAGE_NAME " Daemon version " + FormatFullVersion() + "\n";

        if (gArgs.IsArgSet("-version"))
        {
            strUsage += FormatParagraph(LicenseInfo()) + "\n";
        }
        else
        {
            strUsage += "\nUsage:  bitcoind [options]                     Start " PACKAGE_NAME " Daemon\n";
            strUsage += "\n" + gArgs.GetHelpMessage();
        }

        fprintf(stdout, "%s", strUsage.c_str());
        return true;
    }

    try
    {
        if (!boost::filesystem::is_directory(GetDataDir(false)))
        {
            fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", gArgs.GetArg("-datadir", "").c_str());
            return false;
        }
        if (!gArgs.ReadConfigFiles(error, true)) {
            fprintf(stderr, "Error reading configuration file: %s\n", error.c_str());
            return false;
        }
        // Check for -testnet or -regtest parameter (Params() calls are only valid after this clause)
        try {
            SelectParams(gArgs.GetChainName());
        } catch (const std::exception& e) {
            fprintf(stderr, "Error: %s\n", e.what());
            return false;
        }

        // Error out when loose non-argument tokens are encountered on command line
        for (int i = 1; i < argc; i++) {
            if (!IsSwitchChar(argv[i][0])) {
                fprintf(stderr, "Error: Command line contains unexpected token '%s', see bitcoind -h for a list of options.\n", argv[i]);
                return false;
            }
        }

        // -server defaults to true for bitcoind but not for the GUI so do this here
        gArgs.SoftSetBoolArg("-server", true);
        // Set this early so that parameter interactions go to console
        InitLogging();
        InitParameterInteraction();
        if (!AppInitBasicSetup())
        {
            // InitError will have been called with detailed error, which ends up on console
            return false;
        }
        if (!AppInitParameterInteraction())
        {
            // InitError will have been called with detailed error, which ends up on console
            return false;
        }
        if (!AppInitSanityChecks())
        {
            // InitError will have been called with detailed error, which ends up on console
            return false;
        }
        if (gArgs.GetBoolArg("-daemon", false))
        {
#if HAVE_DECL_DAEMON
            fprintf(stdout, "Bitcoin server starting\n");

            // Daemonize
            if (daemon(1, 0)) { // don't chdir (1), do close FDs (0)
                fprintf(stderr, "Error: daemon() failed: %s\n", strerror(errno));
                return false;
            }
#else
            fprintf(stderr, "Error: -daemon is not supported on this operating system\n");
            return false;
#endif // HAVE_DECL_DAEMON
        }
        // Lock data directory after daemonization
        if (!AppInitLockDataDirectory())
        {
            // If locking the data directory failed, exit immediately
            return false;
        }
        fRet = AppInitMain(interfaces);
        
    LOG(INFO) << "*";
    LOG(INFO) << "* Bitcoin Initialization Ends.....................";
    LOG(INFO) << "*";
    }
    catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInit()");
    } catch (...) {
        PrintExceptionContinue(nullptr, "AppInit()");
    }
    return fRet;
}

static void WaitForShutdown()
{
    while (!ShutdownRequested())
    {
        MilliSleep(200);
    }
    Interrupt();
}

/**
 * Initialize and run bitcoind
 * */
void 
initializeRunBitcoinDaemon(Config cfg)
{   

    LOG(INFO) << "*";
    LOG(INFO) << "* Bitcoin Initialization Starts.....................\n";
    LOG(INFO) << "*";
    try
    {
        
        cfg.setNoListen();
        SetupEnvironment();
        // Connect bitcoind signal handlers
        noui_connect();
        std::string dataDir;
        if (cfg.BITCOIN_DATADIR != "")
        {
            dataDir = "-datadir=" + cfg.BITCOIN_DATADIR;
        }
        else
        {
            dataDir = "-datadir=" + GetDefaultDataDir().string();
        }
        std::cout <<  "Bitcoin data directory path : " << dataDir << "\n";
        
        const char* argvArrayBC[] = {
            "bitcoind", 
            "-regtest", 
            dataDir.c_str()
        };
        bool test = AppInit(3, argvArrayBC);

        if(test && isNewDb)
        {
            std::string publicKey = cfg.BITCOIN_MINER_ADDRESS;
            std::cout << "Miner's public key : " <<  publicKey << "\n";
            // Call generateZaggBlocksToAddress to form blocks for miner
            int nGenerate = cfg.BITCOIN_MINER_BLOCKS;
            LOG(INFO) << "Generating " << nGenerate << " blocks to the address " << publicKey <<  "\n";
            MarkAccountOpFrame::generateZaggBlocksToAddress(publicKey, 101, "");
        }
    }
    catch (std::exception& e)
    {
        LOG(FATAL) << "Bitcoin Initialization Error.\n";
        LOG(FATAL) << e.what();
    }    
}

// ********************* Bitcoin ******************************************** 

void
showOfflineInfo(Config cfg)
{
    // needs real time to display proper stats
    VirtualClock clock(VirtualClock::REAL_TIME);
    cfg.setNoListen();
    Application::pointer app = Application::create(clock, cfg, false);
    if (checkInitialized(app))
    {
        app->reportInfo();
    }
    else
    {
        LOG(INFO) << "Database is not initialized";
    }
}

int
reportLastHistoryCheckpoint(Config cfg, std::string const& outputFile)
{
    VirtualClock clock(VirtualClock::REAL_TIME);
    cfg.setNoListen();
    Application::pointer app = Application::create(clock, cfg, false);

    if (!checkInitialized(app))
    {
        return 1;
    }

    auto state = HistoryArchiveState{};
    auto& wm = app->getWorkManager();
    auto getHistoryArchiveStateWork =
        wm.executeWork<GetHistoryArchiveStateWork>(
            "get-history-archive-state-work", state);

    auto ok = getHistoryArchiveStateWork->getState() == Work::WORK_SUCCESS;
    if (ok)
    {
        std::string filename = outputFile.empty() ? "-" : outputFile;

        if (filename == "-")
        {
            LOG(INFO) << "*";
            LOG(INFO) << "* Last history checkpoint " << state.toString();
            LOG(INFO) << "*";
        }
        else
        {
            state.save(filename);
            LOG(INFO) << "*";
            LOG(INFO) << "* Wrote last history checkpoint " << filename;
            LOG(INFO) << "*";
        }
    }
    else
    {
        LOG(INFO) << "*";
        LOG(INFO) << "* Fetching last history checkpoint failed.";
        LOG(INFO) << "*";
    }

    app->gracefulStop();
    while (clock.crank(true))
        ;

    return ok ? 0 : 1;
}

void
genSeed()
{
    auto key = SecretKey::random();
    std::cout << "Secret seed: " << key.getStrKeySeed().value << std::endl;
    std::cout << "Public: " << key.getStrKeyPublic() << std::endl;
}

int
initializeHistories(Config cfg, std::vector<std::string> const& newHistories)
{
    VirtualClock clock;
    cfg.setNoListen();
    Application::pointer app = Application::create(clock, cfg, false);

    for (auto const& arch : newHistories)
    {
        if (!app->getHistoryArchiveManager().initializeHistoryArchive(arch))
            return 1;
    }
    return 0;
}

void
writeCatchupInfo(Json::Value const& catchupInfo, std::string const& outputFile)
{
    std::string filename = outputFile.empty() ? "-" : outputFile;
    auto content = catchupInfo.toStyledString();

    if (filename == "-")
    {
        LOG(INFO) << "*";
        LOG(INFO) << "* Catchup info: " << content;
        LOG(INFO) << "*";
    }
    else
    {
        std::ofstream out{};
        out.open(filename);
        out.write(content.c_str(), content.size());
        out.close();

        LOG(INFO) << "*";
        LOG(INFO) << "* Wrote catchup info to " << filename;
        LOG(INFO) << "*";
    }
}

int
catchup(Application::pointer app, CatchupConfiguration cc,
        Json::Value& catchupInfo)
{
    if (!checkInitialized(app))
    {
        return 1;
    }

    app->start();

    try
    {
        app->getLedgerManager().startCatchup(cc, true);
    }
    catch (std::invalid_argument const&)
    {
        LOG(INFO) << "*";
        LOG(INFO) << "* Target ledger " << cc.toLedger()
                  << " is not newer than last closed ledger"
                  << " - nothing to do";
        LOG(INFO) << "* If you really want to catchup to " << cc.toLedger()
                  << " run stellar-core new-db";
        LOG(INFO) << "*";
        return 2;
    }

    auto& clock = app->getClock();
    auto& io = clock.getIOService();
    auto synced = false;
    asio::io_service::work mainWork(io);
    auto done = false;
    while (!done && clock.crank(true))
    {
        switch (app->getLedgerManager().getState())
        {
        case LedgerManager::LM_BOOTING_STATE:
        {
            done = true;
            break;
        }
        case LedgerManager::LM_SYNCED_STATE:
        {
            break;
        }
        case LedgerManager::LM_CATCHING_UP_STATE:
        {
            switch (app->getLedgerManager().getCatchupState())
            {
            case LedgerManager::CatchupState::WAITING_FOR_CLOSING_LEDGER:
            {
                done = true;
                synced = true;
                break;
            }
            case LedgerManager::CatchupState::NONE:
            {
                done = true;
                break;
            }
            default:
            {
                break;
            }
            }
            break;
        }
        case LedgerManager::LM_NUM_STATE:
            abort();
        }
    }

    LOG(INFO) << "*";
    if (synced)
    {
        LOG(INFO) << "* Catchup finished.";
    }
    else
    {
        LOG(INFO) << "* Catchup failed.";
    }
    LOG(INFO) << "*";

    catchupInfo = app->getJsonInfo();
    return synced ? 0 : 3;
}

int
publish(Application::pointer app)
{
    if (!checkInitialized(app))
    {
        return 1;
    }

    app->start();

    auto& clock = app->getClock();
    auto& io = clock.getIOService();
    asio::io_service::work mainWork(io);

    auto lcl = app->getLedgerManager().getLastClosedLedgerNum();
    auto isCheckpoint =
        lcl == app->getHistoryManager().checkpointContainingLedger(lcl);
    auto expectedPublishQueueSize = isCheckpoint ? 1 : 0;

    app->getHistoryManager().publishQueuedHistory();
    while (app->getHistoryManager().publishQueueLength() !=
               expectedPublishQueueSize &&
           clock.crank(true))
    {
    }

    LOG(INFO) << "*";
    LOG(INFO) << "* Publish finished.";
    LOG(INFO) << "*";

    return 0;
}
}
