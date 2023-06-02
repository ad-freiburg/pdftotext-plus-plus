#!/bin/bash

# TODO: Write some general description.

DEFAULT_TMP_DIR="/tmp/pdftotext-plus-plus/install-requirements"
S=$(basename "$0")

# ==================================================================================================

# This function installs all requirements needed to execute 'make checkstyle'.
function make_checkstyle() {
  apt-get update && apt-get install -y python3
}

# This function installs all requirements needed to execute 'make compile'.
function make_compile() {
  local TARGET_DIR="${1:-.}"

  apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    libboost-all-dev \
    libfontconfig1-dev \
    libfreetype6-dev \
    libnss3-dev \
    libopenjp2-7-dev \
    libtiff5-dev

  install_tensorflow ${TARGET_DIR}
  install_cppflow ${TARGET_DIR}
  install_poppler ${TARGET_DIR}
  install_utf8proc ${TARGET_DIR}
}

# This function installs all requirements needed to execute 'make test'.
function make_test() {
  local TARGET_DIR="${1:-.}"
  make_compile ${TARGET_DIR}
  install_gtest ${TARGET_DIR}
}

# This function installs all requirements needed to execute 'make install'.
function make_install() {
  local TARGET_DIR="${1:-.}"
  make_compile ${TARGET_DIR}
}

# This function installs all requirements needed to execute 'make packages'.
function make_packages() {
  local TARGET_DIR="${1:-.}"
  make_compile ${TARGET_DIR}
  apt-get update && apt-get install -y tar lsb-release wget
}

# ==================================================================================================

# TODO: Documentation
function install_tensorflow() {
  local TARGET_DIR="${1:-.}"
  local VERSION="${2:-2.11.0}"
  local INCLUDE_DIR="$TARGET_DIR/include"
  local LIB_DIR="$TARGET_DIR/lib"
  local WORK_DIR="$DEFAULT_TMP_DIR/tensorflow-$(date +%s)"
  local TF_ARCHIVE_NAME="libtensorflow-cpu-linux-x86_64-${VERSION}.tar.gz"
  local ORIG_PWD="$(pwd)"

  info_emph "[$S] Installing the C library of tensorflow ..."
  debug " • TARGET_DIR:  '$TARGET_DIR'"
  debug " • VERSION:     '$VERSION'"
  debug " • INCLUDE_DIR: '$INCLUDE_DIR'"
  debug " • LIB_DIR:     '$LIB_DIR'"
  debug " • WORK_DIR:    '$WORK_DIR'"

  # Validate the arguments.
  [[ -z "$TARGET_DIR" ]] && { error "[$S] No path to target directory given."; }
  [[ -z "$VERSION" ]] && { error "[$S] No version given."; }

  # Create the directories.
  mkdir -p "$WORK_DIR"
  mkdir -p "$INCLUDE_DIR"
  mkdir -p "$LIB_DIR"

  # Ensure that the directory paths are absolute.
  WORK_DIR="$(realpath "$WORK_DIR")"
  TARGET_DIR="$(realpath "$TARGET_DIR")"
  INCLUDE_DIR="$(realpath "$INCLUDE_DIR")"
  LIB_DIR="$(realpath "$LIB_DIR")"

  cd "$WORK_DIR"

  info "[$S] Downloading '$TF_ARCHIVE_NAME' ..."
  wget "https://storage.googleapis.com/tensorflow/libtensorflow/$TF_ARCHIVE_NAME"

  info "[$S] Extracting '$TF_ARCHIVE_NAME' ..."
  tar -xzf "$TF_ARCHIVE_NAME" --no-same-owner

  info "[$S] Copying header files to '$INCLUDE_DIR' ..."
  cp -Pr "$WORK_DIR/include/." "$INCLUDE_DIR"

  info "[$S] Copying library files to '$LIB_DIR' ..."
  cp -Pr "$WORK_DIR/lib/." "$LIB_DIR"
  ldconfig "$LIB_DIR"

  cd "$ORIG_PWD"
  rm -rf "$WORK_DIR"
}

