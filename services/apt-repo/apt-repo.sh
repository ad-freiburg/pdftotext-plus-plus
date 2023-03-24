#!/bin/bash

# TODO: Write some general description.

# The default path to the temporary directory (a directory where transient files can be stored).
DEFAULT_TMP_DIR="/tmp/pdftotext-plus-plus/apt-repo"

# Define some log levels needed to specify how verbose this script is.
# For example, to change the log level to 'debug', type: "export LOG_LEVEL=debug; ./apt-repo.sh".
ERROR=4
WARN=3
INFO=2
DEBUG=1
TRACE=0
LOG_LEVEL=${LOG_LEVEL:-"trace"}
case "$LOG_LEVEL" in
  "error")
    LOG_LEVEL=$ERROR ;;
  "warn")
    LOG_LEVEL=$WARN ;;
  "info")
    LOG_LEVEL=$INFO ;;
  "debug")
    LOG_LEVEL=$DEBUG ;;
  "trace")
    LOG_LEVEL=$TRACE ;;
  *)
    echo "WARN: Invalid log level '$LOG_LEVEL'. Using log level 'info' instead."
    LOG_LEVEL=$INFO ;;
esac

# Define some ANSI codes needed to format the log messages.
RB="\033[31;1m"  # red+bold
GB="\033[32;1m"  # green+bold
BB="\033[34;1m"  # blue+bold
MB="\033[35;1m"  # magenta+bold
R="\033[31m"     # red
G="\033[32m"     # green
B="\033[34m"     # blue
MG="\033[35m"    # magenta
N="\033[0m"      # none

# Define the device to which the output of commands like "docker", "gpg" or "deb-scanpackages"
# should be sent. If the log level is "TRACE" or lower, define the device as "/dev/stdout" (so that
# the output of those commands is printed to the console). Otherwise, set the device to "/dev/null"
# (so that the output is not printed to the console).
[[ $LOG_LEVEL -le $TRACE ]] && DEV="/dev/stdout" || DEV="/dev/null"

S=$(basename "$0")

# ==================================================================================================

# This function reads the values required by the "update_with_args" function below from environment
# variables and invokes the "update_with_args" function with passing the read values to the
# function as arguments. This enables the option to invoke the update method from within a Docker
# container, for example by running "docker run <image-name> update".
#
# Required Environmental Variables:
#   $REPO_ROOT_DIR        - The absolute path to the repository's root directory
#   $REPO_POOL_DIR_REL    - The relative path to the repository's pool dir (rel. to $REPO_ROOT_DIR)
#   $REPO_DISTS_DIR_REL   - The relative path to the repository's dists dir (rel. to $REPO_ROOT_DIR)
#   $REPO_NAME            - The name of the repository
#   $REPO_DESCRIPTION     - A description of the repository
#   $REPO_VERSION         - The version of the repository
#   $REPO_MAINTAINER_NAME - The name of the repository's maintainer
#   $REPO_MAINTAINER_MAIL - The mail of the repository's maintainer
#   $GPG_PUBLIC_KEY_FILE  - The path to the public key file
#   $GPG_PRIVATE_KEY_FILE - The path to the private key file
#
# Example usage:
#   export REPO_ROOT_DIR="/repo" REPO_POOL_DIR_REL="pool" ...; ./apt-repo.sh update
#
function update() {
  update_with_args "$REPO_ROOT_DIR" "$REPO_POOL_DIR_REL" "$REPO_DISTS_DIR_REL" "$REPO_NAME" \
      "$REPO_DESCRIPTION" "$REPO_VERSION" "$REPO_MAINTAINER_NAME" "$REPO_MAINTAINER_MAIL" \
      "$GPG_PUBLIC_KEY_FILE" "$GPG_PRIVATE_KEY_FILE"
}

# This function
#
#  (a) creates a GPG keypair (consisting of a private key and a public key) if no such keypair
#      exists yet. This keypair is used to sign the Release files created in step (b);
#  (b) reads the DEB packages stored in the repository's pool directory and creates the "Packages",
#      "Packages.gz", and "Release" files in the repository's dists dir;
#  (c) uses the private key from step (a) for signing the "Release" files and creating the
#      "Release.gpg" and "InRelease" files.
#
# Args:
#   $1  - The absolute path to the repository's root directory.
#   $2  - The relative path to the repository's pool directory (specified relative to $1).
#   $3  - The relative path to the repository's dists directory (specified relative to $1).
#   $4  - The name of the repository.
#   $5  - The description of the repository.
#   $6  - The version of the repository.
#   $7  - The name of the repository's maintainer.
#         NOTE: This name is used as value for the "Name-Real" parameter of gpg.
#   $8  - The mail of the repository's maintainer.
#         NOTE: This mail is used as value for the "Name-Email" parameter of gpg.
#   $9  - The absolute or relative path to the file containing the public GPG key.
#         NOTE: If the path is specified relative, it must be relative to this script.
#   $10 - The absolute or relative path to the file containing the private GPG key to use for
#         signing the Release files.
#         NOTE: If the path is specified relative, it must be relative to this script.
#
# Example usage:
#   update_with_args "/local/data/pdftotext++/apt-repo" "pool" "dists" "Repository of pdftotext++" \
#      "An description of the repository." "1.0" "Claudius Korzen" "korzen@cs.uni-freiburg.de" \
#      "/gpg/key.public" "/gpg/key.private"
#
function update_with_args() {
  local REPO_ROOT_DIR="$1"
  local REPO_POOL_DIR_REL="$2"
  local REPO_DISTS_DIR_REL="$3"
  local REPO_NAME="$4"
  local REPO_DESCRIPTION="$5"
  local REPO_VERSION="$6"
  local REPO_MAINTAINER_NAME="$7"
  local REPO_MAINTAINER_MAIL="$8"
  local GPG_PUBLIC_KEY_FILE="$9"
  local GPG_PRIVATE_KEY_FILE="${10}"

  info_emph "[$S] Updating the repository."
  debug " • REPO_ROOT_DIR:        '$REPO_ROOT_DIR'"
  debug " • REPO_POOL_DIR_REL:    '$REPO_POOL_DIR_REL'"
  debug " • REPO_DISTS_DIR_REL:   '$REPO_DISTS_DIR_REL'"
  debug " • REPO_NAME:            '$REPO_NAME'"
  debug " • REPO_DESCRIPTION:     '$REPO_DESCRIPTION'"
  debug " • REPO_VERSION:         '$REPO_VERSION'"
  debug " • REPO_MAINTAINER_NAME: '$REPO_MAINTAINER_NAME'"
  debug " • REPO_MAINTAINER_MAIL: '$REPO_MAINTAINER_MAIL'"
  debug " • GPG_PUBLIC_KEY_FILE:  '$GPG_PUBLIC_KEY_FILE'"
  debug " • GPG_PRIVATE_KEY_FILE: '$GPG_PRIVATE_KEY_FILE'"

  # Create a GPG keypair if it does not exist yet.
  create_gpg_keypair_if_not_exist "$GPG_PUBLIC_KEY_FILE" "$GPG_PRIVATE_KEY_FILE" \
     "$REPO_MAINTAINER_NAME" "$REPO_MAINTAINER_MAIL"

  # Create the "Packages" and "Packages.gz" files.
  create_packages_files "$REPO_ROOT_DIR" "$REPO_POOL_DIR_REL" "$REPO_DISTS_DIR_REL"

  # Create the "Release" files.
  create_release_files "$REPO_ROOT_DIR" "$REPO_POOL_DIR_REL" "$REPO_DISTS_DIR_REL" \
      "$REPO_NAME" "$REPO_VERSION" "$REPO_DESCRIPTION"

  # Sign the Release files and create the "Release.gpg" and "InRelease" files.
  sign_release_files "$REPO_ROOT_DIR" "$REPO_POOL_DIR_REL" "$REPO_DISTS_DIR_REL" \
      "$GPG_PRIVATE_KEY_FILE"
}

