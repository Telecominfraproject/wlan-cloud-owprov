<p align=center><img src=images/project/logo.svg?sanitize=true/ width="200px" height="200px"></p>

# OpenWiFi Provisioning

## OpenAPI
You may get static page with OpenAPI docs generated from the definition on [GitHub Page](https://telecominfraproject.github.io/wlan-cloud-owprov/).

Also, you may use [Swagger UI](https://petstore.swagger.io/#/) with OpenAPI definition file raw link (i.e. [latest version file](https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-owprov/main/openapi/owprov.yaml)) to get interactive docs page.

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

### OWPROV Service Configuration
The configuration is kept in a file called `owprov.properties`. To understand the content of this file,
please look [here](https://github.com/Telecominfraproject/wlan-cloud-owprov/blob/main/CONFIGURATION.md)

## Firewall Considerations
| Port  | Description                                    | Configurable |
|:------|:-----------------------------------------------|:------------:|
| 16004 | Default port for REST API Access to the OWPROV |     yes      |

## Kafka topics
Toe read more about Kafka, follow the [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/KAFKA.md)

## Contributions
We need more contributors. Should you wish to contribute,
please follow the [contributions](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/CONTRIBUTING.md) document.

## Pull Requests
Please create a branch with the Jira addressing the issue you are fixing or the feature you are implementing.
Create a pull-request from the branch into master.

## Additional OWSDK Microservices
Here is a list of additional OWSDK microservices
| Name | Description | Link | OpenAPI |
| :--- | :--- | :---: | :---: |
| OWSEC | Security Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec/blob/main/openpapi/owsec.yaml) |
| OWGW | Controller Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/openapi/owgw.yaml) |
| OWFMS | Firmware Management Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms/blob/main/openapi/owfms.yaml) |
| OWPROV | Provisioning Service | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov) | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov/blob/main/openapi/owprov.yaml) |
| OWANALYTICS | Analytics Service | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics) | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics/blob/main/openapi/owanalytics.yaml) |

