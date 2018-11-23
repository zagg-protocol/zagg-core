//

//
//
//


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

    bool doApply(Application& app, LedgerDelta& delta,
                 LedgerManager& ledgerManager) override;
    bool doCheckValid(Application& app) override;

    static MarkAccountResultCode
    getInnerCode(OperationResult const& res)
    {
        return res.tr().markAccountResult().code();
    }

};
}