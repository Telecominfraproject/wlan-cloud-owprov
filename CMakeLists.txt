cmake_minimum_required(VERSION 3.13)
project(owprov VERSION 2.2.0)

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
        src/Daemon.cpp src/Daemon.h
        src/Dashboard.h src/Dashboard.cpp
        src/framework/MicroService.cpp src/framework/MicroService.h
        src/framework/Utils.h src/framework/Utils.cpp
        src/framework/RESTAPI_utils.cpp src/framework/RESTAPI_utils.h
        src/RESTAPI/RESTAPI_SecurityObjects.h src/RESTAPI/RESTAPI_SecurityObjects.h
        src/framework/ALBHealthCheckServer.h
        src/framework/AuthClient.cpp src/framework/AuthClient.h
        src/framework/KafkaManager.cpp src/framework/KafkaManager.h
        src/framework/Kafka_topics.h
        src/framework/OpenAPIRequest.cpp src/framework/OpenAPIRequest.h
        src/RESTAPI/RESTAPI_SecurityObjects.h src/RESTAPI/RESTAPI_SecurityObjects.cpp
        src/framework/RESTAPI_handler.cpp src/framework/RESTAPI_handler.h
        src/RESTAPI/RESTAPI_server.h src/RESTAPI/RESTAPI_server.cpp
        src/framework/RESTAPI_system_command.h src/framework/RESTAPI_system_command.cpp
        src/RESTAPI/RESTAPI_InternalServer.h src/RESTAPI/RESTAPI_InternalServer.cpp
        src/StorageService.cpp src/StorageService.h
        src/framework/SubSystemServer.cpp src/framework/SubSystemServer.h
        src/RESTAPI/RESTAPI_ProvObjects.cpp src/RESTAPI/RESTAPI_ProvObjects.h
        src/storage/storage_entity.cpp src/storage/storage_entity.h
        src/storage/storage_policies.cpp src/storage/storage_policies.h
        src/framework/OpenWifiTypes.h
        src/storage/storage_venue.cpp src/storage/storage_venue.h
        src/storage/storage_location.cpp src/storage/storage_location.h
        src/storage/storage_contact.cpp src/storage/storage_contact.h
        src/RESTAPI/RESTAPI_entity_handler.cpp src/RESTAPI/RESTAPI_entity_handler.h
        src/storage/storage_inventory.cpp src/storage/storage_inventory.h
        src/RESTAPI/RESTAPI_contact_handler.cpp src/RESTAPI/RESTAPI_contact_handler.h
        src/RESTAPI/RESTAPI_location_handler.cpp src/RESTAPI/RESTAPI_location_handler.h
        src/RESTAPI/RESTAPI_venue_handler.cpp src/RESTAPI/RESTAPI_venue_handler.h
        src/RESTAPI/RESTAPI_inventory_handler.cpp src/RESTAPI/RESTAPI_inventory_handler.h
        src/RESTAPI/RESTAPI_managementPolicy_handler.cpp src/RESTAPI/RESTAPI_managementPolicy_handler.h
        src/RESTAPI/RESTAPI_inventory_list_handler.cpp src/RESTAPI/RESTAPI_inventory_list_handler.h
        src/RESTAPI/RESTAPI_entity_list_handler.cpp src/RESTAPI/RESTAPI_entity_list_handler.h
        src/storage/storage_management_roles.cpp src/storage/storage_management_roles.h
        src/framework/storage_setup.cpp
        src/SerialNumberCache.h src/SerialNumberCache.cpp
        src/storage/storage_configurations.cpp src/storage/storage_configurations.h
        src/RESTAPI/RESTAPI_configurations_handler.cpp src/RESTAPI/RESTAPI_configurations_handler.h
        src/RESTAPI/RESTAPI_webSocketServer.h src/RESTAPI/RESTAPI_webSocketServer.cpp src/RESTAPI/RESTAPI_contact_list_handler.cpp src/RESTAPI/RESTAPI_contact_list_handler.h src/RESTAPI/RESTAPI_location_list_handler.cpp src/RESTAPI/RESTAPI_location_list_handler.h src/RESTAPI/RESTAPI_venue_list_handler.cpp src/RESTAPI/RESTAPI_venue_list_handler.h src/RESTAPI/RESTAPI_managementPolicy_list_handler.cpp src/RESTAPI/RESTAPI_managementPolicy_list_handler.h src/RESTAPI/RESTAPI_managementRole_handler.cpp src/RESTAPI/RESTAPI_managementRole_handler.h src/RESTAPI/RESTAPI_managementRole_list_handler.cpp src/RESTAPI/RESTAPI_managementRole_list_handler.h src/RESTAPI/RESTAPI_configurations_list_handler.cpp src/RESTAPI/RESTAPI_configurations_list_handler.h src/SecurityDBProxy.cpp src/SecurityDBProxy.h src/APConfig.cpp src/APConfig.h src/framework/RESTAPI_errors.h src/ConfigurationValidator.cpp src/ConfigurationValidator.h src/framework/RESTAPI_GenericServer.cpp src/framework/RESTAPI_GenericServer.h src/AutoDiscovery.cpp src/AutoDiscovery.h src/framework/CIDRUtils.cpp src/framework/CIDRUtils.h src/SDK_stubs.cpp src/SDK_stubs.h src/ConfigSanityChecker.cpp src/ConfigSanityChecker.h src/storage/storage_tags.cpp src/storage/storage_tags.h src/TagServer.cpp src/TagServer.h src/framework/Storage.h
        src/framework/CountryCodes.h src/RESTAPI/RESTAPI_db_helpers.cpp src/RESTAPI/RESTAPI_db_helpers.h)

target_link_libraries(owprov PUBLIC
        ${Poco_LIBRARIES} ${MySQL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ZLIB_LIBRARIES} ${AWSSDK_LINK_LIBRARIES}
        CppKafka::cppkafka nlohmann_json_schema_validator)

