# Compile the gRPC proto library
add_library(proto_grpc
     ../../gen/ltlfuzz.pb.cc
     ../../gen/ltlfuzz.grpc.pb.cc
 )

target_include_directories(proto_grpc PUBLIC
    ${CMAKE_SOURCE_DIR}/gen
    ${GRPC_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
)

target_link_libraries(proto_grpc PUBLIC
    ${PROTOBUF_LIBRARIES}
    ${GRPC_LIBRARIES}
)

add_executable(ltlfuzz_grpc_server
    main.cc
    ltlfuzzServiceImpl.cc
)

target_include_directories(ltlfuzz_grpc_server PRIVATE
    ${GRPC_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/gen
)

target_link_libraries(ltlfuzz_grpc_server PRIVATE
    proto_grpc
    automata
    pthread
    spot
    bddx
    rt
    boost_system
)
