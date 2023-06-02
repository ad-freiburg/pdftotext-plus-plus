#!/bin/bash

# TODO: Write some general description.

# The default path to a temporary directory (a directory where transient files can be stored).
DEFAULT_TMP_DIR="/tmp/pdftotext-plus-plus/package"
S=$(basename "$0")

# ==================================================================================================

# This function parses the specified directory for Dockerfiles (each of which is supposed to build
# a package for a specific distribution). The expected file name of a Dockerfile in this directory
# is "Dockerfile.package.<dist-name>" where <dist-name> is a unique string describing the
# distribution for which the Dockerfile builds a package, for example: "ubuntu2004-amd64".
# Executes each such Dockerfile. The built packages are stored in the directory specified by "$4".
#
# Args:
#   $1 - The absolute or relative path to a directory containing Dockerfiles (each of which is
#        supposed to build a package for a specific distribution).
#        NOTE: If the path is specified relative, it must be relative to this script.
#   $2 - The name of the project.
#   $3 - The name of the binary.
#        NOTE: This name will be stored in the package's control file, under the key "Package".
#   $4 - The description of the project.
#        NOTE: This name will be stored in the package's control file, under the key "Description".
#   $5 - The current version of the project.
#        NOTE: This name will be stored in the package's control file, under the key "Version".
#   $6 - The name of the maintainer of the project.
#        NOTE: This name will be stored in the package's control file, under the key "Maintainer".
#   $7 - The mail of the maintainer of the project.
#        NOTE: This name will be stored in the package's control file, under the key "Maintainer".
#   $8 - The apt packages that need to be installed on the user's system in order to run the binary.
#   $9 - The absolute or relative path to the directory where the built packages should be stored.
#        NOTE: If the path is specified relative, it must be relative to this script.
#
# Example usages:
#   build_packages "./Dockerfiles.packages" "pdftotext-plus-plus" "pdftotext++" "Description."
#       "0.3.1" "Hein Blöd" "blöd@doof.com" "libboost-all-dev libfontconfig1-dev"
#       "/data/pdftotext++/packages"
#
function build_packages() {
  local DOCKERFILES_DIR="$1"
  local PROJECT_NAME="$2"
  local BINARY_NAME="$3"
  local PROJECT_DESCRIPTION="$4"
  local PROJECT_VERSION="$5"
  local MAINTAINER_NAME="$6"
  local MAINTAINER_MAIL="$7"
  local APT_RUN_REQUIREMENTS="$8"
  local TARGET_DIR="$9"

  info_emph "[$S] Building packages ..."
  debug " • DOCKERFILES_DIR:      '$DOCKERFILES_DIR'"
  debug " • PROJECT_NAME:         '$PROJECT_NAME'"
  debug " • BINARY_NAME:          '$BINARY_NAME'"
  debug " • PROJECT_DESCRIPTION:  '$PROJECT_DESCRIPTION'"
  debug " • PROJECT_VERSION:      '$PROJECT_VERSION'"
  debug " • MAINTAINER_NAME:      '$MAINTAINER_NAME'"
  debug " • MAINTAINER_MAIL:      '$MAINTAINER_MAIL'"
  debug " • APT_RUN_REQUIREMENTS: '$APT_RUN_REQUIREMENTS'"
  debug " • TARGET_DIR:           '$TARGET_DIR'"
  debug " • DOCKERFILES:"
  for DOCKERFILE in ${DOCKERFILES_DIR}/Dockerfile.packaging.*; do
      debug "     • $(basename $DOCKERFILE)"
  done

  # Iterate through the Dockerfiles and use each to build a package for a specific distribution.
  for DOCKERFILE in ${DOCKERFILES_DIR}/Dockerfile.packaging.*; do
      build_package "$(echo $DOCKERFILE | cut -d. -f4)" "$DOCKERFILE" "$BINARY_NAME" \
          "$PROJECT_DESCRIPTION" "$PROJECT_VERSION" "$MAINTAINER_NAME" "$MAINTAINER_MAIL" \
          "$APT_RUN_REQUIREMENTS" "$TARGET_DIR"
  done
}

