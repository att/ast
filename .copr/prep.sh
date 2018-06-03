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

COMMIT=$(git rev-parse HEAD)
COMMIT_SHORT=$(git rev-parse --short HEAD)
COMMIT_NUM=$(git rev-list HEAD --count)
COMMIT_DATE=$(date --date="@$(git show -s --format=%ct HEAD)" +%Y%m%d)
VCS_VERSION=$(git describe --always --dirty --tags)

sed "s,#COMMIT#,${COMMIT},;
     s,#SHORTCOMMIT#,${COMMIT_SHORT},;
     s,#COMMITNUM#,${COMMIT_NUM},;
     s,#COMMITDATE#,${COMMIT_DATE},;
     s,#VCS_VERSION#,${VCS_VERSION}," \
         packaging/fedora/ksh.spec.in > packaging/fedora/ksh.spec

git archive --prefix "ast-${COMMIT}/" --format "tar.gz" HEAD -o "packaging/fedora/ksh-${COMMIT_SHORT}.tar.gz"
