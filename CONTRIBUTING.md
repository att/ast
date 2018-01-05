# Guidelines For Developers

This document provides guidelines for making changes to the AST Korn
shell project. This includes rules for how to format the code, naming
conventions, etcetera. Generally known as the style of the code. It
also includes recommended best practices such as creating a Travis-CI
account so you can verify your changes pass all the tests before making
a pull-request.

See the bottom of this document for help on installing the linting and
style reformatting tools discussed in the following sections.

AST Korn shell source requires, and should limit the C features it
uses, to those available in C99 (aka ISO/IEC 9899:1999). See
[issue #145](https://github.com/att/ast/issues/145).

Before introducing a new dependency, please make it optional with graceful
failure if possible. Add any new dependencies to the README.md document
under the *Running* and/or *Building* sections.

## Versioning

This is TBD. For the moment we're using the last version assigned to the
code, `93v-`, since we have not made any enhancements to the code. It is
highly likely we'll switch to [semantic versioning](https://semver.org/).

## Include What You Use

You should not depend on symbols being visible to a `*.c` module from
`#include` statements inside another header file. In other words if your
module does `#include "defs.h"` and that header does `#include "name.h"`
your module should not assume the sub-include is present. It should instead
directly `#include "name.h"` if it needs any symbol from that header. That
makes the actual dependencies much clearer. It also makes it easy to modify
the headers included by a specific header file without having to worry
that will break any module (or header) that includes a particular header.

To help enforce this rule the `bin/lint` command will run the
[include-what-you-use](http://include-what-you-use.org/)
tool. You can find the IWYU project on
[github](https://github.com/include-what-you-use/include-what-you-use).

To install the tool on OS X you'll need to add a
[formula](https://github.com/jasonmp85/homebrew-iwyu) then install it:

```
brew tap jasonmp85/iwyu
brew install iwyu
```

On Ubuntu you can install it via `sudo apt-get install iwyu`.

## Lint Free Code

Automated analysis tools like cppcheck and oclint can point out potential
bugs or code that is extremely hard to understand. They also help ensure
the code has a consistent style and that it avoids patterns that tend to
confuse people.

Ultimately we want lint free code. This includes no compiler warnings and
no warnings from the aforementioned tools. However, at the moment a lot
of cleanup is required to reach that goal. For now simply try to avoid
introducing new lint.

To make linting the code easy there is the `bin/lint` command. If you
pass it the magic string `all` it will lint all the code. If you pass
it a list of files it will lint those. If run with no arguments it will
lint any uncommitted source files. If there are no uncommitted files it
will lint the files in the most recent commit.

### Dealing With Lint Warnings

You are strongly encouraged to address a lint warning by refactoring
the code, changing variable names, adding an explicit initialization,
or whatever action is implied by the warning.

### Suppressing Lint Warnings

Once in a while the lint tools emit a false positive warning. For example,
cppcheck might suggest a memory leak is present when that is not the
case. To suppress that cppcheck warning you should insert a line like
the following immediately prior to the line cppcheck warned about:

```
// cppcheck-suppress memleak // addr not really leaked
```

Suppressing oclint warnings is more complicated
to describe so I'll refer you to the
[OCLint HowTo](http://docs.oclint.org/en/latest/howto/suppress.html#annotations)
on the topic.

## Ensuring Your Changes Conform to the Style Guides

The following sections discuss the specific rules for the style that
should be used when writing AST ksh code. To ensure your changes conform
to the style rules you simply need to run

```
bin/style
```

before commiting your change. That will run `git-clang-format` to rewrite
just the lines you're modifying.

If you've already committed your changes that's okay since it will then
check the files in the most recent commit. This can be useful after
you've merged someone elses change and want to check that it's style
is correct. However, in that case it will run `clang-format` to ensure
the entire file, not just the lines modified by the commit, conform to
the style.

If you want to check the style of the entire code base run

```
bin/style all
```

That command will refuse to restyle any files if you have uncommitted changes.

### Configuring Your Editor

#### ViM

If you use ViM I recommend the [vim-clang-format plugin](https://github.com/rhysd/vim-clang-format) by [@rhysd](https://github.com/rhysd).

You can also get ViM to provide reasonably correct behavior by installing

http://www.vim.org/scripts/script.php?script_id=2636

#### Emacs

If you use Emacs: TBD

### Configuring Your Editor for Ksh Scripts

TBD

### Suppressing Reformatting of C Code

If you have a good reason for doing so you can tell `clang-format` to
not reformat a block of code by enclosing it in comments like this:

```
// clang-format off
code to ignore
// clang-format on
```

However, as I write this there are no places in the code where we use
this and I can't think of any legitimate reasons for exempting blocks of
code from clang-format.

## Ksh Script Style Guide

TBD

## C Style Guide

Note: While `clang-format` is authoritative with respect to everything it
handles (e.g., indentation, spacing around operators) there are some things it
won't fix. So please read the following items.

1. The [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
forms the basis of ths projects C style guide. There are two major
deviations for this project. First, a four, rather than two, space
indent. Second, line lengths up to 100, rather than 80, characters. See
[issue #125](https://github.com/att/ast/issues/125).

1. The `clang-format` command is authoritative with respect to indentation,
whitespace around operators, line breaks, etc.

1. All names in code should be `small_snake_case`. No Hungarian notation is used.

1. Always attach braces to the surrounding context.

1. Indent with spaces, not tabs and use four spaces per indent.

1. Comments should always use the C++ style; i.e., each line of the
comment should begin with a `//` and should be limited to 100 characters.

1. Comments that appear on the same line as a statement should be separated
from the previous text by two spaces. The comment should not be in the
form of a sentence; i.e., `// allow room to prepend args` not `// Allow
room to prepend args.`. If the comment is on its own line(s) it should
be written as a sequence of sentences like you would in any document.

1. All switch `case` blocks should be enclosed in braces. For example:

        ```
        switch (x) {
            case 1: {
                do_something();
                break;
            }
            case 2: {
                do_something_else();
                break;
            }
        }
        ```

1. If a switch `case` block is meant to fall-through to the following
block add an explicit comment: `// FALL THRU`.

1. If a `if` statement has a corresponding `else if` or `else` block you
must put the blocks on separate lines enclosed in braces even if they
would otherwise fit on the preceding control statement. For example:

    ```
    if (a) {
        do_a();
    } else {
        abort();
    }
    ```

1. A `if` or `while` statement whose block is a single line can put the
two statements on the same line if there is room. In that case omit the
braces. Otherwise, even if the block is a single line it must be enclosed
in braces. For example:

        ```
        if (a || b) do_something();
        if (some_really_long_complex_condition_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx) {
            do_something();
        }
        ```

        Never do this even though the C language allows it since it is an
        anti-pattern that leads to bugs:

        ```
        if (some_really_long_complex_condition_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx)
            do_something();

        if (is_true()) {
            do_x();
            do_y();
        } else
            do_z();
        ```

## Testing

The source code for ksh includes a large collection of tests. If you
are making any changes to ksh, running these tests is mandatory to
make sure the behaviour remains consistent and regressions are not
introduced. Even if you don't run the tests they will be run via the
[Travis CI](https://travis-ci.org/att/ast) service.

You are strongly encouraged to add tests when changing the functionality
of ksh. Especially if you are fixing a bug to help ensure there are no
regressions in the future (i.e., we don't reintroduce the bug).

### Local testing

The tests can be run on your local computer on all operating systems supported
by the project. To run the tests:

```
cd build
meson test
```

To run a specific test include its name: `meson test types`.

### Testing with Valgrind

The valgrind tool is invaluable for finding bugs that may only manifest in
specific situations due to the vagaries of memory management and the placement
of variables, structures, etc. To run the tests under control of Valgrind do
this:

```
meson test -t 10 --wrapper valgrind
```

The `-t 10` is a multipler for test timeouts. A much larger multipler,
on the order of `-t 50`, might be needed if you're running the tests in
a virtual machine or other environment with highly constrained resources.

### Travis CI Build and Test

The Travis Continuous Integration services can be used to test your
changes using multiple configurations. This is the same service that the
AST Korrc/cmd/ksh93/data/bash_pre_rc.c shell project uses to ensure new
changes haven't broken anything. Thus it is a really good idea that you
leverage Travis CI before making a pull-request to avoid embarrasment at
breaking the build.

You will need to [fork the repository on GitHub](https://help.github.com/articles/fork-a-repo/).
Then setup Travis to test your changes before you make a pull-request:

1. [Sign in to Travis CI](https://travis-ci.org/auth) with your GitHub
account, accepting the GitHub access permissions confirmation.

1. Once you're signed in, and your repositories are synchronised, go to your [profile page](https://travis-ci.org/profile) and enable the ast repository.

1. Push your changes to GitHub.

You'll receive an email when the tests are complete telling you whether or not any tests failed.

You'll find the configuration used to control Travis in the `.travis.yml` file.

### Git hooks

Since developers sometimes forget to run the tests, it can be helpful to
use git hooks (see githooks(5)) to automate it.

One possibility is a pre-push hook script like this one:

```sh
#!/bin/sh
#### A pre-push hook for the ast/ksh project
# This will run the tests when a push to master is detected, and will stop that if the tests fail
# Save this as .git/hooks/pre-push and make it executable

protected_branch='master'

# Git gives us lines like "refs/heads/frombranch SOMESHA1 refs/heads/tobranch SOMESHA1"
# We're only interested in the branches
while read from _ to _; do
    if [ "x$to" = "xrefs/heads/$protected_branch" ]; then
        isprotected=1
    fi
done
if [ "x$isprotected" = x1 ]; then
    echo "Running tests before push to master"
    cd build
    meson test
    RESULT=$?
    if [ $RESULT -ne 0 ]; then
        echo "Tests failed for a push to master, we can't let you do that" >&2
        exit 1
    fi
fi
exit 0
```

This will check if the push is to the master branch and, if it is,
will run `meson test` and only allow the push if that succeeds. In some
circumstances it might be advisable to circumvent it with `git push
--no-verify`, but usually that should not be necessary.

To install the hook, put it in .git/hooks/pre-push and make it executable.

### Coverity Scan

TBD

## Installing the Required Tools

### Installing the Linting Tools

To install the lint checkers on Mac OS X using HomeBrew:

```
brew tap oclint/formulae
brew install oclint
brew install cppcheck
```

To install the lint checkers on Linux distros that use Apt:

```
sudo apt-get install clang
sudo apt-get install oclint
sudo apt-get install cppcheck
```

### Installing the Reformatting Tools

To install the reformatting tool on Mac OS X using HomeBrew:

```
brew install clang-format
```

To install the reformatting tool on Linux distros that use Apt:

```
apt-cache install clang-format
```

That will list the versions available. Pick the newest one available
(3.9 for Ubuntu 16.10 as I write this) and install it:

```
sudo apt-get install clang-format-3.9
sudo ln -s /usr/bin/clang-format-3.9 /usr/bin/clang-format
```

## Message Translations

TBD
