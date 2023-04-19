#!/bin/bash

USR_TMP_DIR="./usr"

USR_BIN_DIR="/usr/bin"
USR_LIB_DIR="/usr/lib/pdftotext-plus-plus"
USR_RESOURCES_DIR="/usr/share/pdftotext-plus-plus/resources"

# ==================================================================================================

apt-get update
apt-get install -y build-essential cmake git tar wget

wget https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64 -O /usr/bin/yq
chmod +x /usr/bin/yq

make requirements/pre USR_DIR="$USR_TMP_DIR"
make requirements/run USR_DIR="$USR_TMP_DIR"
ldconfig "$USR_TMP_DIR"

make clean compile USR_DIR="$USR_TMP_DIR" RESOURCES_DIR="$USR_RESOURCES_DIR"

mkdir -p "$USR_LIB_DIR"
cp -Pa "$USR_TMP_DIR/lib/." "$USR_LIB_DIR"
ldconfig "$USR_LIB_DIR"

mkdir -p "$USR_RESOURCES_DIR"
cp -Pa "./resources/." "$USR_RESOURCES_DIR"

cp ./build/pdftotext++ "$USR_BIN_DIR"
chmod +x "$USR_BIN_DIR/pdftotext++"