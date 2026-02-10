#!/bin/sh

# This shell script starts the dashboard-gui application.

# Note: Do not forget the 'exec' command. This command replaces the current shell instance with the process to execute, meaning
# all received signals are forwarded to the new process.
exec cage -- $ZRE_CANTOOLS_DIR/bin/dashboard-gui app can0@1000000 $ZRE_CANTOOLS_DIR/config/zr25_glory/can_vehicle.dbc
