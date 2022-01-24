#!/bin/bash

# test.sh
export SUDO_ASKPASS=./_PWD_TEMP_
sudo -A /home/orangepi/oled_monitor/build/src/lv_app/lv_app 

echo "OK!"

exit 0
