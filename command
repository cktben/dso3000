#!/usr/bin/python

import sys
from dso3000 import *

if len(sys.argv) < 2:
	print >> sys.stderr, 'Usage: %s <command>' % sys.argv[0]
	sys.exit(1)

text = ' '.join(sys.argv[1:])

scope = DSO3000()
result = scope.command(text)
if result:
	print result
