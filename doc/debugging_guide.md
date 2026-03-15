# Debugging Guide

For debugging software/hardware issues, there are a number of options.

## Connection

### Via SSH

The most accessible method of connecting to the DART is via SSH. SSH allows a PC to directly control a shell the DART exposes, meaning the PC effectively has full control over the device (also due to the fact the default SSH user is the root account).

To open an SSH connection to the DART:
- Connect an ethernet cable between the device and your PC.
- Run the `dart-cli` application.
- Enter the `s` option to open an interactive SSH session.

From this point, all commands entered into the terminal are executed by the DART. Note that (as of the time of writing this) the DART uses Bash for its interactive shell

### Via USB

TODO(Barach): Not possible currently.

## Determining the Problem

Most issues with the DART tend to manifest as either the dashboard-gui, the can-mdf-logger, or both applications failing to start. The can-mdf-logger failing to start is indicated by a the dashboard-gui not reporting a data logging session number. The dashboard-gui failing to start is indicated by a lack of display.

To determine what caused the application(s) to fail, the init-system's journal can be checked:

`journalctl --no-pager -u init_system`
- Or use the `j` option in the `dart-cli`.

### Hardware Issues

If the issue appears to be a hardware issue, the kernel's message buffer can provide insight. This information can be checked using dmesg:

`dmesg`
- Or use the `d` option in the `dart-cli`

Due to the large volume of the output, it is useful to use `grep` to filter for specific text, for example:

- `dmesg | grep -iF "mcp"`
- `dmesg | grep -iF "can"`
- `dmesg | grep -iF "failed"`
- `dmesg | grep -iF "hdmi"`
- `dmesg | grep -iF "eth"`
- `dmesg | grep -iF "usb"`

While the kernel's message buffer can provide some insight, debugging with an oscilloscope or logic analyzer is often useful.

### Software Issues

For debugging software issues on the device, it is often useful to edit the firmware on the device. Before any changes can be made, the init-system, dashboard-gui, and can-mdf-logger must be stopped (if they are running). This can be done by:

`systemctl stop init_system`

This can be done via the `nano` text editor. To edit the init-system source code, for example:

`nano /root/init_system/main.c`

Ctrl+O can be used save the changes, followed by Ctrl+X to exit.

To make the changes take effect, the software must be recompiled. To recompile the init-system, for example:

`make -C /root/init_system/`

After recompilation, the software can restarted by:

`systemctl restart init_system`

For more involved changes, the firmware can be edited on a PC and copied onto the device. For this, reference the below guide.

[updating_firmware.md](updating_firmware.md)