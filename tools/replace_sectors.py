import sys
import string

f = open("hota-patched.iso", "rb")
original = f.read()
f.close()

f = open(sys.argv[1], "rb")
replace_file = f.read()
f.close()

f = open(sys.argv[2], "rb")
replace_with = f.read()
f.close()

diff = len(replace_file) - len(replace_with)
if diff > 0:
	print "padding %d bytes" % diff
	replace_with = string.ljust(replace_with, len(replace_file))
elif diff < 0:
	print "removing last %d bytes" % (-diff)
	replace_with = replace_with[0:len(replace_file)]

assert len(replace_file) == len(replace_with)

offset = string.index(original, replace_file)
print "found at 0x%x" % offset
assert offset >= 0

hacked = original[0:offset] + replace_with + original[offset+len(replace_with):]

f = open("hota-patched.iso", "wb")
f.write(hacked)
f.close()
