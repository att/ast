vcs_version = vcs_tag(command: ['git', 'describe', '--always', '--dirty', '--tags'],
                      input: 'version.h.in', output: 'version.h', fallback: get_option('fallback-version-number'))

# Yes, vcs_version is a header rather than a .c file but we have to define a dependency to force it
# to ge generated.
ksh93_files += [vcs_version]
