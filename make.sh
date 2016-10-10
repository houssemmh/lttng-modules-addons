#!/bin/bash
make -j8
sudo make modules_install
sudo depmod -a
./control-addons.sh reload
