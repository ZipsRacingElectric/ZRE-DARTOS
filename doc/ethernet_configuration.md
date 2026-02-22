# Ethernet Configuration
In order for host computer to conenct to the DART (without any tedious configuration on said computer), the DART must do two main things:
- Have a static IP address
- Host a DHCP server

Fortunately both of these can done relatively easily with 3rd party software. Specifically, Dnsmasq and Network Manager are used.

## Static IP Configuration

The device can be assigned a static IP address via configuring the systemd-networkd service. The `/etc/systemd/network/` directory contains the configuration files for each network interface. As there is only one network interface on the DART (`eth0`), this is the only interface that needs configured.

In file `/etc/systemd/network/01-eth0.network`:
```
# Match this configuration against the 'eth0' device.
[Match]
Name=eth0

# Set the IP address on this interface to a known IP address.
[Network]
Address=192.168.0.1/24
Gateway=192.168.0.1
DNS=192.168.0.1
```

## DHCP Configuration

Dnsmasq can be installed via the `dnsmasq` package. The system service will be started upon installation.

By default, Dnsmasq only acts as a DNS server, not a DHCP server. As the DART is only intended to be used in a local context (network is not connected to the Internet), the DNS server functionality is not desired.

The desired settings for DHCP-only operation are provided below. For more details on these settings, refer to the Dnsmasq man pages (https://dnsmasq.org/docs/dnsmasq-man.html)

In file `/etc/dnsmasq.conf`:
```
# Setting the DNS port to 0 disables the DNS functionality of DNSMasq. We are only interested in DHCP functionality.
port=0

# This defines the range of IP addresses and the subnet mask.
dhcp-range=192.168.0.101,192.168.0.150,255.255,255.0,12h

# This disables advertising as a DNS server, we can chose an IP address as the default route option, however as there is no
# route to the Internet, we just set it to a bogus address that no device can occupy.
dhcp-option=3,192.168.0.50
```

## SSH Configuration

In SSH, the terms host and client are often used. In this case, host refers to the DART and client refers to any user of the DART.

SSH is, by default, not enabled on most linux distributions due to security reasons. To enabled SSH, the `ssh` package must be installed and the system daemon started.

```
systemctl start ssh
```

In the case of the DART, the typically convention of using SSH keys does not work. This is due to two reasons:
- Every user of the DART has unique SSH keys, but all should be granted access.
	- Normally, each host has a list of authorized keys that should be granted access.
- Every host has the same IP address, but unique SSH keys.
	- Normally, each host has a unique IP address in the same network.

To use the DART in this manner, some configuration must be done on both the host and client side.

### Host Configuration

To give a client access to the host server, the client's public key must be added to the host's `authorized_keys` file. While access can be granted via using a password, this is rather inconvenient.

To give multiple clients access to the host server, each individual client's key could be added to the `authorized_keys` file, or each individual client could use the *same* key for authentication.

**Note: In general, distributing SSH keys like this is very bad practice. In the case of the DART, it is only acceptable due to the fact the DART is isolated from the Internet and intended to be usable by anybody.**

With this simplification, there is only 1 SSH key that needs added to the host's `authorized_keys` file. The public/private key pair can then be distributed to any user of the DART. This key pair is located in the `src/zre_cantools/src/dart_cli/keys/` directory.

### Client Configuration

Upon connecting to an SSH host for the first time, a client will normally add the host's public key to the `known_hosts` file. This file is used to validate that any future connections to the same IP address are to the same host. If the host provides the incorrect public key, the client will reject the connection.

In the case of the DART, this is an issue, as every DART has the same IP address, but a unique key pair. If using the standard SSH configuration, the client will only ever be able to connect to the same instance of a DART and no others. This functionality is intended to prevent man-in-the-middle attacks, where a malicious actor pretends to be a host.

**Note: Disabling the `known_hosts` file is very bad practice. In the case of the DART, it is only acceptable due to the fact the DART is isolated from the Internet.**

To prevent the client from rejecting a connection to an unknown host, the `StrictHostKeyChecking` option can be set to `no`. This does not prevent the SSH client from rejecting an invalid entry in the `known_hosts` file, nor does it prevent SSH from writing the host's public key into said file upon connection.

To prevent the `known_hosts` file from being either read from or written to, the `UserKnownHostsFile` option can be used to point to a different file. By setting it to `/dev/null`, the file will always appear to be empty and immutable.

One slight annoyance with this approach is SSH will print a warning on every connection due to it accepting a new host public key. This warning can be suppressed with the `LogLevel=ERROR` option, where only errors are printed to the program's output.

As local, isolated networks are not susceptible to man-in-the-middle attacks, there is not much concern with disabling this functionality, however, there is much concern with disabling this functionality globally. There is no guarantee clients are not connecting to other SSH servers where security is a concern, so whatever client configuration is required for the DART can *only* be used when connecting to the DART.

To use a unique configuration for specific SSH sessions, the configuration may be specified via the client program's arguments, specifically with the `-o` option.

Summarized, a client can connect to the DART using the below command:

```
ssh -i %s/bin/keys/id_rsa
    -o StrictHostKeyChecking=no
    -o UserKnownHostsFile=/dev/null
    -o LogLevel=ERROR
    -o ConnectTimeout=4
    root@192.168.0.1
```

## System Booting

Normally, the ethernet interface of the DART is unconnected. Systemd will, by default, wait for the device's ethernet interface to come online before allowing other system services to start. This adds ~90 seconds to the boot time when no ethernet cable is connected, which is obviously unacceptable. To avoid this, the `systemd-networkd-wait-online.service` service can be disabled.

```
systemctl disable systemd-networkd-wait-online.service
```