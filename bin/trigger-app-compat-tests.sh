#!/bin/bash
# Get current branch name
# shellcheck disable=SC1117
current=$(git rev-parse --abbrev-ref HEAD | tr -d "\n")
git branch -D application_compatibility 2>/dev/null
git checkout -b application_compatibility
# Use custom .travis.yml file
cp app-compat-travis.yml .travis.yml
git commit -a -m "Trigger application compatibility tests"
git push --force -u origin application_compatibility
# Switch back to previous branch
git checkout "$current"
