/**
 * This file builds a sequencer over RDMA.
 *
 * (c) 2022 Sarthak Moorjani, UIUC
 *
 * (c) 2018 Claude Barthels, ETH Zurich
 * Contact: claudeb@inf.ethz.ch
 *
 */

#include <atomic>
#include <iostream>
#include <map>
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
#define PORT_NUMBER2 8012
#define SERVER_IP "192.168.6.1"
#define SERVER2_IP "192.168.6.3"

#define MSG_SIZE 64

std::atomic<int> counter(0);

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

    auto rdma_server = [] (int port) {

      infinity::core::Context *context = new infinity::core::Context();
      infinity::queues::QueuePairFactory *qpFactory = new  infinity::queues::QueuePairFactory(context);

      printf("Creating buffers to read from and write to\n");
      // infinity::memory::Buffer *bufferToReadWrite = new infinity::memory::Buffer(context, MSG_SIZE * 1024 * 1024 * sizeof(char));

      vector<uint32_t> v(2, 0);
      printf("Creating buffers to receive a message\n");
      infinity::memory::Buffer *bufferToReceive = new infinity::memory::Buffer(context, 2 * sizeof(uint32_t), v);
      infinity::memory::RegionToken *bufferToken = bufferToReceive->createRegionToken();
      uint32_t* initdata = bufferToReceive->getIntData();

      map<uint32_t, infinity::queues::QueuePair*> clientid_map;

      printf("Setting up connection (blocking)\n");
      qpFactory->bindToPort(port);

      int n = 1;
      infinity::core::receive_element_t receiveElement;
      infinity::queues::QueuePair *qps[n];
      for (int i = 0; i < n; i++) {
        qps[i] = qpFactory->acceptIncomingConnection(bufferToken, sizeof(infinity::memory::RegionToken));
        printf("Waiting for the first message (blocking)\n");
        context->postReceiveBuffer(bufferToReceive, true /* int */);
        while(!context->receive(&receiveElement));
        printf("Checking what we received!");
        uint32_t* recvdata = bufferToReceive->getIntData();
        std::cout << recvdata[0];
        std::cout << recvdata[1] << std::endl;
        printf("Message received\n");
        clientid_map[recvdata[0]] = qps[i];
      }

      vector<uint32_t> vsend(2, 0);
      infinity::memory::Buffer* sendbuffer = new infinity::memory::Buffer(context, 2 * sizeof(uint32_t), vsend);
      infinity::requests::RequestToken requestToken(context);
      for (int i = 0; i < n; i++) {
        printf("Sending back first message to client");
        qps[i]->send(sendbuffer, &requestToken, true /* is_int */);
        requestToken.waitUntilCompleted();
      }

      int m = 1;
      auto start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < n * m * 1000 * 1000; i++) {
        context->postReceiveBuffer(receiveElement.buffer, true /* int */);
        while(!context->receive(&receiveElement));
        uint32_t* recvdata = bufferToReceive->getIntData();
        //std::cout << recvdata[0] << endl;
        counter++;
        sendbuffer->UpdateIntMemory(0, counter);
        clientid_map[recvdata[0]]->send(sendbuffer, &requestToken, true /* is_int */);
        requestToken.waitUntilCompleted();
      }

      auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds> (elapsed).count();
      printf("Total Microseconds are %lld", microseconds);

      printf("Message received\n");
      //delete bufferToReadWrite;
      delete bufferToReceive;
      delete sendbuffer;
      //delete buffer2Sided;
    };

    int n = 24;
    thread mythreads[n];
    for (int i = 0; i < n; i++) {
      mythreads[i] = std::thread(rdma_server, 8011 + i);
    }

    for (int i = 0; i < n; i++) {
      mythreads[i].join();
    }

  } else {

    //qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
    //infinity::memory::RegionToken *remoteBufferToken = (infinity::memory::RegionToken *) qp->getUserData();

    //qp2 = qpFactory->connectToRemoteHost(SERVER2_IP, PORT_NUMBER);
    //infinity::memory::RegionToken *remoteBufferToken2 = (infinity::memory::RegionToken *) qp2->getUserData();

    auto rdma_write = [](uint32_t id) {

      infinity::core::Context *context = new infinity::core::Context();
      infinity::queues::QueuePairFactory *qpFactory = new  infinity::queues::QueuePairFactory(context);

      printf("Connecting to remote node\n");
      infinity::queues::QueuePair *qpin = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
      infinity::memory::RegionToken *remoteBufferToken = (infinity::memory::RegionToken *) qpin->getUserData();
      printf("Creating buffers\n");
      infinity::memory::Buffer *buffer1Sided = new infinity::memory::Buffer(context, MSG_SIZE * sizeof(char));
      infinity::memory::Buffer *buffer2Sided = new infinity::memory::Buffer(context, 128 * sizeof(char));

      // Test buffer.
      printf("Test start\n");
      vector<uint32_t> v;
      v.push_back(id);
      v.push_back(8);
      infinity::memory::Buffer* testbuffer = new infinity::memory::Buffer(context, 2 * sizeof(uint32_t), v);
      uint32_t* dat = testbuffer->getIntData();
      std::cout << dat[0];
      std::cout << dat[1] << std::endl;
      printf("Test done");

      //printf("Reading content from remote buffer\n");
      //qp->read(buffer1Sided, remoteBufferToken, &requestToken);
      //requestToken.waitUntilCompleted();

      infinity::requests::RequestToken requestToken(context);
      printf("Sending first message to remote host\n");
      qpin->send(testbuffer, &requestToken, true /* is_int */);
      requestToken.waitUntilCompleted();

      infinity::memory::Buffer *receiveBuffer = new infinity::memory::Buffer(context, 2 * sizeof(uint32_t), v);
      context->postReceiveBuffer(receiveBuffer, true /* int */);
      printf("Waiting for the first message from server (blocking)\n");
      infinity::core::receive_element_t receiveElement;
      while(!context->receive(&receiveElement));
      printf("Checking what we received!");
      uint32_t* recvdata = receiveBuffer->getIntData();
      std::cout << recvdata[0];
      std::cout << recvdata[1] << std::endl;

      auto start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < 1000 * 1000; i++) {
        //printf("Sending message again");
        //testbuffer->UpdateIntMemory(0, i);
        qpin->send(testbuffer, &requestToken, true /* is_int */);
        requestToken.waitUntilCompleted();
        context->postReceiveBuffer(receiveElement.buffer, true /* is_int*/);
        while(!context->receive(&receiveElement));
        recvdata = receiveBuffer->getIntData();
        //std::cout << recvdata[0] << endl;
      }
      std::cout << "Final token" << recvdata[0] << endl;
      auto elapsed = std::chrono::high_resolution_clock::now() - start;
      long long microseconds = std::chrono::duration_cast<std::chrono::microseconds> (elapsed).count();
      printf("Total Microseconds are %lld", microseconds);

      delete receiveBuffer;
      delete buffer1Sided;
      delete buffer2Sided;
      delete testbuffer;
      delete qpin;
    };

    printf("Calculating the total time");
    // Creating threads.
    thread t1(rdma_write, 0);
    thread t2(rdma_write, 1);
    t1.join();
    t2.join();
  }

  delete qp;
  if (!isServer) {
    delete qp2;
  }
  delete qpFactory;
  delete context;

  return 0;

}
