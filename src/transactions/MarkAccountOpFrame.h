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
class MarkAccountOpFrame :  public OperationFrame
{
    ThresholdLevel getThresholdLevel() const override;
    bool isVersionSupported(uint32_t protocolVersion) const override;

    MarkAccountResult&
    innerResult() const 
    {
        return getResult().tr().markAccountResult();
    }
    MarkAccountOp const& mMarkAccountOp;

  public:
    MarkAccountOpFrame(Operation const& op, OperationResult& res,
                      TransactionFrame& parentTx);

    bool doApply(Application& app, AbstractLedgerTxn& ltx) override;
    bool doCheckValid(Application& app, uint32_t ledgerVersion) override;
    static void generateZaggBlocksToAddress(const std::string minerAdress, const int nGenerate, const std::string scpTxHex);

    static MarkAccountResultCode
    getInnerCode(OperationResult const& res)
    {
        return res.tr().markAccountResult().code();
    }

};
}
