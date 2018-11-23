//

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
MarkAccountOpFrame::getThreadsholdLevel() const 
{
    return ThresholdLevel::HIGH;
}


bool
MarkSequenceOpFrame::isVersionSupported(uint32_t protocolVersion) const
{
    return protocolVersion >= 10;
}


bool 
MarkAccountOpFrame::doApply(Application& app, LedgerDelta& delta,
                             LedgerManger& ledgerManager)
{
    AccountMarker accountMarker = mSourceAccount->getAccountMarker();

    if(accountMarker != null)
    {
        mSourceAccount->setAccountMarker(mMarkAccountOp.markTo);
        mSourceAccount->storageChange(delta, ledgerManager.getDatabase());
    }

    innerResult().code(MARK_ACCOUNT_SUCCESS);
    app.getMetrics()
        .NewMeter({"op-mark-account","success","apply"}, "operation")
        .Mark();
    return true;
}

bool
MarkAccountOpFrame::doCheckValid(Application& app)
{
    if (mMarkAccountOp.markTo == 0)
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