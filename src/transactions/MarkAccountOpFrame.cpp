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
#include <rpc/rawtransaction.h>
#include <univalue.h>
#include <rpc/protocol.h>

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
    LedgerTxn ltxInner(ltx);
    auto header = ltxInner.loadHeader();
    auto sourceAccountEntry = loadSourceAccount(ltxInner, header);
    auto& sourceAccount = sourceAccountEntry.current().data.account();

    // Apply the bump (bump succeeds silently if bumpTo <= current)
    {
        sourceAccount.accountMarker = mMarkAccountOp.accountMarker;
        ltxInner.commit();
    }

    // Return successful results
    innerResult().code(MARK_ACCOUNT_SUCCESS);
    app.getMetrics()
    .NewMeter({"op-mark-account","success","apply"}, "operation")
    .Mark();
    return true;
    
}

bool
MarkAccountOpFrame::doCheckValid(Application& app, uint32_t ledgerVersion)
{
    std::ostringstream output;

    std::cout << "inside do check valid of Mark account operation" << mMarkAccountOp.accountMarker <<"\n";
    try
    {
        SendRawTransactionZagg("01000000000101bfaeeadb2d790fdea00538baacf853d8b1dd304ca526334e1a21a4657100b5060100000000fdffffff02a08601000000000017a914f6774b8bec6289510547d0ced58a6f97b486ca69871cac973b000000001600145104207f00f55d981bafc2ce78126c8ef2dd55c6024730440220269518e30dc873425e24a41c9f2999094abe0a212d76901457d610bc453db29a02207d77ac0923cc4d158cca2bce78b95cc03f2ac7924d4655aa5299e3c940ae0720012102a4391cecb2e1e731ce4889abc4df6af08d23b11d346d67fdb851c225dd05ae5771000000");
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