# ==================================================================================================
# CREATE PACKAGES FILES

# This function iterates through the *.deb files stored in the specified pool directory and
# inspects the codenames and architectures specified in their control files. For each different
# (codename, arch) pair, it creates a "Packages" file and "Packages.gz" file, by invoking
# "$(create_packages_file $1 $2 $3 <codename> <arch>)".
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's pool directory (specified relative to $1).
#   $3 - The relative path to the repository's dists directory (specified relative to $1).
#
# Example usage:
#   create_packages_files "/local/data/pdftotext++/apt-repo" "pool" "dists"
#
function create_packages_files() {
  local M="create-packages-files"
  local ROOT_DIR="$1"
  local POOL_DIR_REL="$2"
  local DISTS_DIR_REL="$3"
  local POOL_DIR="$ROOT_DIR/$POOL_DIR_REL"
  local DISTS_DIR="$ROOT_DIR/$DISTS_DIR_REL"

  info_emph "[$S/$M] Creating Packages files ..."
  debug " • ROOT_DIR:      '$ROOT_DIR'"
  debug " • POOL_DIR_REL:  '$POOL_DIR_REL'"
  debug " • DISTS_DIR_REL: '$DISTS_DIR_REL'"

  # Validate the arguments.
  [[ -z "$ROOT_DIR" ]] && { error "[$S/$M] No path to the root directory given."; }
  [[ -z "$POOL_DIR_REL" ]] && { error "[$S/$M] No path to the pool directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S/$M] No path to the dists directory given."; }
  [[ ! -d "$POOL_DIR" ]] && { error "[$S/$M] Pool directory '$POOL_DIR' does not exist."; }

  # Iterate through the deb packages stored in the pool directory and inspect the codenames and
  # architectures specified in their control files. Find all different (codename, arch) pairs.
  local PAIRS=""
  for DEB in $(find "$POOL_DIR" -name "*.deb"); do
    local CODENAME="$(dpkg -f "$DEB" Codename)"
    local ARCH="$(dpkg -f "$DEB" Architecture)"
    PAIRS="$PAIRS$CODENAME $ARCH\n"
  done
  local UNIQ_PAIRS=$(printf "$PAIRS" | sort -u)

  debug " • UNIQ_PAIRS:"
  while IFS=$' ' read -r CODENAME ARCH; do
    debug "   - CODENAME: '$CODENAME'; ARCH: '$ARCH'"
  done < <(printf "$UNIQ_PAIRS\n")
  [[ -z "$UNIQ_PAIRS" ]] && { error "[$S/$M] No (codename, arch) pairs found."; }

  # Create a "Packages" and "Packages.gz" file for each unique (codename, arch) pair.
  while IFS=$' ' read -r CODENAME ARCH; do
    create_packages_file "$ROOT_DIR" "$POOL_DIR_REL" "$DISTS_DIR_REL" "$CODENAME" "$ARCH"
  done < <(printf "$UNIQ_PAIRS\n" )
}

