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
        // this will return block hash of new block with one bitcoin tx
        UniValue blockHash = generateBlocks(NULL, mMarkAccountOp.accountMarker);
        std::string blockHashStr;
        blockHash.setStr(blockHashStr);
        
        std::cout << "inside do check valid of Mark account operation" << blockHashStr <<"\n";

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
    // LedgerTxn ltxInner(ltx);
    // auto header = ltxInner.loadHeader();
    // auto sourceAccountEntry = loadSourceAccount(ltxInner, header);
    // auto& sourceAccount = sourceAccountEntry.current().data.account();

    // // Apply the bump (bump succeeds silently if bumpTo <= current)
    // {
    //     sourceAccount.accountMarker = mMarkAccountOp.accountMarker;
    //     ltxInner.commit();
    // }    
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