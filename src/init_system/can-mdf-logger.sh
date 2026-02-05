#!/bin/sh

# Note: Do not forget the 'exec' command. This command replaces the current shell instance with the process to execute, meaning
# all received signals are forwarded to the new process.
exec $ZRE_CANTOOLS_DIR/bin/can-mdf-logger ~/mdf/ $ZRE_CANTOOLS_DIR/config/zr25_glory/logger_vehicle_config.json can0@1000000