# This function builds a Docker image from the specified Dockerfile and starts a Docker container
# over this image for building a DEB package of the current version of pdftotext++. The built
# package is specific to the distribution used in the Docker container. The built package is stored
# in the directory specified by "$9".
#
# Args:
#   $1 - An arbitrary but unique name of the distribution (for example: "ubuntu1804").
#        NOTE: This name becomes part of the name of the Docker image and Docker container.
#   $2 - The absolute or relative path to the Dockerfile.
#        NOTE: If the path is specified relative, it must be relative to this script.
#   $3 - The name of the package (for example: "pdftotext++").
#        NOTE: This name is passed to the Dockerfile and becomes part of the package's control file.
#   $4 - The description of the package.
#        NOTE: This description is passed to the Dockerfile and becomes part of the control file.
#   $5 - The version of the package.
#        NOTE: This version is passed to the Dockerfile and becomes part of the control file.
#   $6 - The name of the project's maintainer.
#        NOTE: This name is passed to the Dockerfile and becomes part of the control file.
#   $7 - The mail of the project's maintainer.
#        NOTE: This mail is passed to the Dockerfile and becomes part of the control file.
#   $8 - A string containing the names of the APT packages that are required for compiling *and*
#        running pdftotext++. It is expected that the package names are separated by commas, for
#        example: "libboost-all-dev, libfontconfig1".
#        NOTE: This string is passed to the Dockerfile and becomes part of the control file.
#   $9 - The absolute or relative path to the directory where the built package should be stored.
#        NOTE: If the path is specified relative, it must be relative to this script.
#
# Example usage:
#   build_package "ubuntu1804" "./Dockerfile/Dockerfile.ubuntu1804" "pdftotext++" "A description." \
#      "0.3.1" "Claudius Korzen" "korzen@cs.uni-freiburg.de" "libboost-all-dev, libfontconfig1" \
#      "/local/data/pdftotext++/packages"
#
function build_package() {
  local M="build-package"
  local DIST_NAME="$1"
  local DOCKERFILE="$2"
  local PACKAGE_NAME="$3"
  local PACKAGE_DESCRIPTION="$4"
  local PACKAGE_VERSION="$5"
  local MAINTAINER_NAME="$6"
  local MAINTAINER_MAIL="$7"
  local APT_RUN_REQUIREMENTS="$8"
  local TARGET_DIR="$9"
  local D="$DIST_NAME"
  local DOCKER_IMAGE_NAME="pdftotext-plus-plus.package:$DIST_NAME"  # TODO: Parameterize?
  local DOCKER_CONTAINER_NAME="pdftotext-plus-plus.package.$DIST_NAME"  # TODO: Parameterize?

  info_emph "[$S/$M][$D] Building package ..."
  debug " • DIST_NAME:             '$DIST_NAME'"
  debug " • DOCKERFILE:            '$DOCKERFILE'"
  debug " • PACKAGE_NAME:          '$PACKAGE_NAME'"
  debug " • PACKAGE_DESCRIPTION:   '$PACKAGE_DESCRIPTION'"
  debug " • PACKAGE_VERSION:       '$PACKAGE_VERSION'"
  debug " • MAINTAINER_NAME:       '$MAINTAINER_NAME'"
  debug " • MAINTAINER_MAIL:       '$MAINTAINER_MAIL'"
  debug " • APT_RUN_REQUIREMENTS:  '$APT_RUN_REQUIREMENTS'"
  debug " • TARGET_DIR:            '$TARGET_DIR'"
  debug " • DOCKER_IMAGE_NAME:     '$DOCKER_IMAGE_NAME'"
  debug " • DOCKER_CONTAINER_NAME: '$DOCKER_CONTAINER_NAME'"

  # Validate the arguments.
  [[ -z "$DIST_NAME" ]] && { error "[$S/$M][$D] No distribution name given."; }
  [[ -z "$DOCKERFILE" ]] && { error "[$S/$M][$D] No Dockerfile given."; }
  [[ -z "$PACKAGE_NAME" ]] && { error "[$S/$M][$D] No project name given."; }
  [[ -z "$PACKAGE_DESCRIPTION" ]] && { error "[$S/$M][$D] No project description given."; }
  [[ -z "$PACKAGE_VERSION" ]] && { error "[$S/$M][$D] No project version given."; }
  [[ -z "$MAINTAINER_NAME" ]] && { error "[$S/$M][$D] No maintainer name given."; }
  [[ -z "$MAINTAINER_MAIL" ]] && { error "[$S/$M][$D] No maintainer mail given."; }
  [[ -z "$APT_RUN_REQUIREMENTS" ]] && { error "[$S/$M][$D] No APT run requirements given."; }
  [[ -z "$TARGET_DIR" ]] && { error "[$S/$M][$D] No path to the target directory given."; }
  [[ -z "$DOCKER_IMAGE_NAME" ]] && { error "[$S/$M][$D] No name of the image given."; }
  [[ -z "$DOCKER_CONTAINER_NAME" ]] && { error "[$S/$M][$D] No name of the container given."; }
  [[ ! -f "$DOCKERFILE" ]] && { error "[$S/$M][$D] Dockerfile '$DOCKERFILE' does not exist."; }

  # Build a Docker image from the specified Dockerfile.
  info "[$S/$M][$D] Building a Docker image from the Dockerfile ..."
  docker build -f "$DOCKERFILE" -t "$DOCKER_IMAGE_NAME" .

  # Start a Docker container over the built image, and let the container create the package.
  info "[$S/$M][$D] Creating and running the Docker container ..."
  mkdir -p -m 777 "$TARGET_DIR"
  docker run --rm --name "$DOCKER_CONTAINER_NAME" \
    -e PACKAGE_NAME="$PACKAGE_NAME" \
    -e PACKAGE_DESCRIPTION="$PACKAGE_DESCRIPTION" \
    -e PACKAGE_VERSION="$PACKAGE_VERSION" \
    -e PACKAGE_MAINTAINER_NAME="$MAINTAINER_NAME" \
    -e PACKAGE_MAINTAINER_MAIL="$MAINTAINER_MAIL" \
    -e PACKAGE_APT_RUN_REQUIREMENTS="$APT_RUN_REQUIREMENTS" \
    -v "$(realpath $TARGET_DIR)":/out \
    "$DOCKER_IMAGE_NAME"
}

