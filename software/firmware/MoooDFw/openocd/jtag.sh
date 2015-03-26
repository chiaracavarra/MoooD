#!/bin/bash

SCRIPT="$1"

OPENOCD_HOME="/home/marco/SoftwareProjects/st-usb/tools/openocd"
OPENOCD_BIN="${OPENOCD_HOME}/bin"
SCRIPTS_PATH="${OPENOCD_HOME}/share/openocd/scripts"
echo "OpenOCD home: ${OPENOCD_HOME}"

if [ "x${SCRIPT}" == "x" ]; then
  ${OPENOCD_BIN}/openocd -s ${SCRIPTS_PATH} -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg 
else
  ${OPENOCD_BIN}/openocd -s ${SCRIPTS_PATH} -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg -f ${SCRIPT}
fi
