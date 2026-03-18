#!/bin/sh

# This shell script starts the dashboard-gui application.

# As we are not in a desktop environment, no wayland compositor is running yet. To start a graphical application, the 'cage'
# application is used to first start the wayland compositor.

# TODO(Barach): Better docs.

# TODO(Barach): multiple CAN channels

# Note: Do not forget the 'exec' command. This command replaces the current shell instance with the process to execute, meaning
# all received signals are forwarded to the new process.
exec cage -- $ZRE_CANTOOLS_DIR/bin/dashboard-gui --fullscreen $DART_CONFIG/dashboard_gui.json can0@1000000 $DART_CONFIG/main.dbc