# ==================================================================================================

# TODO: Documentation
function install_cppflow() {
  local TARGET_DIR="${1:-.}"
  local GIT_BRANCH="${2:-99e04d7}"
  local GIT_REPO="https://github.com/serizba/cppflow.git"
  local INCLUDE_DIR="$TARGET_DIR/include"
  local LIB_DIR="$TARGET_DIR/lib"
  local WORK_DIR="$DEFAULT_TMP_DIR/cppflow-$(date +%s)"
  local ORIG_PWD="$(pwd)"

  info_emph "[$S] Installing cppflow ..."
  debug " • TARGET_DIR:  '$TARGET_DIR'"
  debug " • GIT_BRANCH:  '$GIT_BRANCH'"
  debug " • INCLUDE_DIR: '$INCLUDE_DIR'"
  debug " • LIB_DIR:     '$LIB_DIR'"
  debug " • WORK_DIR:    '$WORK_DIR'"

  # Validate the arguments.
  [[ -z "$TARGET_DIR" ]] && { error "[$S] No path to target directory given."; }
  [[ -z "$GIT_BRANCH" ]] && { error "[$S] No git branch given."; }

  # Create the directories.
  mkdir -p "$WORK_DIR"
  mkdir -p "$INCLUDE_DIR"
  mkdir -p "$LIB_DIR"

  # Ensure that the directory paths are absolute.
  WORK_DIR="$(realpath "$WORK_DIR")"
  TARGET_DIR="$(realpath "$TARGET_DIR")"
  INCLUDE_DIR="$(realpath "$INCLUDE_DIR")"
  LIB_DIR="$(realpath "$LIB_DIR")"

  cd "$WORK_DIR"

  info "[$S] Cloning repository '$GIT_REPO' ..."
  git clone "$GIT_REPO" .

  info "[$S] Checking out branch '$GIT_BRANCH' ..."
  git checkout "$GIT_BRANCH"

  info "[$S] Building cppflow ..."
  mkdir -p build
  cd build
  cmake \
    -DCMAKE_PREFIX_PATH="$TARGET_DIR" \
    -DCMAKE_INSTALL_PREFIX="." \
    -DBUILD_EXAMPLES=OFF ..
  make -j
  make install

  info "[$S] Copying header files to '$INCLUDE_DIR' ..."
  cp -Pr "$WORK_DIR/build/include/." "$INCLUDE_DIR"

  cd "$ORIG_PWD"
  rm -rf "$WORK_DIR"
}

# ==================================================================================================

