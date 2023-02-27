<p align="center">
    <img src="images/project/logo.svg" width="200"/>
</p>

# OpenWiFi Provisioning Service (OWPROV)
## What is it?
The OWPROV is a service for the TIP OpenWiFi CloudSDK (OWSDK).
OWPROV manages groups of access points through the use of entities and vanues. OWPROV, like all other OWSDK microservices, is
defined using an OpenAPI definition and uses the ucentral communication protocol to interact with Access Points. To use
the OWPROV, you either need to [build it](#building) or use the [Docker version](#docker).

## OpenAPI
You may get static page with OpenAPI docs generated from the definition on [GitHub Page](https://telecominfraproject.github.io/wlan-cloud-owprov/).
Also, you may use [Swagger UI](https://petstore.swagger.io/#/) with OpenAPI definition file raw link (i.e. [latest version file](https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-owprov/main/openapi/owprov.yaml)) to get interactive docs page.

## Building
To build the microservice from source, please follow the instructions in [here](./BUILDING.md)

## Docker
To use the CLoudSDK deployment please follow [here](https://github.com/Telecominfraproject/wlan-cloud-ucentral-deploy)

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

#### Expected directory layout
From the directory where your cloned source is, you will need to create the `certs`, `logs`, and `uploads` directories.
```bash
mkdir certs
mkdir certs/cas
mkdir logs
mkdir uploads
```
You should now have the following:
```text
--+-- certs
  |   +--- cas
  +-- cmake
  +-- cmake-build
  +-- logs
  +-- src
  +-- test_scripts
  +-- openapi
  +-- uploads
  +-- owsec.properties
```

### Certificate
The OWFMS uses a certificate to provide security for the REST API Certificate to secure the Northbound API.

#### The `certs` directory
For all deployments, you will need the following `certs` directory, populated with the proper files.

```text
certs ---+--- restapi-ca.pem
         +--- restapi-cert.pem
         +--- restapi-key.pem
```

## Firewall Considerations
| Port  | Description                                    | Configurable |
|:------|:-----------------------------------------------|:------------:|
| 16004 | Default port for REST API Access to the OWPROV |     yes      |

### Environment variables
The following environment variables should be set from the root directory of the service. They tell the OWGW process where to find
the configuration and the root directory.
```bash
export OWGW_ROOT=`pwd`
export OWGW_CONFIG=`pwd`
```
You can run the shell script `set_env.sh` from the microservice root.

### OWPROV Service Configuration
The configuration is kept in a file called `owprov.properties`. To understand the content of this file,
please look [here](https://github.com/Telecominfraproject/wlan-cloud-owprov/blob/main/CONFIGURATION.md)

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
| OWSUB | Subscriber Service | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal) | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal/blob/main/openapi/userportal.yaml) |

