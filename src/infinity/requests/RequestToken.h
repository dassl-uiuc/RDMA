/**
 * Requests - Request Token
 *
 * (c) 2018 Claude Barthels, ETH Zurich
 * Contact: claudeb@inf.ethz.ch
 *
 */

#ifndef REQUESTS_REQUESTTOKEN_H_
#define REQUESTS_REQUESTTOKEN_H_

#include <atomic>
#include <stdint.h>

#include <infinity/core/Context.h>
#include <infinity/memory/Region.h>

namespace infinity {
namespace requests {

class RequestToken {

public:

	RequestToken(infinity::core::Context *context);

	void reset();

	void setRegion(infinity::memory::Region * region);
	infinity::memory::Region * getRegion();

	void setCompleted(bool success);
	void setAllPrevCompleted();
	bool wasSuccessful();

	bool checkIfCompleted();
	bool checkAllPrevCompleted();
	void waitUntilCompleted();
	void waitUntilCompletedCond(bool *cond);

	void setImmediateValue(uint32_t immediateValue);
	bool hasImmediateValue();
	uint32_t getImmediateValue();

	void setUserData(void* userData, uint32_t userDataSize);
	bool hasUserData();
	void* getUserData();
	uint32_t getUserDataSize();

	std::atomic<bool> completed;  // todo: tmp approach

protected:

	infinity::core::Context * const context;
	infinity::memory::Region * region;

	std::atomic<bool> all_prev_completed;
	std::atomic<bool> success;

	void *userData;
	uint32_t userDataSize;
	bool userDataValid;

	uint32_t immediateValue;
	bool immediateValueValid;

};

} /* namespace requests */
} /* namespace infinity */

#endif /* REQUESTS_REQUESTTOKEN_H_ */
