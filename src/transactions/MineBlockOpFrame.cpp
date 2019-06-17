// Copyright 2017 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "transactions/MineBlockOpFrame.h"
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
#include <lib/bitcoin/src/validation.h>
#include <key_io.h>
namespace stellar 
{
MineBlockOpFrame::MineBlockOpFrame(Operation const& op,
                                        OperationResult& res,
                                        TransactionFrame& parentTx)
    : OperationFrame(op, res, parentTx)
    , mMineBlockOp(mOperation.body.mineBlockOp())
{
}


ThresholdLevel
MineBlockOpFrame::getThresholdLevel() const 
{
    return ThresholdLevel::HIGH;
}


bool
MineBlockOpFrame::isVersionSupported(uint32_t protocolVersion) const
{
    return protocolVersion >= 10;
}

bool 
MineBlockOpFrame::doApply(Application& app, AbstractLedgerTxn& ltx)
{

    std::cout << "inside do doApply valid of Mark account operation";
    try
    {
        // generateZaggBlocksToAddress("2NEhcXwh2J7US9oCgYYZAjf3czpsFf1XPCU", 1,  mMarkAccountOp.accountMarker);

        // Return successful results
        innerResult().code(MINE_BLOCK_SUCCESS);
        app.getMetrics()
        .NewMeter({"op-mine-block","success","apply"}, "operation")
        .Mark();
    }
    catch(const UniValue& objError)
    {
        return false;
    }
    return true;
}



bool
MineBlockOpFrame::doCheckValid(Application& app, uint32_t ledgerVersion)
{
    std::ostringstream output;

    //std::cout << "inside do check valid of Mark account operation" << mMineBlockOp.block` <<"\n";
    try
    {
        //SendRawTransactionZagg(mMarkAccountOp.accountMarker);
    }
    catch (const UniValue& objError)
    {   
        // output << JSONRPCReply(UniValue::VNULL, objError, UniValue::VOBJ);
        // std::cout << output.str() << std::endl;

        // // Return successful results
        app.getMetrics()
            .NewMeter({"op-mine-block", "invalid", "failed"}, "operation")
            .Mark();
        innerResult().code(MINE_BLOCK_FAILED);
        return false;
    }
    return true;
}
}