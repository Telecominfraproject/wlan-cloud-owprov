cmake_minimum_required(VERSION 3.13)
project(owprov VERSION 2.4.0)

set(CMAKE_CXX_STANDARD 17)

if(UNIX AND APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    set(MYSQL_ROOT_DIR /usr/local/opt/mysql-client)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

if(UNIX AND NOT APPLE)
    set(PostgreSQL_TYPE_INCLUDE_DIR /usr/include/postgresql)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build)
    file(READ build BUILD_NUM)
    if(BUILD_INCREMENT)
        MATH(EXPR BUILD_NUM "${BUILD_NUM}+1")
        file(WRITE build ${BUILD_NUM})
    endif()
else()
    set(BUILD_NUM 1)
    file(WRITE build ${BUILD_NUM})
endif()

set(BUILD_SHARED_LIBS 1)
add_definitions(-DAPP_VERSION="${CMAKE_PROJECT_VERSION}" -DBUILD_NUMBER="${BUILD_NUM}" -DAWS_CUSTOM_MEMORY_MANAGEMENT)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost      REQUIRED system)
find_package(OpenSSL    REQUIRED)
find_package(AWSSDK     REQUIRED COMPONENTS s3)
find_package(Poco       REQUIRED COMPONENTS Crypto JWT Net Util NetSSL Data DataSQLite)
find_package(nlohmann_json  REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)

if(SMALL_BUILD)
    find_package(Poco REQUIRED COMPONENTS Crypto JWT Net Util NetSSL Data DataSQLite)
else()
    find_package(CppKafka REQUIRED)
    find_package(PostgreSQL REQUIRED)
    find_package(MySQL REQUIRED)
    find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)
endif()

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka /usr/local/opt/mysql-client/include)

add_executable(owprov
        build
        src/framework/CountryCodes.h
        src/framework/KafkaTopics.h
        src/framework/MicroService.h
        src/framework/OpenWifiTypes.h
        src/framework/orm.h
        src/framework/RESTAPI_errors.h
        src/framework/RESTAPI_protocol.h
        src/framework/StorageClass.h
        src/framework/uCentral_Protocol.h
        src/framework/ConfigurationValidator.cpp
        src/framework/ConfigurationValidator.h
        src/RESTObjects/RESTAPI_SecurityObjects.h src/RESTObjects/RESTAPI_SecurityObjects.cpp
        src/RESTObjects/RESTAPI_ProvObjects.cpp src/RESTObjects/RESTAPI_ProvObjects.h
        src/RESTObjects/RESTAPI_GWobjects.h src/RESTObjects/RESTAPI_GWobjects.cpp
        src/RESTObjects/RESTAPI_FMSObjects.h src/RESTObjects/RESTAPI_FMSObjects.cpp
        src/APIServers.cpp
        src/Daemon.cpp src/Daemon.h
        src/Dashboard.h src/Dashboard.cpp
        src/StorageService.cpp src/StorageService.h
        src/storage/storage_entity.cpp src/storage/storage_entity.h
        src/storage/storage_policies.cpp src/storage/storage_policies.h
        src/storage/storage_venue.cpp src/storage/storage_venue.h
        src/storage/storage_location.cpp src/storage/storage_location.h
        src/storage/storage_contact.cpp src/storage/storage_contact.h
        src/storage/storage_inventory.cpp src/storage/storage_inventory.h
        src/storage/storage_management_roles.cpp src/storage/storage_management_roles.h
        src/storage/storage_configurations.cpp src/storage/storage_configurations.h
        src/storage/storage_tags.cpp src/storage/storage_tags.h
        src/RESTAPI/RESTAPI_entity_handler.cpp src/RESTAPI/RESTAPI_entity_handler.h
        src/RESTAPI/RESTAPI_contact_handler.cpp src/RESTAPI/RESTAPI_contact_handler.h
        src/RESTAPI/RESTAPI_location_handler.cpp src/RESTAPI/RESTAPI_location_handler.h
        src/RESTAPI/RESTAPI_venue_handler.cpp src/RESTAPI/RESTAPI_venue_handler.h
        src/RESTAPI/RESTAPI_inventory_handler.cpp src/RESTAPI/RESTAPI_inventory_handler.h
        src/RESTAPI/RESTAPI_managementPolicy_handler.cpp src/RESTAPI/RESTAPI_managementPolicy_handler.h
        src/RESTAPI/RESTAPI_inventory_list_handler.cpp src/RESTAPI/RESTAPI_inventory_list_handler.h
        src/RESTAPI/RESTAPI_entity_list_handler.cpp src/RESTAPI/RESTAPI_entity_list_handler.h
        src/RESTAPI/RESTAPI_configurations_handler.cpp src/RESTAPI/RESTAPI_configurations_handler.h
        src/RESTAPI/RESTAPI_webSocketServer.h src/RESTAPI/RESTAPI_webSocketServer.cpp
        src/RESTAPI/RESTAPI_contact_list_handler.cpp src/RESTAPI/RESTAPI_contact_list_handler.h
        src/RESTAPI/RESTAPI_location_list_handler.cpp src/RESTAPI/RESTAPI_location_list_handler.h
        src/RESTAPI/RESTAPI_venue_list_handler.cpp src/RESTAPI/RESTAPI_venue_list_handler.h
        src/RESTAPI/RESTAPI_managementPolicy_list_handler.cpp src/RESTAPI/RESTAPI_managementPolicy_list_handler.h
        src/RESTAPI/RESTAPI_managementRole_handler.cpp src/RESTAPI/RESTAPI_managementRole_handler.h
        src/RESTAPI/RESTAPI_managementRole_list_handler.cpp src/RESTAPI/RESTAPI_managementRole_list_handler.h
        src/RESTAPI/RESTAPI_configurations_list_handler.cpp src/RESTAPI/RESTAPI_configurations_list_handler.h
        src/SerialNumberCache.h src/SerialNumberCache.cpp
        src/SecurityDBProxy.cpp src/SecurityDBProxy.h
        src/APConfig.cpp src/APConfig.h
        src/AutoDiscovery.cpp src/AutoDiscovery.h
        src/SDK_stubs.cpp src/SDK_stubs.h
        src/ConfigSanityChecker.cpp src/ConfigSanityChecker.h
        src/TagServer.cpp src/TagServer.h
        src/RESTAPI/RESTAPI_db_helpers.h
        src/JobController.cpp src/JobController.h
        src/JobRegistrations.cpp
        src/storage/storage_jobs.cpp src/storage/storage_jobs.h src/WebSocketClientServer.cpp src/WebSocketClientServer.h src/storage/storage_maps.cpp src/storage/storage_maps.h src/RESTAPI/RESTAPI_map_handler.cpp src/RESTAPI/RESTAPI_map_handler.h src/RESTAPI/RESTAPI_map_list_handler.cpp src/RESTAPI/RESTAPI_map_list_handler.h)

target_link_libraries(owprov PUBLIC
        ${Poco_LIBRARIES} ${MySQL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ZLIB_LIBRARIES} ${AWSSDK_LINK_LIBRARIES}
        CppKafka::cppkafka nlohmann_json_schema_validator)

