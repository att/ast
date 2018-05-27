#!/bin/bash

# Variable OBS_REPO should be set to path to obs repository
set -x

git checkout master 2> /dev/null
last_commit=$(git rev-parse HEAD)
git pull origin master

if [[ $last_commit ==  $(git rev-parse HEAD) ]]; then
    echo "No new changes since last build. Exiting..."
    exit 0
fi

# Remove older files first
pushd "${OBS_REPO}"
osc rm -f *.tar.gz
popd

COMMIT=$(git rev-parse HEAD)
COMMIT_SHORT=$(git rev-parse --short HEAD)
COMMIT_NUM=$(git rev-list HEAD --count)
COMMIT_DATE=$(date --date="@$(git show -s --format=%ct HEAD)" +%Y%m%d)

sed "s,#COMMIT#,${COMMIT},;
     s,#SHORTCOMMIT#,${COMMIT_SHORT},;
     s,#COMMITNUM#,${COMMIT_NUM},;
     s,#COMMITDATE#,${COMMIT_DATE}," \
         packaging/opensuse/ksh.spec.in > "${OBS_REPO}/ksh.spec"

git archive --prefix "ast-${COMMIT}/" --format "tar.gz" HEAD -o "${OBS_REPO}/ksh-${COMMIT_SHORT}.tar.gz"

pushd ${OBS_REPO}
osc add ksh-${COMMIT_SHORT}.tar.gz
osc add ksh.spec
osc commit -m "ksh-${COMMIT_SHORT}"
popd
