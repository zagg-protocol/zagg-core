// Copyright 2017 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "transactions/MarkAccountOpFrame.h"
#include "crypto/SignerKey.h"
#include "database/Database.h"
#include "main/Application.h"
#include "medida/meter.h"
#include "medida/metrics_registry.h"
#include "transactions/TransactionFrame.h"
#include "util/XDROperators.h"
#include <rpc/mining.h>
#include <rpc/rawtransaction.h>
#include <univalue.h>
#include <rpc/protocol.h>
#include <lib/bitcoin/src/version.h>
#include <key_io.h>
namespace stellar 
{
MarkAccountOpFrame::MarkAccountOpFrame(Operation const& op,
                                        OperationResult& res,
                                        TransactionFrame& parentTx)
    : OperationFrame(op, res, parentTx)
    , mMarkAccountOp(mOperation.body.markAccountOp())
{
}


ThresholdLevel
MarkAccountOpFrame::getThresholdLevel() const 
{
    return ThresholdLevel::HIGH;
}


bool
MarkAccountOpFrame::isVersionSupported(uint32_t protocolVersion) const
{
    return protocolVersion >= 10;
}

bool 
MarkAccountOpFrame::doApply(Application& app, AbstractLedgerTxn& ltx)
{

    std::cout << "inside do doApply valid of Mark account operation";
    try
    {
        // This address can be hardcoded.Keeping it here for the purpose of test. 
        // Anyway we neeed to remove mining later.
        CTxDestination destination = DecodeDestination("2NEhcXwh2J7US9oCgYYZAjf3czpsFf1XPCU");
        if (!IsValidDestination(destination)) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Error: Invalid address");
        }

        // For zagg the number of block will be 1
        int nGenerate = 1;
        // This values are taken from bitcoin code base.
        uint64_t nMaxTries = 1000000;
        
        std::shared_ptr<CReserveScript> coinbaseScript = std::make_shared<CReserveScript>();
        coinbaseScript->reserveScript = GetScriptForDestination(destination);

        std::cout << "before calling  generateBlocksZagg\n";
        UniValue blockHash = generateBlocks(coinbaseScript, nMaxTries, false, mMarkAccountOp.accountMarker, nGenerate);
        std::cout << "generateBlocks call complete\n";

        // Return successful results
        innerResult().code(MARK_ACCOUNT_SUCCESS);
        app.getMetrics()
        .NewMeter({"op-mark-account","success","apply"}, "operation")
        .Mark();
    }
    catch(const UniValue& objError)
    {
        return false;
    }
    return true;
}

bool
MarkAccountOpFrame::doCheckValid(Application& app, uint32_t ledgerVersion)
{
    std::ostringstream output;

    std::cout << "inside do check valid of Mark account operation" << mMarkAccountOp.accountMarker <<"\n";
    try
    {
        SendRawTransactionZagg(mMarkAccountOp.accountMarker);
    }
    catch (const UniValue& objError)
    {   
        output << JSONRPCReply(UniValue::VNULL, objError, UniValue::VOBJ);
        std::cout << output.str() << std::endl;

        // Return successful results
        app.getMetrics()
            .NewMeter({"op-mark-account", "invalid", "bad-marker"}, "operation")
            .Mark();
        innerResult().code(MARK_ACCOUNT_BAD_MARKER);
        return false;
    }
    return true;
}
}