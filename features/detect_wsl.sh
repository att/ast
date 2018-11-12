#!/bin/sh
#
# Exit with a zero status if the system appears to be WSL (Windows Subsystem
# for Linux).
uname --kernel-release | grep -q 'Microsoft$'
