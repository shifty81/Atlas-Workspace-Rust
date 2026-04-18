# AtlasWorkspace — CI Build Container
# Provides a reproducible Linux build environment for Atlas Workspace.

FROM ubuntu:24.04 AS base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

FROM base AS build
RUN cmake -S . -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DATLAS_BUILD_TESTS=ON \
    -DATLAS_ENABLE_ONLINE_DEPS=ON \
    && cmake --build build --parallel

FROM build AS test
RUN cd build && ctest --output-on-failure
