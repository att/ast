#!/bin/bash
brew install meson
export CFLAGS='-fno-strict-aliasing -Wno-unknown-pragmas -Wno-missing-braces -Wno-unused-result -Wno-return-type -Wno-int-to-pointer-cast -Wno-parentheses -Wno-unused -Wno-char-subscripts';
meson build && ninja -C build
