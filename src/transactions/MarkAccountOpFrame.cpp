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
    if (mMarkAccountOp.accountMarker == 0)
    {
        app.getMetrics()
            .NewMeter({"op-mark-account", "invalid", "bad-marker"}, "operation")
            .Mark();
        innerResult().code(MARK_ACCOUNT_BAD_MARKER);
        return false;
    }
    return true;
}
}