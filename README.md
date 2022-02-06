# OpenWiFi Provisioning

## Build from source.
You need:
- https://github.com/pboettch/json-schema-validator.git
- https://github.com/nlohmann/json.git

build and install them.

## Root entity
It's UUID value is 0000-0000-0000. Its parent entity must be empty.

## Entity
### Creation rules
- You must set the parent of an entity.
- The only properties you may set at creation are:
  - name
  - description
  - notes
  - parent

### Modification rules
You may modify the following fields in the POST
- name
- description
- notes

### Delete
- Children must be empty

## Inventory Tags
### Creation rules
- Entity must point to an existing non-root entity
- If you associate a venue, it must exist.
- You must use an existing device type. Device type cannot be empty.
- Name, description, notes are allowed.

### Modification rules
- You can modify the device type to another valid one.

## Venue
### Creation rules
- If you include an entity, the parent must bot be set
- if you include a parent, the entity must not be set
- You cannot have children upon creation.
- You may include an array of devices UUIDs
- Topology and design cannot be set

## Geocoding
To support geocoding help, you need to configuration the following in the configuration file. Geocoding is used
when creating location and when reporting analytics.
``` 
geocodeapi = google
google.apikey = **********************************
```
Currently, only google Geocoding is supported. Additional methods may be added in the future.

## Default firmware management rules
FMS is already integrated with OpenWifi. In order to allow it to upgrade devices automatically, you should 
set the following values.
``` 
firmware.updater.upgrade = <true/false>
firmware.updater.releaseonly = <true/false>
```
### firmware.updater.upgrade
Should FMS attempt to upgrade devices by default.

### firmware.updater.releaseonly
Should only RC software be used during upgrades.