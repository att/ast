#!/bin/sh -euf
set -x

# Copr sets clone depth to 500 to avoid cloning big repositories
# however we use number of commits in version number, so fetch
# full repository
# If `get fetch --unshallow` fails, this means if we have full
# repo, so ignore if it fails.
git fetch --unshallow || :

# Fetch tags to determine version number
git fetch --tags

# 2017.0.0-devel-1535-g7c33a1cd-dirty
VCS_VERSION=$(git describe --always --dirty --tags)

# Extract development version number i.e. 2017.0.0
DEVEL_VERSION_NUM=$(echo "$VCS_VERSION" | cut -d'-' -f1)

# 1535
COMMIT_NUM=$(echo "$VCS_VERSION" | cut -d'-' -f3)

# g7c33a1cd
SHORT_COMMIT=$(echo "$VCS_VERSION" | cut -d'-' -f4)

COMMIT=$(git rev-parse HEAD)

sed "s,#VCS_VERSION#,${VCS_VERSION},;
     s,#DEVEL_VERSION_NUM#,${DEVEL_VERSION_NUM},;
     s,#COMMIT_NUM#,${COMMIT_NUM},;
     s,#SHORT_COMMIT#,${SHORT_COMMIT},;
     s,#COMMIT#,${COMMIT}," \
         packaging/fedora/ksh.spec.in > packaging/fedora/ksh.spec

git archive --prefix "ast-${COMMIT}/" --format "tar.gz" HEAD -o "packaging/fedora/ksh-${SHORT_COMMIT}.tar.gz"
