import sys
import string

f = open("Heart Of The Alien (U).iso", "rb")
original = f.read()
f.close()

f = open(sys.argv[1], "rb")
raw = f.read()
f.close()

offset = string.index(original, raw)
print "{\"%s\", 0x%x, 0x%x}," % (sys.argv[1], offset, len(raw))
assert offset >= 0
