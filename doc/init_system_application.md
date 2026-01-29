# The Init-System Application

// TODO(Barach): Waaaay too text heavy.

The init-system is responsible for integrating both the GUI and the data logging applications. The main goal of the init-system is to launch said applications when the device boots and terminate said applications when the system is powering down. The device’s power down sequence is signaled by the power input subsystem via hardware, hence the need to isolate this functionality in the init-system. The init-system is also responsible for bringing the CAN interface hardware online and configuring it for the correct baudrates. Even though multiple applications can share access to this hardware, only one application may perform this initialization. Because of this, the data logger and GUI are designed to expect this initialization be performed before they are launched.

![init_system_software_flow_chart.png](./init_system_software_flow_chart.png)