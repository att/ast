documentation_module = configure_file(output : 'documentation.c',
                                      command: [source_dir + '/scripts/builtin_documentation.sh'])

ksh93_files += [
    'data/aliases.c',
    'data/builtins.c',
    'data/keywords.c',
    'data/lexstates.c',
    'data/limits.c',
    'data/msg.c',
    'data/options.c',
    'data/signals.c',
    'data/strdata.c',
    'data/testops.c',
    'data/variables.c',
    # This file is dynamically constructed by the configure_file() directive above. It will be
    # removed in the future when the documentation format is changed to something more modern.
    # See https://github.com/att/ast/issues/507
    documentation_module,
]
