#!/bin/sh
set -e
cd "$(dirname "$0")/../.."
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/src -w /src emscripten/emsdk:3.1.74 sh -c   "emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release -DSTOCKKEEPER_BUILD_TESTS=OFF && cmake --build build-wasm --target stockkeeper_web"
mkdir -p web/src/wasm
cp build-wasm/stockkeeper_web.mjs build-wasm/stockkeeper_web.wasm web/src/wasm/
