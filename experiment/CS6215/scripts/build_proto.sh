#!/bin/bash
set -e

# Ensure Go binaries (like protoc-gen-go-grpc) are in PATH
export PATH="$(go env GOPATH)/bin:$PATH"

PROTO_DIR=./proto
OUT_DIR=.

protoc \
    --proto_path="$PROTO_DIR" \
    --go_out="$OUT_DIR" \
    --go-grpc_out="$OUT_DIR" \
    "$PROTO_DIR/ltlfuzz.proto"

echo "ltlfuzz build finish"
