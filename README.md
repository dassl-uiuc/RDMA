# Getting to run RDMA on Cloudlab.

First, we need to have machines with either ConnectX4 or ConnectX5 NIC connected to the same switch.
Then, read and use the install.sh script in this directory to install the drivers and reboot the cloudlab machines.

A detailed documentation about debugging steps and progress about installation can be found here -
https://docs.google.com/document/d/1KKPbAVcWff8ao5jKimIWggsZqlWtJh6IUnaEktA2gto/edit?pli=1

This also contains a genilib script to create the xl170 cloudlab machines.

There are few files in the /src/examples directory which are built over the RDMA infrastructure.

The sequencer file there is a custom sequencer which runs as a closed loop. The system is multi-threaded and a quick read of the code will enable you to modify the code to use appropriate number of threads and number of requests per thread. Relevant documentation is present in the examples directory.


# Original documentation starts here.

# Infinity - A lightweight C++ RDMA library for InfiniBand

Infinity is a simple, powerful, object-oriented abstraction of ibVerbs. The library enables users to build sophisticated applications that use Remote Direct Memory Access (RDMA) without sacrificing performance. It significantly lowers the barrier to get started with RDMA programming. Infinity provides support for two-sided (send/receive) as well as one-sided (read/write/atomic) operations. The library is written in C++ and has been ported to Rust ([Infinity-Rust](https://github.com/utaal/infinity-rust/)) by @utaal.

## Installation

Installing ''ibVerbs'' is a prerequisite before building Infinity. The output is located in ''release/libinfinity.a''.

```sh
$ make library # Build the library
$ make examples # Build the examples
```
## Using Infinity

Using Infinity is straight-forward and requires only a few lines of C++ code.

```C
// Create new context
infinity::core::Context *context = new infinity::core::Context();

// Create a queue pair
infinity::queues::QueuePairFactory *qpFactory = new  infinity::queues::QueuePairFactory(context);
infinity::queues::QueuePair *qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);

// Create and register a buffer with the network
infinity::memory::Buffer *localBuffer = new infinity::memory::Buffer(context, BUFFER_SIZE);

// Get information from a remote buffer
infinity::memory::RegionToken *remoteBufferToken = new infinity::memory::RegionToken(REMOTE_BUFFER_INFO);

// Read (one-sided) from a remote buffer and wait for completion
infinity::requests::RequestToken requestToken(context);
qp->read(localBuffer, remoteBufferToken, &requestToken);
requestToken.waitUntilCompleted();

// Write (one-sided) content of a local buffer to a remote buffer and wait for completion
qp->write(localBuffer, remoteBufferToken, &requestToken);
requestToken.waitUntilCompleted();

// Send (two-sided) content of a local buffer over the queue pair and wait for completion
qp->send(localBuffer, &requestToken);
requestToken.waitUntilCompleted();

// Close connection
delete remoteBufferToken;
delete localBuffer;
delete qp;
delete qpFactory;
delete context;
```

## Citing Infinity in Academic Publications

This library has been created in the context of my work on parallel and distributed join algorithms. Detailed project descriptions can be found in two papers published at ACM SIGMOD 2015 and VLDB 2017. Further publications concerning the use of RDMA have been submitted to several leading systems conferences and are currently under review. Therefore, for the time being, please refer to the publications listed below when referring to this library.

Claude Barthels, Simon Loesing, Gustavo Alonso, Donald Kossmann.
**Rack-Scale In-Memory Join Processing using RDMA.**
*Proceedings of the 2015 ACM SIGMOD International Conference on Management of Data, June 2015.*  
**PDF:** http://barthels.net/publications/barthels-sigmod-2015.pdf


Claude Barthels, Ingo Müller, Timo Schneider, Gustavo Alonso, Torsten Hoefler.
**Distributed Join Algorithms on Thousands of Cores.**
*Proceedings of the VLDB Endowment, Volume 10, Issue 5, January 2017.*  
**PDF:** http://barthels.net/publications/barthels-vldb-2017.pdf

---

```
@inproceedings{barthels-sigmod-2015,
  author    = {Claude Barthels and
               Simon Loesing and
               Gustavo Alonso and
               Donald Kossmann},
  title     = {Rack-Scale In-Memory Join Processing using {RDMA}},
  booktitle = {{SIGMOD}},
  pages     = {1463--1475},
  year      = {2015},
  url       = {http://doi.acm.org/10.1145/2723372.2750547}
}
```



```
@article{barthels-pvldb-2017,
  author    = {Claude Barthels and
               Ingo M{\"{u}}ller and
               Timo Schneider and
               Gustavo Alonso and
               Torsten Hoefler},
  title     = {Distributed Join Algorithms on Thousands of Cores},
  journal   = {{PVLDB}},
  volume    = {10},
  number    = {5},
  pages     = {517--528},
  year      = {2017},
  url       = {http://www.vldb.org/pvldb/vol10/p517-barthels.pdf}
}
```

