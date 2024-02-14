#ifndef REQUESTS_COMBINEDREQTOKEN_H_
#define REQUESTS_COMBINEDREQTOKEN_H_

#include <infinity/requests/RequestToken.h>

namespace infinity {
namespace requests {

struct CombinedReqToken {
    core::Context *const context;
    RequestToken tk1;
    RequestToken tk2;

    CombinedReqToken(core::Context *ctx);

    void WaitUntilBothCompleted();
    bool CheckBothCompleted();
};

}  // namespace requests
}  // namespace infinity

#endif
