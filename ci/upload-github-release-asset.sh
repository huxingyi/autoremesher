#!/usr/bin/env bash
#
# Author: Stefan Buck
# License: MIT
# https://gist.github.com/stefanbuck/ce788fee19ab6eb0b4447a85fc99f447
#
# Note: This file has been slightly changed from the original version(Stefan Buck) by adding asset overwrite.
#
# This script accepts the following parameters:
#
# * owner
# * repo
# * tag
# * filename
# * github_api_token
#
# Script to upload a release asset using the GitHub API v3.
#
# Example:
#
# upload-github-release-asset.sh github_api_token=TOKEN owner=stefanbuck repo=playground tag=v0.1.0 filename=./build.zip
#

# Check dependencies.
set -e
xargs=$(which gxargs || which xargs)

# Validate settings.
[ "$TRACE" ] && set -x

CONFIG=$@

for line in $CONFIG; do
  eval "$line"
done

# Define variables.
GH_API="https://api.github.com"
GH_REPO="$GH_API/repos/$owner/$repo"
GH_TAGS="$GH_REPO/releases/tags/$tag"
AUTH="Authorization: token $github_api_token"
WGET_ARGS="--content-disposition --auth-no-challenge --no-cookie"
CURL_ARGS="-LJO#"

if [[ "$tag" == "LATEST" ]]; then
  GH_TAGS="$GH_REPO/releases/latest"
fi

if [ "$branch" == "master" ] || [ "$branch" == "ci" ]; then
  echo "Start upload progress..."
else
  echo "This branch($branch) no need to upload."
  exit 0
fi

# Validate token.
curl -o /dev/null -sH "$AUTH" $GH_REPO || { echo "Error: Invalid repo, token or network issue!";  exit 1; }

# Read asset tags.
response=$(curl -sH "$AUTH" $GH_TAGS)

# Get ID of the release.
eval $(echo "$response" | grep -m 1 "id.:" | grep -w id | tr : = | tr -cd '[[:alnum:]]=')
[ "$id" ] || { echo "Error: Failed to get release id for tag: $tag"; echo "$response" | awk 'length($0)<100' >&2; exit 1; }
release_id="$id"

# Get ID of the asset based on given filename.
id=""
eval $(echo "$response" | grep -C2 "name.:.\+$filename" | grep -m 1 "id.:" | grep -w id | tr : = | tr -cd '[[:alnum:]]=')
assert_id="$id"
if [ "$assert_id" == "" ]; then
    echo "No need to overwrite asset"
else
    echo "Deleting asset($assert_id)... "
    curl "$GITHUB_OAUTH_BASIC" -X "DELETE" -H "Authorization: token $github_api_token" "https://api.github.com/repos/$owner/$repo/releases/assets/$assert_id"
fi

# Upload asset
echo "Uploading asset... "

# Construct url
GH_ASSET="https://uploads.github.com/repos/$owner/$repo/releases/$release_id/assets?name=$(basename $filename)"

response=$(curl "$GITHUB_OAUTH_BASIC" --data-binary @"$filename" -H "Authorization: token $github_api_token" -H "Content-Type: application/octet-stream" $GH_ASSET)
echo $response
founderr=$(echo "$response" | grep "errors\|Error" | wc -l)
if [ "$founderr" -eq "0" ]; then
    echo "Upload success"
else
    echo "Upload failed"
    exit 1;
fi
