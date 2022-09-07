cmake_minimum_required(VERSION 3.10)

project(transport-catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

set(HEADER_FILES "domain.h" "geo.h" "graph.h" "json_builder.h" "json_reader.h" "json.h" "map_renderer.h" "ranges.h" "request_handler.h" "router.h"
                "serialization.h" "svg.h" "transport_catalogue.h" "transport_router.h")

add_executable(transport_catalogue 
    ${PROTO_SRCS} 
    ${PROTO_HDRS}
    main.cpp
    json_reader.cpp
    domain.cpp
    transport_catalogue.cpp
    map_renderer.cpp
    request_handler.cpp
    svg.cpp
    json.cpp
    geo.cpp
    json_builder.cpp
    transport_router.cpp
    serialization.cpp
    ${HEADER_FILES}
    )

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads) 

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

