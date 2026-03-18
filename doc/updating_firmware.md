# Updating Firmware

Note: This guide assumes the user has ZRE-CAN-Tools installed on the system's path.

## Updating ZRE-CAN-Tools

To update ZRE-CAN-Tools, the dart-cli can be used:

```
dart-cli --update-zre-cantools=/path/to/zre_cantools/
```

This command will move the DART's version of ZRE-CAN-Tools to a backup directory (`/root/zre_cantools_backup`), copy the target version of ZRE-CAN-Tools onto the DART, and compile said version. If at any point the process fails, the dart-cli will attempt to recover the backup. If this process fails, it is the responsibility of the user to manually restore the backup.

## Updating the Init-system

To update the init-system, the dart-cli can be used:

```
dart-cli --update-zre-cantools=/path/to/init_system/
```

**Important: The directory being copied MUST be named "init_system", otherwise the process will fail.**

This command will move the DART's version of the init-system to a backup directory (`/root/init_system_backup`), copy the target version of the init-system onto the DART, and compile said version. If at any point the process fails, the dart-cli will attempt to recover the backup. If this process fails, it is the responsibility of the user to manually restore the backup.