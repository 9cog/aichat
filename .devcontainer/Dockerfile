FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
        doxygen \
        libreadline-dev \
        ninja-build \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*
