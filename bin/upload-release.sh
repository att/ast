#!/bin/bash
set -x
SELF="$0"
TAG="$1"
TOKEN="$2"

NV="ksh-${TAG}"

usage() {
    printf "Usage: %s TAG TOKEN\n" "$SELF" >&2
    exit 1
}

die() {
    printf "%s: error: %s\n" "$SELF" "$*" >&2
    exit 1
}

# check arguments
test "$TAG" = "$(git describe --tags "$TAG")" || usage
test -n "$TOKEN" || usage

# dump a tarball
SRC_TAR="${NV}.tar"
git archive --prefix="$NV/" --format="tar" HEAD -- . > "$SRC_TAR" \
                                        || die "failed to export sources"

# produce .tar.gz
TAR_GZ="${NV}.tar.gz"
gzip -c "$SRC_TAR" > "$TAR_GZ"          || die "failed to create $TAR_GZ"

# produce .tar.xz
TAR_XZ="${NV}.tar.xz"
xz -c "$SRC_TAR" > "$TAR_XZ"            || die "failed to create $TAR_XZ"

# sign the tarballs
for file in "$TAR_GZ" "$TAR_XZ"; do
    gpg --armor --detach-sign "$file" || die "tarball signing failed"
    test -f "${file}.asc" || die "tarball signature was not created"
done

# I did these steps manually during ksh-2020.0.0 alpha release.
# TODO: Find out why these API calls fail 
# file to store response from GitHub API
# JSON="./${NV}-github-release.js"
# 
# # create a new release on GitHub
# curl "https://api.github.com/repos/att/ast/releases" \
#     -o "$JSON" --fail --verbose \
#     --header "Authorization: token $TOKEN" \
#     --data '{
#     "tag_name": "'"$TAG"'",
#     "target_commitish": "master",
#     "name": "'"$NV"'",
#     "draft": false,
#     "prerelease": true
# }' || exit $?
# 
# # parse upload URL from the response
# UPLOAD_URL="$(grep '^ *"upload_url": "' "$JSON" \
#     | sed -e 's/^ *"upload_url": "//' -e 's/{.*}.*$//')"
# grep '^https://uploads.github.com/.*/assets$' <<< "$UPLOAD_URL" || exit $?
# 
# # upload both .tar.gz and .tar.xz
# for comp in gzip xz; do
#     file="${NV}.tar.${comp:0:2}"
#     curl "${UPLOAD_URL}?name=${file}" \
#         -T "$file" --fail --verbose \
#         --header "Authorization: token $TOKEN" \
#         --header "Content-Type: application/x-${comp}" \
# 	|| exit $?
# done
# 
# # upload signatures
# for file in "${TAR_GZ}.asc" "${TAR_XZ}.asc"; do
#     curl "${UPLOAD_URL}?name=${file}" \
#         -T "$file" --fail --verbose \
#         --header "Authorization: token $TOKEN" \
#         --header "Content-Type: text/plain" \
# 	|| exit $?
# done