# This function creates a "Packages" file and "Packages.gz" file for a specific distribution, in
# the format as described in step 2 of [1]. The "Packages" file is stored in
# "$(get_packages_file_path $1 $3 $4 $5)", and the "Packages.gz" file in
# "$(get_packages_gz_file_path $1 $3 $4 $5)".
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's pool directory (specified relative to $1).
#   $3 - The relative path to the repository's dists directory (specified relative to $1).
#   $4 - The codename of the distribution (for example: "bionic" or "jammy").
#   $5 - The processor architecture (for example: "i386" or "amd64").
#
# Example usage:
#   create_packages_file "/local/data/pdftotext++/apt-repo" "pool" "dists" "bionic" "amd64"
#
function create_packages_file() {
  local M="create-packages-files"
  local ROOT_DIR="$1"
  local POOL_DIR_REL="$2"
  local DISTS_DIR_REL="$3"
  local CODENAME="$4"
  local ARCH="$5"
  local C="$CODENAME"
  local A="$ARCH"
  local POOL_DIR="$ROOT_DIR/$POOL_DIR_REL"
  local DISTS_DIR="$ROOT_DIR/$DISTS_DIR_REL"
  local PKG_FILE="$(get_packages_file_path "$1" "$3" "$4" "$5")";
  local GZ_FILE="$(get_packages_gz_file_path "$1" "$3" "$4" "$5")";

  info_emph "[$S/$M][$C][$A] Creating Packages file ..."
  debug " • ROOT_DIR:      '$ROOT_DIR'"
  debug " • POOL_DIR_REL:  '$POOL_DIR_REL'"
  debug " • DISTS_DIR_REL: '$DISTS_DIR_REL'"
  debug " • CODENAME:      '$CODENAME'"
  debug " • ARCH:          '$ARCH'"
  debug " • PKG_FILE:      '$PKG_FILE'"
  debug " • GZ_FILE:       '$GZ_FILE'"

  # Validate the arguments.
  [[ -z "$ROOT_DIR" ]] && { error "[$S/$M][$C][$A] No path to the root directory given."; }
  [[ -z "$POOL_DIR_REL" ]] && { error "[$S/$M][$C][$A] No path to the pool directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S/$M][$C][$A] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S/$M][$C][$A] No codename given."; }
  [[ -z "$ARCH" ]] && { error "[$S/$M][$C][$A] No arch given."; }
  [[ -z "$PKG_FILE" ]] && { error "[$S/$M][$C][$A] No path to the Packages file given."; }
  [[ -z "$GZ_FILE" ]] && { error "[$S/$M][$C][$A] No path to the Packages.gz file given."; }
  [[ ! -d "$POOL_DIR" ]] && { error "[$S/$M][$C][$A] Pool directory '$POOL_DIR' does not exist."; }

  # Iterate through all DEB packages which are stored in the specified pool directory and contain
  # the specified codename in their filename. For each such package, add an entry to the Packages
  # file, in the format as described in step 2 of [1].
  mkdir -p $(dirname "$PKG_FILE")
  > "$PKG_FILE"  # Make sure that the Packages file is initially empty.
  for DEB in $(find "$POOL_DIR" -name "*$CODENAME*.deb" | sort -V); do
    echo "Package: $(dpkg -f "$DEB" Package)" >> "$PKG_FILE"
    echo "Depends: $(dpkg -f "$DEB" Depends)" >> "$PKG_FILE"
    echo "Description: $(dpkg -f "$DEB" Description)" >> "$PKG_FILE"
    echo "Version: $(dpkg -f "$DEB" Version)" >> "$PKG_FILE"
    echo "Architecture: $(dpkg -f "$DEB" Architecture)" >> "$PKG_FILE"
    echo "Maintainer: $(dpkg -f "$DEB" Maintainer)" >> "$PKG_FILE"
    # Add the relative path to the DEB package (relative to the repository's root directory).
    echo "Filename: $(realpath --relative-to "$ROOT_DIR" "$DEB")" >> "$PKG_FILE"
    echo "Size: $(stat -c %s "$DEB")" >> "$PKG_FILE"
    echo "MD5Sum: $(md5sum $DEB | cut -d" " -f1)" >> "$PKG_FILE"
    echo "SHA1: $(sha1sum $DEB | cut -d" " -f1)" >> "$PKG_FILE"
    echo "SHA256: $(sha256sum $DEB | cut -d" " -f1)" >> "$PKG_FILE"
    echo "" >> "$PKG_FILE"
  done

  # Create the Packages.gz file.
  cat "$PKG_FILE" | gzip -f9 > "$GZ_FILE"
}

# ==================================================================================================
# CREATE RELEASE FILES.

# This function iterates through the *.deb files stored in the specified pool directory and
# inspects the codenames specified in their control files. For each different codename, it creates
# a "Release" file, in the format as described in step 2 of [1], by calling "$(create_release_file
# $1 $2 $3 $4 <codename> <codename> $5 'stable' $6)".
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's pool directory (specified relative to $1).
#   $3 - The relative path to the repository's dists directory (specified relative to $1).
#   $4 - The value which should be associated with the "Origin" key in a Release file.
#   $5 - The value which should be associated with the "Version" key in a Release file.
#   $6 - The value which should be associated with the "Description" key in a Release file.
#
# Example usage:
#   create_release_files "/local/data/pdftotext++/apt-repo" "pool" "dists" "pdftotext++" \
#      "1.0" "A fancy description of the repository."
#
function create_release_files() {
  local M="create-release-files"
  local ROOT_DIR="$1"
  local POOL_DIR_REL="$2"
  local DISTS_DIR_REL="$3"
  local ORIGIN="$4"
  local VERSION="$5"
  local DESCRIPTION="$6"
  local POOL_DIR="$ROOT_DIR/$POOL_DIR_REL"
  local DISTS_DIR="$ROOT_DIR/$DISTS_DIR_REL"

  info_emph "[$S/$M] Creating Release files ..."
  debug " • ROOT_DIR:      '$ROOT_DIR'"
  debug " • POOL_DIR_REL:  '$POOL_DIR_REL'"
  debug " • DISTS_DIR_REL: '$DISTS_DIR_REL'"
  debug " • ORIGIN:        '$ORIGIN'"
  debug " • VERSION:       '$VERSION'"
  debug " • DESCRIPTION:   '$DESCRIPTION'"

  # Validate the arguments.
  [[ -z "$ROOT_DIR" ]] && { error "[$S/$M] No path to the root directory given."; }
  [[ -z "$POOL_DIR_REL" ]] && { error "[$S/$M] No path to the pool directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S/$M] No path to the dists directory given."; }
  [[ -z "$ORIGIN" ]] && { error "[$S/$M] No origin given."; }
  [[ -z "$VERSION" ]] && { error "[$S/$M] No version given."; }
  [[ -z "$DESCRIPTION" ]] && { error "[$S/$M] No description given."; }
  [[ ! -d "$POOL_DIR" ]] && { error "[$S/$M] Pool directory '$POOL_DIR' does not exist."; }

  # Iterate through the DEB packages stored in the specified pool directory and inspect the
  # codenames specified in their control files. Find all different codenames.
  local CODENAMES=""
  for DEB in $(find "$POOL_DIR" -name "*.deb"); do
    local CODENAME="$(dpkg -f "$DEB" Codename)"
    CODENAMES="$CODENAMES$CODENAME\n"
  done
  local UNIQ_CODENAMES=$(printf "$CODENAMES" | sort -u)

  UNIQ_CODENAMES_STR=""
  while read -r NAME; do
    UNIQ_CODENAMES_STR="$UNIQ_CODENAMES_STR'$NAME', "
  done < <(printf "$UNIQ_CODENAMES\n" )
  debug " • UNIQ_CODENAMES: ${UNIQ_CODENAMES_STR::-2}"
  [[ -z "$UNIQ_CODENAMES" ]] && { error "[$S/$M] No codenames found."; }

  # Create a Release file for each unique codename.
  while read -r CODENAME; do
    create_release_file "$ROOT_DIR" "$POOL_DIR_REL" "$DISTS_DIR_REL" "$ORIGIN" "$CODENAME" \
       "$CODENAME" "$VERSION" "stable" "$DESCRIPTION"
  done < <(printf "$UNIQ_CODENAMES\n" )
}