# This function builds a DEB package of pdftotext++, specific to the distribution and processor
# architecture used on the machine on which this function is executed. The package is stored under
# "$8/$(define_deb_package_name $1 $3 <codename> <arch>).deb, where "<codename>" is the codename of
# the distribution (for example: "bionic" or "jammy") and "<arch>" is the processor architecture
# (for example: "i386" or "amd64").
#
# NOTE: This function is called by the different "./Dockerfiles/Dockerfile.package.*" files, each
# of which is supposed to build a package of pdftotext++ for a different distribution.
#
# Args:
#   $1 - The project name (for example: "pdftotext++").
#        NOTE: This name will be stored under the key "Package" in the package's control file.
#   $2 - The project description.
#        NOTE: This description will be stored under the key "Description" in the control file.
#   $3 - The project version (for example: "0.3.1").
#        NOTE: This version will be stored under the key "Version" in the control file.
#   $4 - The name of the project's maintainer.
#   $5 - The mail of the project's maintainer.
#        NOTE: The name and mail of the maintainer will be stored under the key "Maintainer" in
#        the control file, in the format "$4 <$5>"
#   $6 - A string containing the names of the APT packages which are required to compile and run
#        pdftotext++. It is expected that the names are separated by commas, for example:
#        "libboost-all-dev, libfontconfig1".
#        NOTE 1: It is expected that these packages were already installed in a previous step.
#        This function does *not* install the specified packages.
#        NOTE 2: This string is stored under the "Depends" key in the packages's control file.
#        It must be equal to the string specified under the "Depends" key in the Packages file
#        associated with the package in the APT repository (this file is created by the apt-repo.sh
#        script). If one or more of the specified packages are not installed on the target system,
#        the command "apt-get install pdftotext++" refuses to install the pdftotext++ package.
#   $7 - The absolute or relative path to a directory which contains the header files and shared
#        libraries that are required to compile pdftotext++ and that are not available from the
#        standard APT repositories of Ubuntu (for example, Tensorflow or Poppler).
#        NOTE 1: It is expected that this directory contains an "include" subdirectory (containing
#        the header files) and a "lib" subdirectory (containing the library files).
#        NOTE 2: It is expected that the libraries in the "lib" subdictionary are also required
#        for running pdftotext++. All libraries are therefore stored as part of the package, in the
#        directory specified by the $PKG_LIB_DIR_REL variable below.
#   $8 - The absolute or relative path to the directory where the built package should be stored.
#        NOTE: If the path is specified relative, it must be relative to this script.
#
# Example usage:
#   build_deb_package "pdftotext++" "A description." "0.3.1" "Claudius Korzen" \
#      "korzen@cs.uni-freiburg.de" "libboost-all-dev libfontconfig1" "/usr/local" \
#      "/local/data/pdftotext++/packages"
#
function build_deb_package() {
  local M="build-deb-package"
  local PROJECT_NAME="$1"
  local PROJECT_DESCRIPTION="$2"
  local PROJECT_VERSION="$3"
  local MAINTAINER_NAME="$4"
  local MAINTAINER_MAIL="$5"
  local APT_RUN_REQUIREMENTS="$6"
  local USR_DIR="$7"
  local TARGET_DIR="$8"
  local MAINTAINER="$MAINTAINER_NAME <$MAINTAINER_MAIL>"
  local CODENAME="$(lsb_release -cs)"
  local DISTRIBUTION="$(lsb_release -is) $(lsb_release -rs)"  # for example: "Ubuntu 22.04"
  local ARCH="$(dpkg --print-architecture)"
  local PKG_NAME="$(define_deb_package_name "$PROJECT_NAME" "$PROJECT_VERSION" "$CODENAME" "$ARCH")"
  local TMP_BUILD_DIR="$DEFAULT_TMP_DIR/build-$(date +%s)"
  local TMP_PACKAGE_DIR="$DEFAULT_TMP_DIR/$PKG_NAME"
  # The following three paths are specified relative to $TMP_PACKAGE_DIR.
  local PKG_CONTROL_FILE_REL="/DEBIAN/control"    # The path to the package's control file.
  local PKG_TRIGGERS_FILE_REL="/DEBIAN/triggers"  # The path to the package's triggers file.
  local PKG_SO_CONF_FILE_REL="/etc/ld.so.conf.d/pdftotext++.conf"  # The *.so.conf file.
  local PKG_BIN_DIR_REL="/usr/bin"                # The directory where to put the main binary.
  local PKG_LIB_DIR_REL="/usr/lib/pdftotext-plus-plus"  # The directory where to put the libraries.
  local PKG_RES_DIR_REL="/usr/share/pdftotext-plus-plus/resources" # The dir where to put resources.

  info_emph "[$S/$M] Building (native) DEB package ..."
  debug " • PACKAGE_NAME:          '$PACKAGE_NAME'"
  debug " • PACKAGE_DESCRIPTION:   '$PACKAGE_DESCRIPTION'"
  debug " • PACKAGE_VERSION:       '$PACKAGE_VERSION'"
  debug " • MAINTAINER_NAME:       '$MAINTAINER_NAME'"
  debug " • MAINTAINER_MAIL:       '$MAINTAINER_MAIL'"
  debug " • APT_RUN_REQUIREMENTS:  '$APT_RUN_REQUIREMENTS'"
  debug " • USR_DIR:               '$USR_DIR'"
  debug " • TARGET_DIR:            '$TARGET_DIR'"
  debug " • CODENAME:              '$CODENAME'"
  debug " • DISTRIBUTION:          '$DISTRIBUTION'"
  debug " • ARCH:                  '$ARCH'"
  debug " • PKG_NAME:              '$PKG_NAME'"
  debug " • TMP_BUILD_DIR:         '$TMP_BUILD_DIR'"
  debug " • TMP_PACKAGE_DIR:       '$TMP_PACKAGE_DIR'"
  debug " • PKG_CONTROL_FILE_REL:  '$PKG_CONTROL_FILE_REL'"
  debug " • PKG_TRIGGERS_FILE_REL: '$PKG_TRIGGERS_FILE_REL'"
  debug " • PKG_SO_CONF_FILE_REL:  '$PKG_SO_CONF_FILE_REL'"
  debug " • PKG_BIN_DIR_REL:       '$PKG_BIN_DIR_REL'"
  debug " • PKG_LIB_DIR_REL:       '$PKG_LIB_DIR_REL'"
  debug " • PKG_RES_DIR_REL:       '$PKG_RES_DIR_REL'"

  # Validate the arguments.
  [[ -z "$PACKAGE_NAME" ]] && { error "[$S/$M] No project name given."; }
  [[ -z "$PACKAGE_DESCRIPTION" ]] && { error "[$S/$M] No package description given."; }
  [[ -z "$PACKAGE_VERSION" ]] && { error "[$S/$M] No package version given."; }
  [[ -z "$MAINTAINER_NAME" ]] && { error "[$S/$M] No maintainer name given."; }
  [[ -z "$MAINTAINER_MAIL" ]] && { error "[$S/$M] No maintainer mail given."; }
  [[ -z "$APT_RUN_REQUIREMENTS" ]] && { error "[$S/$M] No APT package requirements given."; }
  [[ -z "$USR_DIR" ]] && { error "[$S/$M] No path to usr directory given."; }
  [[ -z "$TARGET_DIR" ]] && { error "[$S/$M] No path to target directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S/$M] No codename given."; }
  [[ -z "$DISTRIBUTION" ]] && { error "[$S/$M] No distribution given."; }
  [[ -z "$ARCH" ]] && { error "[$S/$M] No architecture given."; }
  [[ -z "$PKG_NAME" ]] && { error "[$S/$M] No package name given."; }
  [[ -z "$TMP_BUILD_DIR" ]] && { error "[$S/$M] No path to temporary build directory given."; }
  [[ -z "$TMP_PACKAGE_DIR" ]] && { error "[$S/$M] No path to temporary package directory given."; }
  [[ -z "$PKG_CONTROL_FILE_REL" ]] && { error "[$S/$M] No path to the control file given."; }
  [[ -z "$PKG_TRIGGERS_FILE_REL" ]] && { error "[$S/$M] No path to the triggers file given."; }
  [[ -z "$PKG_SO_CONF_FILE_REL" ]] && { error "[$S/$M] No path to the *.so.conf file given."; }
  [[ -z "$PKG_BIN_DIR_REL" ]] && { error "[$S/$M] No path to the bin directory given."; }
  [[ -z "$PKG_LIB_DIR_REL" ]] && { error "[$S/$M] No path to the lib directory given."; }
  [[ -z "$PKG_RES_DIR_REL" ]] && { error "[$S/$M] No path to the resources directory given."; }

  # Compile pdftotext. Update the path to the build directory, to store the produced files (e.g.,
  # the *.o files) in the temporary build directory. Update the path to the resources directory,
  # since in the package, the resources are stored in another directory than in the development
  # environment (e.g., in "/usr/share/pdftotext-plus-plus/resources" instead of in "./resources").
  info "[$S/$M] Compiling the project ..."
  mkdir -p "$TMP_BUILD_DIR"
  make clean compile BUILD_DIR="$TMP_BUILD_DIR" RESOURCES_DIR="$PKG_RES_DIR_REL" \
     USR_DIR="$USR_DIR" VERSION="$PACKAGE_VERSION" INFO_STYLE="$MG[$S/$M/make]" > "$DEV" 2>&1

  # Create the control file.
  info "[$S/$M] Creating the control file ..."
  local PKG_CONTROL_FILE="$TMP_PACKAGE_DIR/$PKG_CONTROL_FILE_REL"
  mkdir -p $(dirname "$PKG_CONTROL_FILE")
  echo "Package: $PACKAGE_NAME" > "$PKG_CONTROL_FILE"
  echo "Depends: $APT_RUN_REQUIREMENTS" >> "$PKG_CONTROL_FILE"
  echo "Maintainer: $MAINTAINER" >> "$PKG_CONTROL_FILE"
	echo "Description: $PACKAGE_DESCRIPTION" >> "$PKG_CONTROL_FILE"
  echo "Version: $PACKAGE_VERSION" >> "$PKG_CONTROL_FILE"
	echo "Codename: $CODENAME" >> "$PKG_CONTROL_FILE"
  echo "Distribution: $DISTRIBUTION" >> "$PKG_CONTROL_FILE"
	echo "Architecture: $ARCH" >> "$PKG_CONTROL_FILE"

  # Create a *.conf file in $PKG_SO_CONF_FILE_REL, telling ldconfig (which is triggered by the entry
  # in the DEBIAN/triggers file created below) in which directory the shared libraries are stored.
  info "[$S/$M] Creating *.so.conf file ..."
  local PKG_SO_CONF_FILE="$TMP_PACKAGE_DIR/$PKG_SO_CONF_FILE_REL"
  mkdir -p "$(dirname "$PKG_SO_CONF_FILE")"
  echo "$PKG_LIB_DIR_REL" > "$PKG_SO_CONF_FILE"

  # Create the triggers file. It is responsible for invoking ldconfig after the pdftotext++ package
  # was installed on the target system, so that the shared libraries in the directory specified in
  # the *.conf file (created above) are found.
  info "[$S/$M] Creating the triggers file ..."
  local PKG_TRIGGER_FILE="$TMP_PACKAGE_DIR/$PKG_TRIGGERS_FILE_REL"
  mkdir -p $(dirname "$PKG_TRIGGER_FILE")
  echo "activate-noawait ldconfig" > "$PKG_TRIGGER_FILE"

  # Copy the main binary into the package.
  info "[$S/$M] Copying the main binary file into the package ..."
  local PKG_BIN_DIR="$TMP_PACKAGE_DIR/$PKG_BIN_DIR_REL"
  mkdir -p "$PKG_BIN_DIR"
  cp "$TMP_BUILD_DIR/pdftotext++" "$PKG_BIN_DIR"  # TODO: Parameterize the "pdftotext++" part?

  # Copy the resources into the package.
  info "[$S/$M] Copying the resources into the package ..."
  local PKG_RES_DIR="$TMP_PACKAGE_DIR/$PKG_RES_DIR_REL"
  mkdir -p "$PKG_RES_DIR"
  # The "." at the end of "resources/." copies the contents of the folder, not the folder itself.
  cp -Pa "resources/." "$PKG_RES_DIR"  # TODO: Parameterize the "resources" part?

  # Copy the shared libraries stored in '$USR_DIR/lib/' into the package.
  info "[$S/$M] Copying the shared libraries into the package ..."
  local PKG_LIB_DIR="$TMP_PACKAGE_DIR/$PKG_LIB_DIR_REL"
  mkdir -p "$PKG_LIB_DIR"
  # The "." at the end of "lib/." copies the contents of the folder, not the folder itself.
  cp -Pa "$USR_DIR/lib/." "$PKG_LIB_DIR"  # TODO: Parameterize the "lib" part?

  # Build the package.
	info "[$S/$M] Building the package ..."
  dpkg-deb --build --root-owner-group "$TMP_PACKAGE_DIR" > "$DEV" 2>&1
  [[ ! -f "$TMP_PACKAGE_DIR.deb" ]] && { error "[$S/$M] Package not built."; }

  # Copy the package to the target directory.
  info "[$S/$M] Copying package ..."
  mkdir -p "$TARGET_DIR"
  cp "$TMP_PACKAGE_DIR.deb" "$TARGET_DIR"

  # Cleaning up.
  rm -rf "$TMP_BUILD_DIR"
  rm -rf "$TMP_PACKAGE_DIR"
  info "XXX"
}

# ==================================================================================================
# PATHS.

# This function outputs the designated name of the package built for a specific distribution.
#
# Args:
#   $1 - The project name (for example: "pdftotext++").
#   $2 - The version of the project (for example: "1.0.2").
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#   $4 - The processor architecture (for example: "i386" or "amd64").
#
# Example usage:
#   define_deb_package_name "pdftotext++" "1.0.2" "bionic" "amd64"
#
function define_deb_package_name() {
  local PROJECT_NAME="$1"
  local PROJECT_VERSION="$2"
  local CODENAME="$3"
  local ARCH="$4"
  local PKG_REVISION="0"  # TODO: Consider to pass the revision also as an argument.

  [[ -z "$PROJECT_NAME" ]] && { error "No project name given."; }
  [[ -z "$PROJECT_VERSION" ]] && { error "No project version given."; }
  [[ -z "$CODENAME" ]] && { error "No codename given."; }
  [[ -z "$ARCH" ]] && { error "No architecture given."; }
  [[ -z "$PKG_REVISION" ]] && { error "No package revision given."; }

  echo "${PROJECT_NAME}_${PROJECT_VERSION}-${PKG_REVISION}${CODENAME}_${ARCH}"
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
