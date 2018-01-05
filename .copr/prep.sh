#!/bin/sh -euf

COMMIT=$(git rev-parse HEAD)
COMMIT_SHORT=$(git rev-parse --short HEAD)
COMMIT_NUM=$(git rev-list HEAD --count)
COMMIT_DATE=$(date --date="@$(git show -s --format=%ct HEAD)" +%Y%m%d)

sed "s,#COMMIT#,${COMMIT},;
     s,#SHORTCOMMIT#,${COMMIT_SHORT},;
     s,#COMMITNUM#,${COMMIT_NUM},;
     s,#COMMITDATE#,${COMMIT_DATE}," \
         packaging/fedora/ksh.spec.in > packaging/fedora/ksh.spec

git archive --prefix "ast-${COMMIT}/" --format "tar.gz" "${COMMIT}" -o "packaging/fedora/ksh-${COMMIT_SHORT}.tar.gz"
