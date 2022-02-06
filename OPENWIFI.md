# OpenWiFi integration flags

## Default Device Configuration from provisioning
In order to enable the GW to provide default configurations from the provisioning service,
the GW must have the following configuration:

```
autoprovisioning.process = prov,default
```
This will tell the GW to ask provisioning for the configuration file. If no file exists, it will use the default configuration file.
```
autoprovisioning.process = default
```
This will tell the GW to only use default file configurations. This is the default.

## Automatic Firmware Management
### FMS configuration
In order to have FMS update devices automatically or follow the update policy, the following must be set 
in the FMS configuration file.
```
autoupdater.enabled = true
```
The default value is `false`
### Provisioning configuration
The default settings can be set in the Provisioning service configuration:
``` 
firmware.updater.upgrade = false
firmware.updater.releaseonly = false
```
#### firmware.updater.upgrade
This selects whether upgrades are allowed by default or not. The default is `false`.
#### firmware.updater.releaseonly
The selects whether upgrades should consider all released firmware or just firmware marked as release. The default is `false`.
### Firmware upgrade rules
Th default rules are only used when no rules are set at the entity, venue, or device level. These settings always supresede aany default values.


