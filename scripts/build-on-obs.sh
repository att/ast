#!/bin/bash

set -ex

if [[ -z $OBS_REPO ]]; then
    echo "OBS_REPO should be set to path to obs repository"
    exit 1
fi

# Build from master branch
git checkout master 2> /dev/null
last_commit=$(git rev-parse HEAD)
git pull origin master

# Fetch tags to determine version number
git fetch --tags

# Check if new commits were added since last build
if [[ $last_commit ==  $(git rev-parse HEAD) ]]; then
    echo "No new changes since last build. Exiting..."
    exit 0
fi

# Remove older files first
pushd "${OBS_REPO}"
osc rm -f -- *
popd

# This is the ksh version number seen inside scripts
# For e.g. with ksh --version
# 2017.0.0-devel-1535-g7c33a1cd-dirty
VCS_VERSION=$(git describe --always --dirty --tags)

# Extract development version number i.e. 2017.0.0
DEVEL_VERSION_NUM=$(echo "$VCS_VERSION" | cut -d'-' -f1)

# 1535
COMMIT_NUM=$(echo "$VCS_VERSION" | cut -d'-' -f3)

# g7c33a1cd
SHORT_COMMIT=$(echo "$VCS_VERSION" | cut -d'-' -f4)

COMMIT=$(git rev-parse HEAD)
# shellcheck disable=SC1117
CHANGELOG_DATE=$(date +"%a\, %d %b %Y %R:%S %z")

# This is version number of rpm and debian packages
VERSION=${DEVEL_VERSION_NUM}+git.${COMMIT_NUM}.${SHORT_COMMIT}

sed "s,#VERSION#,${VERSION},;
     s,#VCS_VERSION#,${VCS_VERSION},;
     s,#COMMIT#,${COMMIT},;" \
         packaging/opensuse/ksh.spec.in > "${OBS_REPO}/ksh.spec"

# Make all debian related changes in a temporary branch
git branch -D debian_build
git checkout -b debian_build

sed  -i "s,#VCS_VERSION#,${VCS_VERSION},;" \
         packaging/debian/rules

sed  -i "s,#COMMIT#,${COMMIT},;
        s,#VERSION#,${VERSION},;
        s,#CHANGELOG_DATE#,${CHANGELOG_DATE},;" \
         packaging/debian/changelog

# OBS looks for debian package files under root directory
git mv packaging/debian debian

git commit -a -m "Update debian packaging files"

# tar czvfh $OBS_REPO/ksh_${COMMIT_DATE}.debian.tar.gz debian
git archive --prefix "ast-${COMMIT}/" --format "tar.gz" HEAD -o "${OBS_REPO}/ksh_${VERSION}.orig.tar.gz"

MD5SUM=$(md5sum  "${OBS_REPO}/ksh_${VERSION}.orig.tar.gz" | cut -f1 -d' ')
FILESIZE=$(stat --printf "%s"  "${OBS_REPO}/ksh_${VERSION}.orig.tar.gz")
sed "s,#VERSION#,${VERSION},;
     s,#MD5SUM#,${MD5SUM},;
     s,#FILESIZE#,${FILESIZE}," \
        debian/ksh.dsc > "${OBS_REPO}/ksh_${VERSION}.dsc"

pushd "${OBS_REPO}"
osc addremove
osc commit -m "Build from ${COMMIT}"
popd
