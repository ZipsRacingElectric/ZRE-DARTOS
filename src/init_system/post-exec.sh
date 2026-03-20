#!/bin/sh

# TODO(Barach): Mounting the root filesystem as readonly will help prevent file corruption.

# Halt the operating system forcefully.
# - This sends the kill signal to all system services and unmounts all filesystems.
# - This does not physically power off the device, just halts all CPU action.
systemctl halt --force