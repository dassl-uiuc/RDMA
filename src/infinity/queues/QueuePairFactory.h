/**
 * Queues - Queue Pair Factory
 *
 * (c) 2018 Claude Barthels, ETH Zurich
 * Contact: claudeb@inf.ethz.ch
 *
 */

#ifndef QUEUES_QUEUEPAIRFACTORY_H_
#define QUEUES_QUEUEPAIRFACTORY_H_

#include <string>

#include <stdlib.h>
#include <stdint.h>


#include <infinity/core/Configuration.h>
#include <infinity/core/Context.h>
#include <infinity/queues/QueuePair.h>

namespace infinity {
namespace queues {

typedef struct {

	uint16_t localDeviceId;
	uint32_t queuePairNumber;
	uint32_t sequenceNumber;
	uint32_t userDataSize;
	char userData[infinity::core::Configuration::MAX_CONNECTION_USER_DATA_SIZE];
	union ibv_gid gid;

} serializedQueuePair;

class QueuePairFactory {
public:

	QueuePairFactory(infinity::core::Context *context);
	~QueuePairFactory();

	/**
	 * Bind to port for listening to incoming connections
	 */
	void bindToPort(uint16_t port);

	/**
	 * Accept incoming connection request (passive side)
	 */
	QueuePair * acceptIncomingConnection(void *userData = NULL, uint32_t userDataSizeInBytes = 0);

	/**
	 * Accept incoming connection request without creating a qp
	 * @return socket for the accepted connection
	 */
	int waitIncomingConnection(serializedQueuePair **recvBuf);

	/**
	 * Reply to the incomming connection with MR info and create a qp
	 * @param socket the connection socket used for reply
	 * @return the created qp
	 */
	QueuePair * replyIncomingConnection(int socket, serializedQueuePair* recvBuf, void *userData = NULL, uint32_t userDataSizeInBytes = 0);

	/**
	 * Connect to remote machine (active side)
	 */
	QueuePair * connectToRemoteHost(const char* hostAddress, uint16_t port, void *userData = NULL, uint32_t userDataSizeInBytes = 0);

	/**
	 * Create loopback queue pair
	 */
	QueuePair * createLoopback(void *userData = NULL, uint32_t userDataSizeInBytes = 0);

	/**
	 * Get ip address of this device
	 */
	static std::string &getIpAddress() { return ipAddress; }

	/**
	 * Get the socket for accepting incoming connection
	 */
	int getServerSocket() { return serverSocket; }
	
	static bool calculateIpAddress();

protected:

	infinity::core::Context * context;

	int32_t serverSocket;

	inline static std::string ipAddress;
};

} /* namespace queues */
} /* namespace infinity */

#endif /* QUEUES_QUEUEPAIRFACTORY_H_ */
