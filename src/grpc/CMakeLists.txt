# include_directories(${GRPC_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})
# link_directories(${GRPC_LIBRARY_DIRS} ${PROTOBUF_LIBRARY_DIRS})
#
# add_library(proto_grpc
#     ../../gen/ltlfuzz.pb.cc
#     ../../gen/ltlfuzz.grpc.pb.cc
# )
#
#
# add_executable(ltlfuzz_grpc_server main.cc ltlfuzzServiceImpl.cc)
# target_link_libraries(ltlfuzz_grpc_server proto_grpc automata_core)
#
# Include the generated gRPC headers (ltlfuzz.pb.h, ltlfuzz.grpc.pb.h)
include_directories(
    ${GRPC_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/gen   # 👈 this is what was missing
)

link_directories(
    ${GRPC_LIBRARY_DIRS}
    ${PROTOBUF_LIBRARY_DIRS}
)

# Compile the gRPC proto library
add_library(proto_grpc
    ../../gen/ltlfuzz.pb.cc
    ../../gen/ltlfuzz.grpc.pb.cc
)

# Build the gRPC server binary
add_executable(ltlfuzz_grpc_server
    main.cc
    ltlfuzzServiceImpl.cc
)

# Link everything together
target_link_libraries(ltlfuzz_grpc_server
    proto_grpc
    automata
    ${PROTOBUF_LIBRARIES}
    ${GRPC_LIBRARIES}
    pthread
)