# TODO: Documentation
function install_poppler() {
  local TARGET_DIR="${1:-.}"
  local GIT_BRANCH="${2:-065dca3}"
  # local GIT_REPO="https://github.com/freedesktop/poppler.git"
  local GIT_REPO="https://gitlab.freedesktop.org/poppler/poppler.git"
  local INCLUDE_DIR="$TARGET_DIR/include"
  local LIB_DIR="$TARGET_DIR/lib"
  local WORK_DIR="$DEFAULT_TMP_DIR/poppler-$(date +%s)"
  local ORIG_PWD=$(pwd)

  info_emph "[$S] Installing poppler ..."
  debug " • TARGET_DIR:  '$TARGET_DIR'"
  debug " • GIT_BRANCH:  '$GIT_BRANCH'"
  debug " • INCLUDE_DIR: '$INCLUDE_DIR'"
  debug " • LIB_DIR:     '$LIB_DIR'"
  debug " • WORK_DIR:    '$WORK_DIR'"

  # Validate the arguments.
  [[ -z "$TARGET_DIR" ]] && { error "[$S] No path to target directory given."; }
  [[ -z "$GIT_BRANCH" ]] && { error "[$S] No git branch given."; }

  # Create the directories.
  mkdir -p "$WORK_DIR"
  mkdir -p "$INCLUDE_DIR"
  mkdir -p "$LIB_DIR"

  # Ensure that the directory paths are absolute.
  WORK_DIR="$(realpath "$WORK_DIR")"
  TARGET_DIR="$(realpath "$TARGET_DIR")"
  INCLUDE_DIR="$(realpath "$INCLUDE_DIR")"
  LIB_DIR="$(realpath "$LIB_DIR")"

	cd "$WORK_DIR"

  info "[$S] Cloning repository '$GIT_REPO' ..."
  git clone "$GIT_REPO" .

  info "[$S] Checking out branch '$GIT_BRANCH' ..."
  git checkout "$GIT_BRANCH"

  info "[$S] Building poppler ..."
  mkdir -p build
	cd build
  cmake \
    -DBUILD_GTK_TESTS=OFF \
    -DBUILD_QT5_TESTS=OFF \
    -DBUILD_QT6_TESTS=OFF \
    -DBUILD_CPP_TESTS=OFF \
    -DBUILD_MANUAL_TESTS=OFF \
    -DENABLE_CPP=OFF \
    -DENABLE_GLIB=OFF \
    -DENABLE_GOBJECT_INTROSPECTION=OFF \
    -DENABLE_QT5=OFF \
    -DENABLE_QT6=OFF \
    -DENABLE_LIBCURL=OFF \
    -DRUN_GPERF_IF_PRESENT=OFF \
    -DENABLE_LIBPNG=OFF ..
  make poppler

  info "[$S] Copying header files to '$INCLUDE_DIR' ..."
  mkdir -p "$INCLUDE_DIR/poppler"
  mkdir -p "$INCLUDE_DIR/goo"
  find "$WORK_DIR/build" -maxdepth 1 -name "*.h" -exec cp -Pr "{}" "$INCLUDE_DIR" \;
  find "$WORK_DIR/build/poppler" -maxdepth 1 -name "*.h" -exec cp -Pr "{}" "$INCLUDE_DIR" \;
  find "$WORK_DIR/poppler" -maxdepth 1 -name "*.h" -exec cp -Pr "{}" "$INCLUDE_DIR/poppler" \;
  find "$WORK_DIR/goo" -maxdepth 1 -name "*.h" -exec cp -Pr "{}" "$INCLUDE_DIR/goo" \;

  info "[$S] Copying library files to '$LIB_DIR' ..."
  find "$WORK_DIR/build" -name "libpoppler.*" -exec cp -P "{}" "$LIB_DIR" \;
  ldconfig "$LIB_DIR"

  cd "$ORIG_PWD"
  rm -rf "$WORK_DIR"
}

# ==================================================================================================

# TODO: Documentation.
function install_utf8proc() {
  local TARGET_DIR="${1:-.}"
  local GIT_BRANCH="${2:-1cb28a6}"
  local GIT_REPO="https://github.com/JuliaStrings/utf8proc.git"
  local INCLUDE_DIR="$TARGET_DIR/include"
  local LIB_DIR="$TARGET_DIR/lib"
  local WORK_DIR="$DEFAULT_TMP_DIR/utf8proc-$(date +%s)"
  local ORIG_PWD="$(pwd)"

  info_emph "[$S] Installing utf8proc ..."
  debug " • TARGET_DIR:  '$TARGET_DIR'"
  debug " • GIT_BRANCH:  '$GIT_BRANCH'"
  debug " • INCLUDE_DIR: '$INCLUDE_DIR'"
  debug " • LIB_DIR:     '$LIB_DIR'"
  debug " • WORK_DIR:    '$WORK_DIR'"

  # Validate the arguments.
  [[ -z "$TARGET_DIR" ]] && { error "[$S] No path to target directory given."; }
  [[ -z "$GIT_BRANCH" ]] && { error "[$S] No git branch given."; }

  # Create the directories.
  mkdir -p "$WORK_DIR"
  mkdir -p "$INCLUDE_DIR"
  mkdir -p "$LIB_DIR"

  # Ensure that the directory paths are absolute.
  WORK_DIR="$(realpath "$WORK_DIR")"
  TARGET_DIR="$(realpath "$TARGET_DIR")"
  INCLUDE_DIR="$(realpath "$INCLUDE_DIR")"
  LIB_DIR="$(realpath "$LIB_DIR")"

  cd "$WORK_DIR"

  info "[$S] Cloning repository '$GIT_REPO' ..."
  git clone "$GIT_REPO" .

  info "[$S] Checking out branch '$GIT_BRANCH' ..."
  git checkout "$GIT_BRANCH"

  info "[$S] Building utf8proc ..."
  mkdir -p "build"
	cd build
  cmake ..
  make

  info "[$S] Copying header files to '$INCLUDE_DIR' ..."
  find "$WORK_DIR" -maxdepth 1 -name "*.h" -exec cp -Pr "{}" "$INCLUDE_DIR" \;

  info "[$S] Copying library files to '$LIB_DIR' ..."
  find "$WORK_DIR/build" -maxdepth 1 -name "*.a" -exec cp -Pr "{}" "$LIB_DIR" \;
  ldconfig "$LIB_DIR"

  cd "$ORIG_PWD"
  rm -rf "$WORK_DIR"
}

