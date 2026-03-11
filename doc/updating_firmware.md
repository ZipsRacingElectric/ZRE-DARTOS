# Updating Firmware

Note: This guide assumes the user has a development version (git repo & source code) of ZRE-CAN-Tools installed and ZRE-CAN-Tools is present in the system's path.

The commands used in this guide are written for a POSIX shell, however they can be executed in Windows by substituting the environment variables.

- For command prompt, replace `$VARIABLE` with `%VARIABLE%`.
- For powershell, replace `$VARIABLE` with `$Env:VARIABLE`.

## Updating ZRE-CAN-Tools

To update the DART's version of ZRE-CAN-Tools, the target version can simply be copied onto the device and compiled.

```
# Stop the init-system
dart-cli --ssh -- root@192.168.0.1 'systemctl stop init_system'

# Backup the DART's version of ZRE-CAN-Tools
dart-cli --ssh -- root@192.168.0.1 'mv /root/zre_cantools/ /root/zre_cantools_backup/'

# Copy the target version of ZRE-CAN-Tools onto the DART
dart-cli --ssh -- root@192.168.0.1 'mkdir -p /root/zre_cantools'
dart-cli --scp -- -r $ZRE_CANTOOLS_DIR/config/ root@192.168.0.1:/root/zre_cantools/
dart-cli --scp -- -r $ZRE_CANTOOLS_DIR/src/ root@192.168.0.1:/root/zre_cantools/
dart-cli --scp -- -r $ZRE_CANTOOLS_DIR/lib/ root@192.168.0.1:/root/zre_cantools/
dart-cli --scp -- $ZRE_CANTOOLS_DIR/include.mk root@192.168.0.1:/root/zre_cantools/
dart-cli --scp -- $ZRE_CANTOOLS_DIR/makefile root@192.168.0.1:/root/zre_cantools/

# Compile ZRE-CAN-Tools on the device
dart-cli --ssh -- root@192.168.0.1 make -C /root/zre_cantools/

# Restart the init-system
dart-cli --ssh -- root@192.168.0.1 'systemctl restart init_system'

# Delete the old version of ZRE-CAN-Tools
dart-cli --ssh -- root@192.168.0.1 'rm -r /root/zre_cantools_backup/'
```

The above commands assume the installed version of ZRE-CAN-Tools is the target version to be copied to the DART. To use a version that isn't currently installed, the `$ZRE_CANTOOLS_DIR` variable can be replaced with the desired directory.

If the file transfer or compilation fails, the DART's old version of ZRE-CAN-Tools can be restored by:

```
# Delete the failed version of ZRE-CAN-Tools
dart-cli --ssh -- root@192.168.0.1 'rm -r /root/zre_cantools/'

# Restore the old version
dart-cli --ssh -- root@192.168.0.1 'mv /root/zre_cantools_backup/ /root/zre_cantools/'

# Restart the init-system
dart-cli --ssh -- root@192.168.0.1 'systemctl restart init_system'
```