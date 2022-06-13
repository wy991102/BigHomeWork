#!/bin/sh
sudo make 
sudo mknod /dev/demo_test c 236 0
sudo chmod 666 /dev/demo_test
sudo rmmod hello.ko
sudo insmod hello.ko
watch "dmesg | tail -20"

