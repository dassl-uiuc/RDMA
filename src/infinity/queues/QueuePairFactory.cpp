/**
 * Queues - Queue Pair Factory
 *
 * (c) 2018 Claude Barthels, ETH Zurich
 * Contact: claudeb@inf.ethz.ch
 *
 */

#include "QueuePairFactory.h"

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <infinity/utils/Debug.h>
#include <infinity/utils/Address.h>

namespace infinity {
namespace queues {

QueuePairFactory::QueuePairFactory(infinity::core::Context *context) {

	this->context = context;
	this->serverSocket = -1;
}

QueuePairFactory::~QueuePairFactory() {

	if (serverSocket >= 0) {
		close(serverSocket);
	}

}

void QueuePairFactory::bindToPort(uint16_t port) {

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	INFINITY_ASSERT(serverSocket >= 0, "[INFINITY][QUEUES][FACTORY] Cannot open server socket.\n");

	sockaddr_in serverAddress;
	memset(&(serverAddress), 0, sizeof(sockaddr_in));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);

	int32_t enabled = 1;
	int32_t returnValue = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
	INFINITY_ASSERT(returnValue == 0, "[INFINITY][QUEUES][FACTORY] Cannot set socket option to reuse address.\n");

	returnValue = bind(serverSocket, (sockaddr *) &serverAddress, sizeof(sockaddr_in));
	INFINITY_ASSERT(returnValue == 0, "[INFINITY][QUEUES][FACTORY] Cannot bind to local address and port.\n");

	returnValue = listen(serverSocket, 128);
	INFINITY_ASSERT(returnValue == 0, "[INFINITY][QUEUES][FACTORY] Cannot listen on server socket.\n");

