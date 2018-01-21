import sys

layers = 12
delta = 0.00000000001

if len(sys.argv) < 3:
	print 'Usage: python compare_layers.py <file> <reference>'
	sys.exit(2)

with open(sys.argv[1], 'r') as fin:
	indata = fin.readlines()

with open(sys.argv[2], 'r') as fref:
	refdata = fref.readlines()

for i in range(layers):
	invals = indata[i].split(',')
	if invals[0] != 'LAYER' + str(i):
		print 'ERROR: Invalid input data for layer ' + str(i)
		sys.exit(2)

	refvals = refdata[i].split(',')
	if refvals[0] != 'LAYER' + str(i):
		print 'ERROR: Invalid reference data for layer ' + str(i)
		sys.exit(2)

	if len(refvals) != len(invals):
		print 'ERROR: Dimensionality error in layer %d (expected: %d, was: %d)' % \
			(i, len(refvals), len(invals))
		sys.exit(2)

	for j in range(1, len(invals)):
		if abs(float(invals[j])-float(refvals[j])) > delta:
			print 'ERROR: Value %d at layer %d is wrong: %f (should be %f)' % \
				(j, i, float(invals[j]), float(refvals[j]))
			sys.exit(1)

print 'OK'
