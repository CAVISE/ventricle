FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        build-essential=12.10ubuntu1 \
        ca-certificates=20260601~24.04.1 \
        cmake=3.28.3-1build7 \
        git=1:2.43.0-1ubuntu7.3 \
        libabsl-dev=20220623.1-3.1ubuntu3.2 \
        libboost-all-dev=1.83.0.1ubuntu2 \
        libssl-dev=3.0.13-0ubuntu3.16 \
        pkg-config=1.8.1-2build1 \
        python3=3.12.3-0ubuntu2.1 \
        python3-venv=3.12.3-0ubuntu2.1 \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/ci \
    && /opt/ci/bin/pip install --no-cache-dir \
        clang-format==22.1.8 \
        clang-tidy==22.1.8 \
        eclipse-sumo==1.27.1 \
        meson==1.12.1 \
        ninja==1.13.2

ENV SUMO_HOME="/opt/ci/lib/python3.12/site-packages/sumo"
ENV CMAKE_PREFIX_PATH="${SUMO_HOME}"
ENV LD_LIBRARY_PATH="${SUMO_HOME}/lib64"
ENV PATH="/opt/ci/bin:${PATH}"
