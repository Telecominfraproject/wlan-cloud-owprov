cmake_minimum_required(VERSION 3.13)
project(ucentraltopo VERSION 0.1.0)

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

if(SMALL_BUILD)
    find_package(Poco REQUIRED COMPONENTS Crypto JWT Net Util NetSSL Data DataSQLite)
else()
    find_package(CppKafka REQUIRED)
    find_package(PostgreSQL REQUIRED)
    find_package(MySQL REQUIRED)
    find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)
endif()

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka /usr/local/opt/mysql-client/include)

add_executable( ucentraltopo
        build
        src/Daemon.cpp src/Daemon.h
        src/Dashboard.h src/Dashboard.cpp
        src/MicroService.cpp src/MicroService.h
        src/Utils.h src/Utils.cpp
        src/RESTAPI_utils.cpp src/RESTAPI_utils.h
        src/RESTAPI_SecurityObjects.h src/RESTAPI_SecurityObjects.h
        src/RESTAPI_TopoObjects.cpp src/RESTAPI_TopoObjects.h
        src/ALBHealthCheckServer.h
        src/AuthClient.cpp src/AuthClient.h
        src/KafkaManager.cpp src/KafkaManager.h
        src/Kafka_topics.h
        src/OpenAPIRequest.cpp src/OpenAPIRequest.h
        src/RESTAPI_SecurityObjects.h src/RESTAPI_SecurityObjects.cpp
        src/RESTAPI_handler.cpp src/RESTAPI_handler.h
        src/RESTAPI_server.h src/RESTAPI_server.cpp
        src/RESTAPI_system_command.h src/RESTAPI_system_command.cpp
        src/RESTAPI_InternalServer.h src/RESTAPI_InternalServer.cpp
        src/StorageService.cpp src/StorageService.h
        src/SubSystemServer.cpp src/SubSystemServer.h
        src/storage_pgql.cpp src/storage_mysql.cpp src/storage_sqlite.cpp
        src/storage_tables.cpp
        src/uCentralTypes.h
        )

target_link_libraries(ucentraltopo PUBLIC
        ${Poco_LIBRARIES} ${MySQL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ZLIB_LIBRARIES} ${AWSSDK_LINK_LIBRARIES}
        CppKafka::cppkafka )