# ==================================================================================================

# TODO: Documentation.
function install_gtest() {
  local TARGET_DIR="${1:-.}"
  local GIT_BRANCH="${2:-b796f7d}"
  local GIT_REPO="https://github.com/google/googletest.git"
  local INCLUDE_DIR="$TARGET_DIR/include"
  local LIB_DIR="$TARGET_DIR/lib"
  local WORK_DIR="$DEFAULT_TMP_DIR/gtest-$(date +%s)"
  local ORIG_PWD="$(pwd)"

  info_emph "[$S] Installing gtest ..."
  debug " • TARGET_DIR:  '$TARGET_DIR'"
  debug " • GIT_BRANCH:  '$GIT_BRANCH'"
  debug " • INCLUDE_DIR: '$INCLUDE_DIR'"
  debug " • LIB_DIR:     '$LIB_DIR'"
  debug " • WORK_DIR:    '$WORK_DIR'"

  # Validate the arguments.
  [[ -z "$TARGET_DIR" ]] && { error "[$S] No path to target directory given."; }
  [[ -z "$GIT_BRANCH" ]] && { error "[$S] No git branch given."; }

  # Create the directories.
  mkdir -p "$WORK_DIR"
  mkdir -p "$INCLUDE_DIR"
  mkdir -p "$LIB_DIR"

  # Ensure that the directory paths are absolute.
  WORK_DIR="$(realpath "$WORK_DIR")"
  TARGET_DIR="$(realpath "$TARGET_DIR")"
  INCLUDE_DIR="$(realpath "$INCLUDE_DIR")"
  LIB_DIR="$(realpath "$LIB_DIR")"

  cd "$WORK_DIR"

  info "[$S] Cloning repository '$GIT_REPO' ..."
  git clone "$GIT_REPO" .

  info "[$S] Checking out branch '$GIT_BRANCH' ..."
  git checkout "$GIT_BRANCH"

  info "[$S] Building gtest ..."
  mkdir -p build
  cd build
  cmake ..
  make

  info "[$S] Copying header files to '$INCLUDE_DIR' ..."
  cp -Pr "$WORK_DIR/googletest/include/." "$INCLUDE_DIR"

  info "[$S] Copying library files to '$LIB_DIR' ..."
  cp -Pr "$WORK_DIR/build/lib/." "$LIB_DIR"
  ldconfig "$LIB_DIR"

  cd "$ORIG_PWD"
  rm -rf "$WORK_DIR"
}

# ==================================================================================================

# Change the directory before executing the script, so that all relative paths specified in the
# script are interpreted relative to the script (and not to the current working directory).
# After executing the script, change back to the original directory (so that the user does not
# notice the directory change).
ORIG_PWD=$(pwd)
PARENT_PATH=$(cd "$(dirname "$BASH_SOURCE[0]")" ; pwd -P)
cd $PARENT_PATH

# Exit when any command fails.
set -e

source log.sh

cd $ORIG_PWD

"$@"