# This function creates a "Release" file for a specific distribution (i.e., the distribution
# relating to the codename specified by $6), in the format as described in step 2 of [1]. The file
# is stored in "$(get_release_file_path $1 $3 $6)".
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's pool directory (specified relative to $1).
#   $3 - The relative path to the repository's dists directory (specified relative to $1).
#   $4 - The value which should be associated with the "Origin" key in the Release file.
#   $5 - The value which should be associated with the "Suite" key in the Release file.
#   $6 - The value which should be associated with the "Codename" key in the Release file.
#   $7 - The value which should be associated with the "Version" key in the Release file.
#   $8 - The value which should be associated with the "Components" key in the Release file.
#   $9 - The value which should be associated with the "Description" key in the Release file.
#
# Example usage:
#   create_release_file "/local/data/pdftotext++/apt-repo" "pool" "dists" "pdftotext++" \
#      "bionic" "bionic" "1.0" "stable" "A fancy description of the repository."
#
function create_release_file() {
  local M="create-release-files"
  local ROOT_DIR="$1"
  local POOL_DIR_REL="$2"
  local DISTS_DIR_REL="$3"
  local ORIGIN="$4"
  local SUITE="$5"
  local CODENAME="$6"
  local VERSION="$7"
  local COMPONENTS="$8"
  local DESCRIPTION="$9"
  local C="$CODENAME"
  local POOL_DIR="$ROOT_DIR/$POOL_DIR_REL"
  local DISTS_DIR="$ROOT_DIR/$DISTS_DIR_REL"
  local DATE="$(date -Ru)"
  local RELEASE_FILE="$(get_release_file_path "$1" "$3" "$6")";

  info_emph "[$S/$M][$C] Creating Release file ..."
  debug " • ROOT_DIR:      '$ROOT_DIR'"
  debug " • POOL_DIR_REL:  '$POOL_DIR_REL'"
  debug " • DISTS_DIR_REL: '$DISTS_DIR_REL'"
  debug " • ORIGIN:        '$ORIGIN'"
  debug " • SUITE:         '$SUITE'"
  debug " • CODENAME:      '$CODENAME'"
  debug " • VERSION:       '$VERSION'"
  debug " • COMPONENTS:    '$COMPONENTS'"
  debug " • DESCRIPTION:   '$DESCRIPTION'"
  debug " • DATE:          '$DATE'"
  debug " • RELEASE_FILE:  '$RELEASE_FILE'"

  # Validate the arguments.
  [[ -z "$ROOT_DIR" ]] && { error "[$S/$M][$C] No path to the root directory given."; }
  [[ -z "$POOL_DIR_REL" ]] && { error "[$S/$M][$C] No path to the pool directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S/$M][$C] No path to the dists directory given."; }
  [[ -z "$ORIGIN" ]] && { error "[$S/$M][$C] No origin given."; }
  [[ -z "$SUITE" ]] && { error "[$S/$M][$C] No suite given."; }
  [[ -z "$CODENAME" ]] && { error "[$S/$M][$C] No codename given."; }
  [[ -z "$VERSION" ]] && { error "[$S/$M][$C] No version given."; }
  [[ -z "$COMPONENTS" ]] && { error "[$S/$M][$C] No components given."; }
  [[ -z "$DESCRIPTION" ]] && { error "[$S/$M][$C] No description given."; }
  [[ -z "$DATE" ]] && { error "[$S/$M][$C] No date given."; }
  [[ -z "$RELEASE_FILE" ]] && { error "[$S/$M][$C] No path to the Release file given."; }
  [[ ! -d "$POOL_DIR" ]] && { error "[$S/$M][$C] Pool directory '$POOL_DIR' does not exist."; }

  # Iterate through the *.deb files belonging to the given codename and inspect the architectures
  # specified in their control files. Find all different architectures.
  local ARCHS=""
  for DEB in $(find "$POOL_DIR" -name "*.deb"); do
    local ARCH="$(dpkg -f "$DEB" Architecture)"
    ARCHS="$ARCHS$ARCH\n"
  done
  local UNIQ_ARCHS=$(printf "$ARCHS" | sort -u)

  UNIQ_ARCHS_STR=""
  while read -r ARCH; do
    UNIQ_ARCHS_STR="$UNIQ_ARCHS_STR'$ARCH', "
  done < <(printf "$UNIQ_ARCHS\n")
  debug " • UNIQ_ARCHS:    ${UNIQ_ARCHS_STR::-2}"
  [[ -z "$UNIQ_ARCHS" ]] && { error "[$S/$M][$C] No architectures found."; }

  # Create the Release file.
  local RELEASE_FILE_PARENT_DIR=$(dirname "$RELEASE_FILE")
  mkdir -p "$RELEASE_FILE_PARENT_DIR"
  echo "Origin: $ORIGIN" > "$RELEASE_FILE"
  echo "Suite: $SUITE" >> "$RELEASE_FILE"
  echo "Codename: $CODENAME" >> "$RELEASE_FILE"
  echo "Version: $VERSION" >> "$RELEASE_FILE"
  echo "Architectures: $UNIQ_ARCHS" >> "$RELEASE_FILE"
  echo "Components: $COMPONENTS" >> "$RELEASE_FILE"
  echo "Description: $DESCRIPTION" >> "$RELEASE_FILE"
  echo "Date: $DATE" >> "$RELEASE_FILE"

  # Iterate through the files in the Release file's parent directory (ignore the Release file). For
  # each, add the MD5 message digest, SHA1 message digest and the SHA256 message digest to the
  # Release file.
  FILES=$(find "$RELEASE_FILE_PARENT_DIR" -type f ! -name "*Release*")
  [[ -z "$FILES" ]] && { error "[$S/$M][$C] No files found."; }

  echo "MD5Sum:" >> "$RELEASE_FILE"
  for F in $FILES; do
    local SUM="$(md5sum "$F" | cut -d" " -f1)"
    local BYTES="$(wc -c "$F" | cut -d" " -f1)"
    local PATH_REL="$(realpath --relative-to "$RELEASE_FILE_PARENT_DIR" "$F")"
    echo " $SUM $BYTES $PATH_REL" >> "$RELEASE_FILE"
  done

  echo "SHA1:" >> "$RELEASE_FILE"
  for F in $FILES; do
    local SUM="$(sha1sum "$F" | cut -d" " -f1)"
    local BYTES="$(wc -c "$F" | cut -d" " -f1)"
    local PATH_REL="$(realpath --relative-to "$RELEASE_FILE_PARENT_DIR" "$F")"
    echo " $SUM $BYTES $PATH_REL" >> "$RELEASE_FILE"
  done

  echo "SHA256:" >> "$RELEASE_FILE"
  for F in $FILES; do
    local SUM="$(sha256sum "$F" | cut -d" " -f1)"
    local BYTES="$(wc -c "$F" | cut -d" " -f1)"
    local PATH_REL="$(realpath --relative-to "$RELEASE_FILE_PARENT_DIR" "$F")"
    echo " $SUM $BYTES $PATH_REL" >> "$RELEASE_FILE"
  done
}

