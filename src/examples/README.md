# Running examples on cloudlab machines.

This tutorial will help you run the examples in this directory.
The sequencer and read-write-send have been used for performance tests in the past.

Please make sure to update the Configuration.h with the correct name of the interface.
Also, depending on which device the RDMA NIC is hosted, you would also need to change the constructor of the Context class.

The device id corresponds to the index in the list of the RDMA devices.

You should run the following comnad -

sarthakm@node-0:~/RDMA$ sudo /opt/mellanox/iproute2/sbin/rdma link

link mlx5_0/1 state ACTIVE physical_state LINK_UP netdev eno33

link mlx5_1/1 state DOWN physical_state DISABLED netdev eno34

link mlx5_2/1 state ACTIVE physical_state LINK_UP netdev enp65s0f0

link mlx5_3/1 state DOWN physical_state DISABLED netdev enp65s0f1

Here, for instance, we used mlx5_2/1 for our tests, which corresponds to index 2 in the entire list. (This is 0 indexed)
Therefore, the device id in Context would be 2 here.

xl170 machines sometimes had mlx5_3 as the link which was UP, in that case, the device ID would be set to 3.

After making these changes and installing the drivers etc, you can optionally do a ibv_pingpong test the details of which are mentioned in the document linked
to the README in RDMA directory.

After compiling the library, these performance runs can be reproduced!
