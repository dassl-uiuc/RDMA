/**
 * Examples - Read/Write/Send Operations
 *
 * (c) 2018 Claude Barthels, ETH Zurich
 * Contact: claudeb@inf.ethz.ch
 *
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <thread>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/requests/RequestToken.h>

#define PORT_NUMBER 8011
#define SERVER_IP "192.168.6.1"
#define SERVER2_IP "192.168.6.3"

#define MSG_SIZE 64

using namespace std;
using std::chrono::high_resolution_clock;

// Usage: ./progam -s for server and ./program for client component
int main(int argc, char **argv) {

  bool isServer = false;

  while (argc > 1) {
    if (argv[1][0] == '-') {
      switch (argv[1][1]) {

        case 's': {
          isServer = true;
          break;
        }

      }
    }
    ++argv;
    --argc;
  }

  infinity::core::Context *context = new infinity::core::Context();
  infinity::queues::QueuePairFactory *qpFactory = new  infinity::queues::QueuePairFactory(context);
  infinity::queues::QueuePair *qp;
  infinity::queues::QueuePair *qp2;

  if(isServer) {

    printf("Creating buffers to read from and write to\n");
    infinity::memory::Buffer *bufferToReadWrite = new infinity::memory::Buffer(context, MSG_SIZE * 1024 * 1024 * sizeof(char));
    infinity::memory::RegionToken *bufferToken = bufferToReadWrite->createRegionToken();

    printf("Creating buffers to receive a message\n");
    infinity::memory::Buffer *bufferToReceive = new infinity::memory::Buffer(context, 128 * sizeof(char));
    context->postReceiveBuffer(bufferToReceive);

    printf("Setting up connection (blocking)\n");
    qpFactory->bindToPort(PORT_NUMBER);
    qp = qpFactory->acceptIncomingConnection(bufferToken, sizeof(infinity::memory::RegionToken));

    printf("Waiting for message (blocking)\n");
    infinity::core::receive_element_t receiveElement;
    while(!context->receive(&receiveElement));

    printf("Message received\n");
    delete bufferToReadWrite;
    delete bufferToReceive;

  } else {

    printf("Connecting to remote node\n");
    qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
    infinity::memory::RegionToken *remoteBufferToken = (infinity::memory::RegionToken *) qp->getUserData();

    //qp2 = qpFactory->connectToRemoteHost(SERVER2_IP, PORT_NUMBER);
    //infinity::memory::RegionToken *remoteBufferToken2 = (infinity::memory::RegionToken *) qp2->getUserData();

    auto rdma_write = [](infinity::queues::QueuePair *qp, infinity::core::Context *context, infinity::memory::RegionToken* remoteBufferToken) {

      printf("Creating buffers\n");
      infinity::memory::Buffer *buffer1Sided = new infinity::memory::Buffer(context, MSG_SIZE * sizeof(char));
      infinity::memory::Buffer *buffer2Sided = new infinity::memory::Buffer(context, 128 * sizeof(char));
    
      // Test buffer.
      printf("Test start\n");
      vector<uint32_t> v;
      v.push_back(5);
      v.push_back(8);
      infinity::memory::Buffer* testbuffer = new infinity::memory::Buffer(context, 2 * sizeof(uint32_t), v);
      uint32_t* dat = testbuffer->getIntData();
      std::cout << dat[0];
      std::cout << dat[1] << std::endl;
      printf("Test done");
  
      printf("Reading content from remote buffer\n");
      infinity::requests::RequestToken requestToken(context);
      qp->read(buffer1Sided, remoteBufferToken, &requestToken);
      requestToken.waitUntilCompleted();

      auto start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < 1024 * 1024; i++) {
        // printf("Writing content to remote buffer\n");
        qp->write(buffer1Sided, 0, remoteBufferToken, MSG_SIZE * i, MSG_SIZE, &requestToken);
        requestToken.waitUntilCompleted();
      }
      auto elapsed = std::chrono::high_resolution_clock::now() - start;
      long long microseconds = std::chrono::duration_cast<std::chrono::microseconds> (elapsed).count();
      printf("Microseconds are %lld", microseconds);
      printf("Sending message to remote host\n");
      qp->send(buffer2Sided, &requestToken);
      requestToken.waitUntilCompleted();

      delete buffer1Sided;
      delete buffer2Sided;
      delete testbuffer;
    };
   
    printf("Calculating the total time");
    auto start = std::chrono::high_resolution_clock::now(); 
    // Creating threads.
    thread t1(rdma_write, qp, context, remoteBufferToken);
    // thread t2(rdma_write, qp2, context, remoteBufferToken2);
    t1.join();
    // t2.join();
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds> (elapsed).count();
    printf("Total Microseconds are %lld", microseconds);
  }

  delete qp;
  if (!isServer) {
    delete qp2;
  }
  delete qpFactory;
  delete context;

  return 0;

}
