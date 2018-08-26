#!/bin/bash
# Get current branch name
# shellcheck disable=SC1117
current=$(git rev-parse --abbrev-ref HEAD | tr -d "\n")
# Coverity scan is triggered only for coverity_scan branch
git branch -D coverity_scan 2>/dev/null
git checkout -b coverity_scan
# Use custom .travis.yml file for coverity
cp coverity-travis.yml .travis.yml
git commit -a -m "Trigger coverity scan"
git push --force -u origin coverity_scan
# Switch back to previous branch
git checkout "$current"
