#!/bin/bash
brew update
brew install meson
meson build && ninja -C build
