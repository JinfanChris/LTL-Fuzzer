protoc --proto_path=proto --cpp_out=gen --grpc_out=gen --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) proto/ltlfuzz.proto

