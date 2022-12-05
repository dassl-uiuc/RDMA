# This script lets you install the required drivers to setup RDMA.

cd /opt

sudo wget http://www.mellanox.com/downloads/ofed/MLNX_OFED-5.3-1.0.0.1/MLNX_OFED_LINUX-5.3-1.0.0.1-ubuntu18.04-x86_64.tgz

sudo tar -xzvf MLNX_OFED_LINUX-5.3-1.0.0.1-ubuntu18.04-x86_64.tgz
cd MLNX_OFED_LINUX-5.3-1.0.0.1-ubuntu18.04-x86_64
sudo ./mlnxofedinstall --auto-add-kernel-support --without-fw-update

sudo /etc/init.d/openibd restart

# After this step, we must reboot the machine for the networks to reflect
# in the ifconfig.
# Now, we can make the RDMA directory, change the Configuration.h file
# with the name of the network interface on which RDMA is hosted.
# You must also change the index in the Context.h file constructor.
# This index is the index at which your NIC is hosted. More details are present in the
# README which links to the detailed documentation and also in the README inside the 
# src/examples directory where an example is illustrated.

# Now you can get started!!
