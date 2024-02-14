#include "CombinedReqToken.h"

namespace infinity {
namespace requests {

CombinedReqToken::CombinedReqToken(core::Context* ctx) : context(ctx), tk1(ctx), tk2(ctx) {}

void CombinedReqToken::WaitUntilBothCompleted() {
    while (!tk1.completed.load() || !tk2.completed.load()) {
        context->pollTwoSendCompletion();
    }
}

bool CombinedReqToken::CheckBothCompleted() {
    if (tk1.completed.load() && tk2.completed.load()) {
        return true;
    } else {
        context->pollTwoSendCompletion();
        return (tk1.completed.load() && tk2.completed.load());
    }
}

}  // namespace requests
}  // namespace infinity