# ==================================================================================================
# ENCRYPTION.

# This function creates a non-expiring and non-password-protected 4096 bit RSA keypair. The public
# key is exported to "$1" and the private key to "$2". If these files already exist, this function
# does not create a keypair.
#
# Args:
#   $1 - The absolute or relative path to the file to which the public key should be exported.
#        NOTE: If the path is specified relative, it must be relative to this script.
#   $2 - The absolute or relative path to the file to which the private key should be exported.
#        NOTE: If the path is specified relative, it must be relative to this script.
#   $3 - The value to associate with the "Name-Real" parameter of gpg.
#   $4 - The value to associate with the "Name-Email" parameter of gpg.
#
# Example usage:
#  create_gpg_keypair_if_not_exist "./key.public" "./key.private" "Claudius Korzen" \
#     "korzen@cs.uni-freiburg.de"
#
function create_gpg_keypair_if_not_exist() {
  local M="create-gpg-keypair"
  local PUBLIC_KEY_FILE="$1"
  local PRIVATE_KEY_FILE="$2"
  local NAME_REAL="$3"
  local NAME_EMAIL="$4"
  local TMP_KEYRING_DIR="$DEFAULT_TMP_DIR/gpg-$(date +%s)"

  info_emph "[$S/$M] Creating GPG keypair ..."
  debug " • PUBLIC_KEY_FILE:  '$PUBLIC_KEY_FILE'"
  debug " • PRIVATE_KEY_FILE: '$PRIVATE_KEY_FILE'"
  debug " • NAME_REAL:        '$NAME_REAL'"
  debug " • NAME_EMAIL:       '$NAME_EMAIL'"
  debug " • TMP_KEYRING_DIR:  '$TMP_KEYRING_DIR'"

  # Validate the arguments.
  [[ -z "$PUBLIC_KEY_FILE" ]] && { error "[$S/$M] No path to the public key file given."; }
  [[ -z "$PRIVATE_KEY_FILE" ]] && { error "[$S/$M] No path to the private key file given."; }
  [[ -z "$NAME_REAL" ]] && { error "[$S/$M] No value for the 'Name-Real' parameter given."; }
  [[ -z "$NAME_EMAIL" ]] && { error "[$S/$M] No value for the 'Name-Email' parameter given."; }

  # Abort if both key files already exist.
  if [[ -f "$PUBLIC_KEY_FILE" && -f "$PRIVATE_KEY_FILE" ]]; then
    info "[$S/$M] GPG keypair already exists. Abort."
    return;
  fi

  # Create the temporary keyring directory.
  mkdir -p -m 0700 "$TMP_KEYRING_DIR"
  export GNUPGHOME="$TMP_KEYRING_DIR"

  # Create a batch script, for creating the keypair using the --batch feature of gpg, rather than
  # using the interactive prompt. TODO: Are the chosen arguments in the script reasonable?
  local TMP_SCRIPT="$TMP_KEYRING_DIR/create_gpg_keypair_script.txt"
  echo "Key-Type: RSA" > "$TMP_SCRIPT"
  echo "Key-Length: 4096" >> "$TMP_SCRIPT"
  echo "Name-Real: $NAME_REAL" >> "$TMP_SCRIPT"
  echo "Name-Email: $NAME_EMAIL" >> "$TMP_SCRIPT"
  echo "Expire-Date: 0" >> "$TMP_SCRIPT"
  echo "%no-ask-passphrase" >> "$TMP_SCRIPT"
  echo "%no-protection" >> "$TMP_SCRIPT"
  echo "%commit" >> "$TMP_SCRIPT"

  info "[$S/$M] Creating keypair ..."
  gpg --no-tty --gen-key --batch "$TMP_SCRIPT" > "$DEV" 2>&1

  info "[$S/$M] Exporting the public key ..."
  mkdir -p "$(dirname "$PUBLIC_KEY_FILE")"
  gpg --armor --export > "$PUBLIC_KEY_FILE"

  info "[$S/$M] Exporting the private key ..."
  mkdir -p "$(dirname "$PRIVATE_KEY_FILE")"
  gpg --armor --export-secret-keys > "$PRIVATE_KEY_FILE"

  # Cleanup.
  rm -rf "$TMP_KEYRING_DIR"
}

