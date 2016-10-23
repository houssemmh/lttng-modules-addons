#!/bin/bash
make -j8
sudo insmod addons/lttng-meminfo.ko
#sudo make modules_install
#sudo depmod -a
#./control-addons.sh reload
