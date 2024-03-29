/*
 * Memory - Buffer
 *
 * (c) 2018 Claude Barthels, ETH Zurich
 * Contact: claudeb@inf.ethz.ch
 *
 */

#ifndef MEMORY_BUFFER_H_
#define MEMORY_BUFFER_H_

#include <infinity/core/Context.h>
#include <infinity/memory/Region.h>
#include <infinity/memory/RegisteredMemory.h>

#include <vector>

namespace infinity {
namespace memory {

class Buffer : public Region {

public:

	Buffer(infinity::core::Context *context, uint64_t sizeInBytes);
	
	// Constructor for integer buffer.
	Buffer(infinity::core::Context *context, uint64_t sizeInBytes, std::vector<uint32_t> data);
	Buffer(infinity::core::Context *context, infinity::memory::RegisteredMemory *memory, uint64_t offset, uint64_t sizeInBytes);
	Buffer(infinity::core::Context *context, void *memory, uint64_t sizeInBytes);
	~Buffer();

public:

	void * getData();
	uint32_t* getIntData();
	// Update one index in the integer array.
	void UpdateIntMemory(int index, uint32_t value);
	void resize(uint64_t newSize, void *newData = NULL);

protected:

	bool memoryRegistered;
	bool memoryAllocated;


};

} /* namespace memory */
} /* namespace infinity */

#endif /* MEMORY_BUFFER_H_ */
