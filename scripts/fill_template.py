#!/usr/bin/env python
import sys

# Convert the sequence of key/value pairs on the command line into a dict.
# Note that the first element of sys.argv is the script name which we ignore.
i = iter(sys.argv[1:])
data = dict(zip(i, i))

# Slurp the template from stdin.
template = sys.stdin.read()

# Sub the data into the template.
s = template.format(**data)

# Write the document to stdout.
print(s)
