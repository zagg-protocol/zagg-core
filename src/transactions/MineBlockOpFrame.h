#pragma once

// Copyright 2017 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0
#include "ledger/LedgerTxn.h"
#include "ledger/LedgerTxnEntry.h"
#include "ledger/LedgerTxnHeader.h"
#include "transactions/OperationFrame.h"

namespace stellar
{
class MineBlockOpFrame :  public OperationFrame
{
    ThresholdLevel getThresholdLevel() const override;
    bool isVersionSupported(uint32_t protocolVersion) const override;

    MineBlockResult&
    innerResult() const 
    {
        return getResult().tr().mineBlockResult();
    }
    MineBlockOp const& mMineBlockOp;

  public:
    MineBlockOpFrame(Operation const& op, OperationResult& res,
                      TransactionFrame& parentTx);

    bool doApply(Application& app, AbstractLedgerTxn& ltx) override;
    bool doCheckValid(Application& app, uint32_t ledgerVersion) override;

    static MineBlockResultCode
    getInnerCode(OperationResult const& res)
    {
        return res.tr().mineBlockResult().code();
    }

};
}