	INFINITY_DEBUG("[INFINITY][QUEUES][FACTORY] Accepting connections on IP address %s and port %d.\n", ipAddress.c_str(), port);
}

QueuePair * QueuePairFactory::acceptIncomingConnection(void *userData, uint32_t userDataSizeInBytes) {
	serializedQueuePair *recvBuf;
	int sendSocket = waitIncomingConnection(&recvBuf);
	return replyIncomingConnection(sendSocket, recvBuf, userData, userDataSizeInBytes);
}

int QueuePairFactory::waitIncomingConnection(serializedQueuePair **recvBuf) {
	int connectionSocket = accept(this->serverSocket, (sockaddr *) NULL, NULL);
	INFINITY_ASSERT(connectionSocket >= 0, "[INFINITY][QUEUES][FACTORY] Cannot open connection socket.\n");

	serializedQueuePair *receiveBuffer = (serializedQueuePair*) calloc(1, sizeof(serializedQueuePair));
	int32_t returnValue = recv(connectionSocket, receiveBuffer, sizeof(serializedQueuePair), 0);
	INFINITY_ASSERT(returnValue == sizeof(serializedQueuePair), "[INFINITY][QUEUES][FACTORY] Incorrect number of bytes received. Expected %lu. Received %d.\n",
			sizeof(serializedQueuePair), returnValue);
	*recvBuf = receiveBuffer;
	return connectionSocket;
}

QueuePair * QueuePairFactory::replyIncomingConnection(int socket, serializedQueuePair* receiveBuffer, void *userData, uint32_t userDataSizeInBytes) {
	INFINITY_ASSERT(userDataSizeInBytes < infinity::core::Configuration::MAX_CONNECTION_USER_DATA_SIZE,
			"[INFINITY][QUEUES][FACTORY] User data size is too large.\n")
	
	int32_t returnValue;
	serializedQueuePair *sendBuffer = (serializedQueuePair*) calloc(1, sizeof(serializedQueuePair));
	
	QueuePair *queuePair = new QueuePair(this->context);
	queuePair->setRemoteSocket(socket);

	sendBuffer->localDeviceId = queuePair->getLocalDeviceId();
	sendBuffer->queuePairNumber = queuePair->getQueuePairNumber();
	sendBuffer->sequenceNumber = queuePair->getSequenceNumber();
	sendBuffer->userDataSize = userDataSizeInBytes;
	sendBuffer->gid = queuePair->getLocalGid();
	memcpy(sendBuffer->userData, userData, userDataSizeInBytes);

	returnValue = send(socket, sendBuffer, sizeof(serializedQueuePair), 0);
	INFINITY_ASSERT(returnValue == sizeof(serializedQueuePair),
			"[INFINITY][QUEUES][FACTORY] Incorrect number of bytes transmitted. Expected %lu. Received %d.\n", sizeof(serializedQueuePair), returnValue);

	INFINITY_DEBUG("[INFINITY][QUEUES][FACTORY] Pairing (%u, %u, %u, %u)-(%u, %u, %u, %u)\n", queuePair->getLocalDeviceId(), queuePair->getQueuePairNumber(),
			queuePair->getSequenceNumber(), userDataSizeInBytes, receiveBuffer->localDeviceId, receiveBuffer->queuePairNumber, receiveBuffer->sequenceNumber,
			receiveBuffer->userDataSize);

	queuePair->activate(receiveBuffer->localDeviceId, receiveBuffer->queuePairNumber, receiveBuffer->sequenceNumber, receiveBuffer->gid);
	queuePair->setRemoteUserData(receiveBuffer->userData, receiveBuffer->userDataSize);

	this->context->registerQueuePair(queuePair);

	free(receiveBuffer);
	free(sendBuffer);

	return queuePair;
}

QueuePair * QueuePairFactory::connectToRemoteHost(const char* hostAddress, uint16_t port, void *userData, uint32_t userDataSizeInBytes) {

	INFINITY_ASSERT(userDataSizeInBytes < infinity::core::Configuration::MAX_CONNECTION_USER_DATA_SIZE,
			"[INFINITY][QUEUES][FACTORY] User data size is too large.\n")

	serializedQueuePair *receiveBuffer = (serializedQueuePair*) calloc(1, sizeof(serializedQueuePair));
	serializedQueuePair *sendBuffer = (serializedQueuePair*) calloc(1, sizeof(serializedQueuePair));

	sockaddr_in remoteAddress;
	memset(&(remoteAddress), 0, sizeof(sockaddr_in));
	remoteAddress.sin_family = AF_INET;
	inet_pton(AF_INET, hostAddress, &(remoteAddress.sin_addr));
	remoteAddress.sin_port = htons(port);

	int connectionSocket = socket(AF_INET, SOCK_STREAM, 0);
	INFINITY_ASSERT(connectionSocket >= 0, "[INFINITY][QUEUES][FACTORY] Cannot open connection socket.\n");

	int returnValue = connect(connectionSocket, (sockaddr *) &(remoteAddress), sizeof(sockaddr_in));
	INFINITY_ASSERT(returnValue == 0, "[INFINITY][QUEUES][FACTORY] Could not connect to server. ret %d\n", returnValue);

	QueuePair *queuePair = new QueuePair(this->context);
	queuePair->setRemoteSocket(connectionSocket);
	queuePair->setRemoteAddr(hostAddress);

	sendBuffer->localDeviceId = queuePair->getLocalDeviceId();
	sendBuffer->queuePairNumber = queuePair->getQueuePairNumber();
	sendBuffer->sequenceNumber = queuePair->getSequenceNumber();
	sendBuffer->userDataSize = userDataSizeInBytes;
	sendBuffer->gid = queuePair->getLocalGid();
	memcpy(sendBuffer->userData, userData, userDataSizeInBytes);

	returnValue = send(connectionSocket, sendBuffer, sizeof(serializedQueuePair), 0);
	INFINITY_ASSERT(returnValue == sizeof(serializedQueuePair),
			"[INFINITY][QUEUES][FACTORY] Incorrect number of bytes transmitted. Expected %lu. Received %d.\n", sizeof(serializedQueuePair), returnValue);

	returnValue = recv(connectionSocket, receiveBuffer, sizeof(serializedQueuePair), 0);
	INFINITY_ASSERT(returnValue == sizeof(serializedQueuePair),
			"[INFINITY][QUEUES][FACTORY] Incorrect number of bytes received. Expected %lu. Received %d.\n", sizeof(serializedQueuePair), returnValue);

	INFINITY_DEBUG("[INFINITY][QUEUES][FACTORY] Pairing (%u, %u, %u, %u)-(%u, %u, %u, %u)\n", queuePair->getLocalDeviceId(), queuePair->getQueuePairNumber(),
			queuePair->getSequenceNumber(), userDataSizeInBytes, receiveBuffer->localDeviceId, receiveBuffer->queuePairNumber, receiveBuffer->sequenceNumber,
			receiveBuffer->userDataSize);

	queuePair->activate(receiveBuffer->localDeviceId, receiveBuffer->queuePairNumber, receiveBuffer->sequenceNumber, receiveBuffer->gid);
	queuePair->setRemoteUserData(receiveBuffer->userData, receiveBuffer->userDataSize);

	this->context->registerQueuePair(queuePair);

	free(receiveBuffer);
	free(sendBuffer);

	return queuePair;

}

QueuePair* QueuePairFactory::createLoopback(void *userData, uint32_t userDataSizeInBytes) {

	QueuePair *queuePair = new QueuePair(this->context);
	queuePair->activate(queuePair->getLocalDeviceId(), queuePair->getQueuePairNumber(), queuePair->getSequenceNumber(), queuePair->getLocalGid());
	queuePair->setRemoteUserData(userData, userDataSizeInBytes);

	this->context->registerQueuePair(queuePair);

	return queuePair;

}

bool QueuePairFactory::calculateIpAddress() {
	char *ipAddressOfDevice = infinity::utils::Address::getIpAddressOfInterface(infinity::core::Configuration::DEFAULT_NET_DEVICE);
	ipAddress = ipAddressOfDevice;
	free(ipAddressOfDevice);
	return true;
}

const bool getIp = QueuePairFactory::calculateIpAddress();

} /* namespace queues */
} /* namespace infinity */
