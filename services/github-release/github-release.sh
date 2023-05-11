#!/bin/bash

PARENT_DIR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
S=$(basename "$0")

function create_release() {
  local VERSION="$1"  # The release version.
  local PACKAGES_DIR="$2"  # The path to the local directory where the packages are stored.
  local OWNER="ad-freiburg"  # The account owner of the GitHub repository.
  local REPO="pdftotext-plus-plus"  # The name of the GitHub repository.
  local ACCESS_TOKEN=$(cat ${PARENT_DIR_PATH}/github-token.local)  # The GitHub access token.
  local TAG_NAME="v${VERSION}"  # The name of the tag.
  local BODY="Release v${VERSION}."  # The release description.

  info_emph "[$S] Creating GitHub release ..."
  debug " • VERSION:      '$VERSION'"
  debug " • PACKAGES_DIR: '$PACKAGES_DIR'"
  debug " • OWNER:        '$OWNER'"
  debug " • REPO:         '$REPO'"
  debug " • TAG_NAME:     '$TAG_NAME'"
  debug " • BODY:         '$BODY'"

  # Validate the arguments.
  [[ -z "$VERSION" ]] && { error "[$S] No version given."; }
  [[ -z "$PACKAGES_DIR" ]] && { error "[$S] No packages directory given."; }
  [[ -z "$OWNER" ]] && { error "[$S] No repository owner given."; }
  [[ -z "$REPO" ]] && { error "[$S] No repository given."; }
  [[ -z "$ACCESS_TOKEN" ]] && { error "[$S] No access token given."; }
  [[ -z "$TAG_NAME" ]] && { error "[$S] No tag name given."; }
  [[ -z "$BODY" ]] && { error "[$S] No body given."; }

  info "[$S] Trying to create the GitHub release ..."
  local CREATE_RELEASE_RESPONSE=$(curl -sL \
    -X POST \
    -H "Accept: application/vnd.github+json" \
    -H "Authorization: Bearer ${ACCESS_TOKEN}"\
    -H "X-GitHub-Api-Version: 2022-11-28" \
    https://api.github.com/repos/${OWNER}/${REPO}/releases \
    -d "{\"tag_name\": \"v${VERSION}\", \"target_commitish\": \"master\", \"body\": \"${BODY}\"}")

  # Check if the GitHub response contains an error.
  local RELEASE_ID=$(echo "$CREATE_RELEASE_RESPONSE" | jq -r '.id')
  if [ "$RELEASE_ID" = "null" ]; then
    error "Could not create the GitHub release:\n$CREATE_RELEASE_RESPONSE"
    exit 1
  fi

  # Extract the release id from the GitHub response.
  info "Successfully created GitHub release."
  debug " • RELEASE_ID: '$RELEASE_ID'"

  # Upload each package in the packages directory that contains the specified version.
  info "[$S] Uploading packages ..."
  for DEB in ${PACKAGES_DIR}/*${VERSION}*.deb; do
      local FILENAME=$(basename $DEB)
      local DISTRIBUTION="$(dpkg -f "$DEB" Distribution)"
      local ARCH="$(dpkg -f "$DEB" Architecture)"
      info "[$S] Uploading package '$DEB' ..."
      debug " • FILENAME: '$FILENAME'"
      debug " • DISTRIBUTION: '$DISTRIBUTION'"
      debug " • ARCHITECTURE: '$ARCH'"

      local UPLOAD_PACKAGE_RESPONSE=$(curl -sL \
        -X POST \
        -H "Accept: application/vnd.github+json" \
        -H "Authorization: Bearer ${ACCESS_TOKEN}"\
        -H "X-GitHub-Api-Version: 2022-11-28" \
        -H "Content-Type: application/octet-stream" \
        "https://uploads.github.com/repos/${OWNER}/${REPO}/releases/${RELEASE_ID}/assets?name=${FILENAME}&label=DEB%20package%20(${DISTRIBUTION// /%20}%3B%20${ARCH})" \
        --data-binary "@${DEB}")

      # Check if the GitHub response contains an error.
      local UPLOAD_PACKAGE_IS_ERROR=$(echo "$UPLOAD_PACKAGE_RESPONSE" | jq 'has("errors")')
      if [ "$UPLOAD_PACKAGE_IS_ERROR" = "true" ]; then
        error "Could not upload package \"${DEB}\":\n$UPLOAD_PACKAGE_RESPONSE"
        exit 1
      fi
  done
}

# Exit when any command fails.
set -e

source $PARENT_DIR_PATH/../../scripts/log.sh

"$@"