# This function iterates through the *.deb files stored in the specified pool directory and inspects
# the codenames specified in their control files. For each different codename, it uses the specified
# private key for (a) signing the corresponding "Release" file and storing the result in
# "Release.gpg" (stored next to the "Release" file) and (b) creating an "InRelease" file
# (containing the content of the "Release" file and the "Release.gpg" file in one file). More
# information about these files can be found in step 3 of [1].
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's pool directory (specified relative to $1).
#   $3 - The relative path to the repository's dists directory (specified relative to $1).
#   $4 - The absolute path to the file containing the private key to use for signing the Release
#        files.
#
# Example usage:
#   sign_release_files "/local/data/pdftotext++/apt-repo" "pool" "dists" "./keys/key.private"
#
function sign_release_files() {
  local M="sign-release-files"
  local ROOT_DIR="$1"
  local POOL_DIR_REL="$2"
  local DISTS_DIR_REL="$3"
  local PRIV_KEY_FILE="$4"
  local POOL_DIR="$ROOT_DIR/$POOL_DIR_REL"
  local TMP_KEYRING_DIR="$DEFAULT_TMP_DIR/gpg-$(date +%s)"

  info_emph "[$S/$M] Signing the Release files ..."
  debug " • ROOT_DIR:      '$ROOT_DIR'"
  debug " • POOL_DIR_REL:  '$POOL_DIR_REL'"
  debug " • DISTS_DIR_REL: '$DISTS_DIR_REL'"
  debug " • PRIV_KEY_FILE: '$PRIV_KEY_FILE'"

  # Validate the arguments.
  [[ -z "$ROOT_DIR" ]] && { error "[$S/$M] No path to the root directory given."; }
  [[ -z "$POOL_DIR_REL" ]] && { error "[$S/$M] No path to the pool directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S/$M] No path to the dists directory given."; }
  [[ -z "$PRIV_KEY_FILE" ]] && { error "[$S/$M] No path to the private key file given."; }
  [[ ! -d "$POOL_DIR" ]] && { error "[$S/$M] Pool directory '$POOL_DIR' does not exist."; }
  [[ ! -f "$PRIV_KEY_FILE" ]] && { error "[$S/$M][$C] Private key '$PRIV_KEY_FILE' doesn't exist."; }

  # Iterate through the DEB packages stored in the specified pool directory and inspect the
  # codenames specified in their control files. Find all different codenames.
  local CODENAMES=""
  for DEB in $(find "$POOL_DIR" -name "*.deb"); do
    local CODENAME="$(dpkg -f "$DEB" Codename)"
    CODENAMES="$CODENAMES$CODENAME\n"
  done
  local UNIQ_CODENAMES=$(printf "$CODENAMES" | sort -u)

  UNIQ_CODENAMES_STR=""
  while read -r NAME; do
    UNIQ_CODENAMES_STR="$UNIQ_CODENAMES_STR'$NAME', "
  done < <(printf "$UNIQ_CODENAMES\n")
  debug " • UNIQ_CODENAMES:   ${UNIQ_CODENAMES_STR::-2}"
  [[ -z "$UNIQ_CODENAMES" ]] && { error "[$S/$M] No codenames found."; }

  # Create the temporary keyring dir.
  mkdir -p -m 0700 "$TMP_KEYRING_DIR"
  export GNUPGHOME="$TMP_KEYRING_DIR"

  info "[$S/$M] Importing private key ..."
  cat "$PRIV_KEY_FILE" | gpg --import > "$DEV" 2>&1

  # Sign the release file for each unique codename.
  while read -r CODENAME; do
    sign_release_file "$ROOT_DIR" "$DISTS_DIR_REL" "$CODENAME"
  done < <(printf "$UNIQ_CODENAMES\n")

  # Cleanup.
  rm -rf "$TMP_KEYRING_DIR"
}

# This function signs the "Release" file that is associated with the given codename. The result
# is stored in a "Release.gpg" file, which is stored next to the "Release" file. It further creates
# a "InRelease" file, containing the content of the "Release" file and the "Release.gpg" in a
# single file (this file is also stored next to the "Release" file).
#
# Args:
#   $1 - The absolute path to repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#
# Example usage:
#   sign_release_file "/local/data/pdftotext++/apt-repo" "dists" "bionic"
#
function sign_release_file() {
  local M="sign-release-files"
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CODENAME="$3"
  local C="$CODENAME"
  local DISTS_DIR="$ROOT_DIR/$DISTS_DIR_REL"
  local RELEASE_FILE="$(get_release_file_path "$ROOT_DIR" "$DISTS_DIR_REL" "$CODENAME")"
  local RELEASE_GPG_FILE="$(get_release_gpg_file_path "$ROOT_DIR" "$DISTS_DIR_REL" "$CODENAME")"
  local INRELEASE_FILE="$(get_inrelease_file_path "$ROOT_DIR" "$DISTS_DIR_REL" "$CODENAME")"

  info_emph "[$S/$M][$C] Signing Release file ..."
  debug " • CODENAME:         '$CODENAME'"
  debug " • ROOT_DIR:         '$ROOT_DIR'"
  debug " • DISTS_DIR_REL:    '$DISTS_DIR_REL'"
  debug " • RELEASE_FILE:     '$RELEASE_FILE'"
  debug " • RELEASE_GPG_FILE: '$RELEASE_GPG_FILE'"
  debug " • INRELEASE_FILE:   '$INRELEASE_FILE'"

  # Validate the arguments.
  [[ -z "$CODENAME" ]] && { error "[$S/$M][$C] No codename given."; }
  [[ -z "$ROOT_DIR" ]] && { error "[$S/$M][$C] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S/$M][$C] No path to the dists directory given."; }
  [[ -z "$RELEASE_FILE" ]] && { error "[$S/$M][$C] No path to the Release file given."; }
  [[ -z "$RELEASE_GPG_FILE" ]] && { error "[$S/$M][$C] No path to the Release.gpg file given."; }
  [[ -z "$INRELEASE_FILE" ]] && { error "[$S/$M][$C] No path to the InRelease file given."; }
  [[ ! -f "$RELEASE_FILE" ]] && { error "[$S/$M][$C] Release file '$RELEASE_FILE' doesn't exist."; }

  info "[$S/$M][$C] Signing Release file ..."
  mkdir -p "$(dirname "$RELEASE_GPG_FILE")"
  cat "$RELEASE_FILE" | gpg -abs > "$RELEASE_GPG_FILE"

  info "[$S/$M][$C] Creating InRelease file ..."
  mkdir -p "$(dirname "$INRELEASE_FILE")"
  cat "$RELEASE_FILE" | gpg -abs --clearsign > "$INRELEASE_FILE"
}

# ==================================================================================================

# This function reads the values required by the "start_server_with_args" function below from
# environment variables and invokes the "start_server_with_args" function, with passing the read
# values to the function as arguments. This enables the option to invoke the start_server method
# from within a Docker container, for example by running "docker run <image-name> start_server".
#
# Required Environmental Variables:
#   $REPO_ROOT_DIR        - The absolute path to the repository's root directory
#   $REPO_POOL_DIR_REL    - The relative path to the repository's pool dir (rel. to $REPO_ROOT_DIR)
#   $REPO_DISTS_DIR_REL   - The relative path to the repository's dists dir (rel. to $REPO_ROOT_DIR)
#   $REPO_NAME            - The name of the repository
#   $REPO_DESCRIPTION     - A description of the repository
#   $REPO_VERSION         - The version of the repository
#   $REPO_MAINTAINER_NAME - The name of the repository's maintainer
#   $REPO_MAINTAINER_MAIL - The mail of the repository's maintainer
#   $GPG_PUBLIC_KEY_FILE  - The path to the public key file
#   $GPG_PRIVATE_KEY_FILE - The path to the private key file
#
# Example usage:
#   export REPO_ROOT_DIR="/repo" REPO_POOL_DIR_REL="pool" ...; ./apt-repo.sh update
#
function start_server() {
  start_server_with_args "$REPO_ROOT_DIR" "$REPO_POOL_DIR_REL" "$REPO_DISTS_DIR_REL" \
      "$GPG_PUBLIC_KEY_FILE" "$NGINX_CUSTOM_CONFIG_FILE"
}

