echo stop > /sys/class/remoteproc/remoteproc0/state
echo echo_test.elf  > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state
/home/fpga/rpmsg-echo-test/build/rpmsg_echo_test
