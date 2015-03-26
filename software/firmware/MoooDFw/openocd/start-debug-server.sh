#!/bin/bash

OPENOCD_HOME="/home/marco/SoftwareProjects/st-usb/tools/openocd/bin"
SCRIPTS_PATH="${OPENOCD_HOME}/../share/openocd/scripts"

$OPENOCD_HOME/openocd -s $SCRIPTS_PATH -f board/stm32f4discovery.cfg
