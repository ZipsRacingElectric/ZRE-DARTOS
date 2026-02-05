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

SSH is, by default, not enabled on most linux distributions due to security reasons. To enabled SSH, the `ssh` package must be installed and the system daemon started.

```
systemctl start ssh
```

TODO(Barach): Pub/priv key