# This function starts a web server for serving the files in the repository's pool directory and
# the repository's dists directory.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's pool directory (specified relative to $1).
#   $3 - The relative path to the repository's dists directory (specified relative to $1).
#   $4 - The path to the file containing the public key that can be used to verify the signatures
#        of the Release files.
#        NOTE: This key file becomes downloadable from "https://<apt-repo-url>/gpg"
#   $5 - (optional) The absolute or relative path to a custom nginx config file.
#        NOTE: If not specified, the default "/etc/nginx/nginx.conf" file is used instead.
#        If the path is specified relative, it must be relative to this script.
#
# Example usages:
#   start_server_with_args "/repo" "pool" "dists" "./gpg/key.public"
#   start_server_with_args "/repo" "pool" "dists" "./gpg/key.public" "apt-repo.nginx.conf"
#
function start_server_with_args() {
  local M="start-server"
  local ROOT_DIR="$1"
  local POOL_DIR_REL="$2"
  local DISTS_DIR_REL="$3"
  local PUBLIC_KEY_FILE="$4"
  local CONFIG_FILE="$5"
  local POOL_DIR="$ROOT_DIR/$POOL_DIR_REL"
  local DISTS_DIR="$ROOT_DIR/$DISTS_DIR_REL"
  local NGINX_HTML_DIR="/usr/share/nginx/html"
  local NGINX_CONFIG_FILE="/etc/nginx/nginx.conf"

  info_emph "[$S/$M] Starting the web server ..."
  debug " • ROOT_DIR:          '$ROOT_DIR'"
  debug " • POOL_DIR_REL:      '$POOL_DIR_REL'"
  debug " • DISTS_DIR_REL:     '$DISTS_DIR_REL'"
  debug " • PUBLIC_KEY_FILE:   '$PUBLIC_KEY_FILE'"
  debug " • CONFIG_FILE:       '$CONFIG_FILE'"
  debug " • NGINX_HTML_DIR:    '$NGINX_HTML_DIR'"
  debug " • NGINX_CONFIG_FILE: '$NGINX_CONFIG_FILE'"

  # Validate the arguments.
  [[ -z "$ROOT_DIR" ]] && { error "[$S/$M] No path to the root directory given."; }
  [[ -z "$POOL_DIR_REL" ]] && { error "[$S/$M] No path to the pool directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S/$M] No path to the dists directory given."; }
  [[ -z "$PUBLIC_KEY_FILE" ]] && { error "[$S/$M] No path to the public key file given."; }
  [[ ! -d "$POOL_DIR" ]] && { error "[$S/$M] Pool directory '$POOL_DIR' does not exist."; }
  [[ ! -d "$DISTS_DIR" ]] && { error "[$S/$M] Dists directory '$DISTS_DIR' does not exist."; }
  [[ ! -f "$PUBLIC_KEY_FILE" ]] && { error "[$S/$M] Public key '$PUBLIC_KEY_FILE' doesn't exist."; }
  [[ ! -d "$NGINX_HTML_DIR" ]] && { error "[$S/$M] HTML dir '$NGINX_HTML_DIR' does not exist."; }

  # Make the files in the repository's pool directory and the repository's dists directory, by
  # adding a symbolic link in the $NGINX_HTML_DIR.
  # NOTE: To let nginx follow symbolic links, add "disable_symlinks off;" to the nginx.conf.
  rm -rf "$NGINX_HTML_DIR/index.html"  # clear the default index.html.
  ln -s "$POOL_DIR" "$NGINX_HTML_DIR"
  ln -s "$DISTS_DIR" "$NGINX_HTML_DIR"
  ln -s "$PUBLIC_KEY_FILE" "$NGINX_HTML_DIR/gpg"

  # If a config file is specified, replace "/etc/nginx/nginx.conf" by this config file.
  if [[ ! -z "$CONFIG_FILE" ]]; then
    # Ensure that the config file exists.
    [[ ! -f "$CONFIG_FILE" ]] && { error "[$S/$M] Config file '$CONFIG_FILE' does not exist."; }

    info "[$S/$M] Copying config file to '$NGINX_CONFIG_FILE' ..."
    cp "$CONFIG_FILE" "$NGINX_CONFIG_FILE"
  fi

  nginx -g "daemon off;"
}

# ==================================================================================================
# LOGGING.

# This function prints the specified debug message to the console, if the current log level is
# equal to "debug" or lower.
#
# Args:
#   $1 - The debug message to print.
#
# Example usage:
#   debug "Creating package ..."
#
function debug() {
  if [[ $LOG_LEVEL -le $DEBUG ]]; then
    echo "$1"
  fi
}

# This function prints the specified info message to the console (in bold), if the current log
# level is equal to "info" or lower.
#
# Args:
#   $1 - The info message to print.
#
# Example usage:
#   info_emph "Creating package ..."
#
function info_emph() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$MB$1$N"
  fi
}

# This function prints the specified info message to the console, if the current log level is
# equal to "info" or lower.
#
# Args:
#   $1 - The info message to print.
#
# Example usage:
#   info "Creating package ..."
#
function info() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$MG$1$N"
  fi
}

# This function prints the specified success message to the console (in bold), if the current log
# level is equal to "info" or lower.
#
# Args:
#   $1 - The success message to print.
#
# Example usage:
#   success_emph "Package created successfully."
#
function success_emph() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$GB$1$N"
  fi
}

# This function prints the specified success message to the console, if the current log level is
# equal to "info" or lower.
#
# Args:
#   $1 - The success message to print.
#
# Example usage:
#   success "Package created successfully."
#
function success() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$G$1$N"
  fi
}

# This function (a) prints the specified error message to the console, together with the function
# name and the line number specifying where the error occurred, if the current log level is
# equal to "error" or lower, and (b) terminates the execution of the script with status code 1.
#
# Args:
#   $1 - The error message to print.
#
# Example usage:
#   error "Parameter is missing."
#
function error() {
  if [[ $LOG_LEVEL -le $ERROR ]]; then
    echo -e "$RB$1 (in ${FUNCNAME[1]}:${BASH_LINENO[0]})$N"
  fi
  exit 1
}

# ==================================================================================================
# PATHS.

