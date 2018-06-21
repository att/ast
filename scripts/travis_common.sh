#
# This is meant to be sourced by the script that actually manages the build
# and running the unit tests.
#
export LANG=en_US.UTF-8
export CFLAGS="-fno-strict-aliasing -Wno-unknown-pragmas -Wno-missing-braces"
CFLAGS="$CFLAGS -Wno-unused-result -Wno-return-type -Wno-int-to-pointer-cast"
CFLAGS="$CFLAGS -Wno-parentheses -Wno-unused -Wno-unused-but-set-variable -Wno-cpp"

# Removing all the TRAVIS_*, NodeJS, Ruby and other irrelevant env vars. This
# makes a huge difference in the size of the unit test logfile.
unset GEM_HOME
unset GEM_PATH
unset HAS_JOSH_K_SEAL_OF_APPROVAL
unset MANPATH
unset MY_RUBY_HOME
for v in $(env | sed -n -e '/^TRAVIS/s/=.*//p' -e '/^NVM/s/=.*//p' -e '/^rvm/s/=.*//p')
do
    unset $v
done