# This function outputs the *relative* path to the directory where the "Packages" file of a
# specific distribution should be stored. The path is specified relative to the repository's root
# directory.
#
# Args:
#   $1 - The relative path to the repository's dists directory (specified relative to the
#        repository's root directory).
#   $2 - The codename of the distribution (for example: "bionic" or "jammy").
#   $3 - The processor architecture (for example: "i386" or "amd64").
#
# Example usage:
#   get_packages_file_parent_dir_rel "dists" "bionic" "amd64"
#
function get_packages_file_parent_dir_rel() {
  local DISTS_DIR_REL="$1"
  local CODENAME="$2"
  local ARCH="$3"

  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S] No codename given."; }
  [[ -z "$ARCH" ]] && { error "[$S] No architecture given."; }

  echo "$DISTS_DIR_REL/$CODENAME/stable/binary-$ARCH"
}

# This function outputs the *absolute* path to the directory where the "Packages" file of a
# specific distribution should be stored.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#   $4 - The processor architecture (for example: "i386" or "amd64").
#
# Example usage:
#   get_packages_file_parent_dir "/local/data/pdftotext++/apt-repo" "dists" "bionic" "amd64"
#
function get_packages_file_parent_dir() {
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CODENAME="$3"
  local ARCH="$4"

  [[ -z "$ROOT_DIR" ]] && { error "[$S] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S] No codename given."; }
  [[ -z "$ARCH" ]] && { error "[$S] No architecture given."; }

  echo "$ROOT_DIR/$(get_packages_file_parent_dir_rel "$DISTS_DIR_REL" "$CODENAME" "$ARCH")"
}

# This function outputs the *absolute* path to the "Packages" file of a specific distribution.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#   $4 - The processor architecture (for example: "i386" or "amd64").
#
# Example usage:
#   get_packages_file_path "/local/data/pdftotext++/apt-repo" "dists" "bionic" "amd64"
#
function get_packages_file_path() {
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CN="$3"
  local ARCH="$4"

  [[ -z "$ROOT_DIR" ]] && { error "[$S] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CN" ]] && { error "[$S] No codename given."; }
  [[ -z "$ARCH" ]] && { error "[$S] No architecture given."; }

  echo "$(get_packages_file_parent_dir "$ROOT_DIR" "$DISTS_DIR_REL" "$CN" "$ARCH")/Packages"
}

# This function outputs the *absolute* path to the "Packages.gz" file of a specific distribution.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#   $4 - The processor architecture (for example: "i386" or "amd64").
#
# Example usage:
#   get_packages_gz_file_path "/local/data/pdftotext++/apt-repo" "dists" "bionic" "amd64"
#
function get_packages_gz_file_path() {
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CN="$3"
  local ARCH="$4"

  [[ -z "$ROOT_DIR" ]] && { error "[$S] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CN" ]] && { error "[$S] No codename given."; }
  [[ -z "$ARCH" ]] && { error "[$S] No architecture given."; }

  echo "$(get_packages_file_parent_dir "$ROOT_DIR" "$DISTS_DIR_REL" "$CN" "$ARCH")/Packages.gz"
}

# This function outputs the *relative* path to the directory where the "Release" file of a specific
# distribution should be stored. The path is specified relative to the repository's root directory.
#
# Args:
#   $1 - The relative path to the repository's dists directory (specified relative to the
#        repository's root dir).
#   $2 - The codename of the distribution (for example: "bionic" or "jammy").
#
# Example usage:
#   get_release_file_parent_dir_rel "dists" "bionic"
#
function get_release_file_parent_dir_rel() {
  local DISTS_DIR_REL="$1"
  local CODENAME="$2"

  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S] No codename given."; }

  echo "$DISTS_DIR_REL/$CODENAME"
}

# This function outputs the *absolute* path to the directory where the "Release" file of a
# specific distribution should be stored.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#
# Example usage:
#   get_release_file_parent_dir "/local/data/pdftotext++/apt-repo" "dists" "bionic"
#
function get_release_file_parent_dir() {
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CODENAME="$3"

  [[ -z "$ROOT_DIR" ]] && { error "[$S] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S] No codename given."; }

  echo "$ROOT_DIR/$(get_release_file_parent_dir_rel "$DISTS_DIR_REL" "$CODENAME")"
}

# This function outputs the *absolute* path to the "Release" file of a specific distribution.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#
# Example usage:
#   get_release_file_path "/local/data/pdftotext++/apt-repo" "dists" "bionic"
#
function get_release_file_path() {
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CODENAME="$3"

  [[ -z "$ROOT_DIR" ]] && { error "[$S] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S] No codename given."; }

  echo "$(get_release_file_parent_dir "$ROOT_DIR" "$DISTS_DIR_REL" "$CODENAME")/Release"
}

# This function outputs the *absolute* path to the "Release.gpg" file of a specific distribution.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#
# Example usage:
#   get_release_gpg_file_path "/local/data/pdftotext++/apt-repo" "dists" "bionic"
#
function get_release_gpg_file_path() {
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CODENAME="$3"

  [[ -z "$ROOT_DIR" ]] && { error "[$S] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S] No codename given."; }

  echo "$(get_release_file_parent_dir "$ROOT_DIR" "$DISTS_DIR_REL" "$CODENAME")/Release.gpg"
}

# This function outputs the *absolute* path to the "InRelease" file of a specific distribution.
#
# Args:
#   $1 - The absolute path to the repository's root directory.
#   $2 - The relative path to the repository's dists directory (specified relative to $1).
#   $3 - The codename of the distribution (for example: "bionic" or "jammy").
#
# Example usage:
#   get_inrelease_file_path "/local/data/pdftotext++/apt-repo" "dists" "bionic"
#
function get_inrelease_file_path() {
  local ROOT_DIR="$1"
  local DISTS_DIR_REL="$2"
  local CODENAME="$3"

  [[ -z "$ROOT_DIR" ]] && { error "[$S] No path to the root directory given."; }
  [[ -z "$DISTS_DIR_REL" ]] && { error "[$S] No path to the dists directory given."; }
  [[ -z "$CODENAME" ]] && { error "[$S] No codename given."; }

  echo "$(get_release_file_parent_dir "$ROOT_DIR" "$DISTS_DIR_REL" "$CODENAME")/InRelease"
}

# ==================================================================================================

# Change the directory before executing the script, so that all relative paths specified in the
# script are interpreted relative to the script (and not to the current working directory).
# After executing the script, change back to the original directory (so that the user does not
# notice the directory change).
PARENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
cd $PARENT_PATH
# Exit when any command fails.
set -e
"$@"
cd $OLDPWD

# ==================================================================================================
#
# [1] https://web.archive.org/web/20230211170217/https://earthly.dev/blog/creating-and-hosting-your-own-deb-packages-and-apt-repo/
#