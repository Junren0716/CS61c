import os
import sys

if __name__ == '__main__':
  assert(len(sys.argv) == 3)
  master = open("master").read().strip()
  os.system('scp -r -i %s %s root@%s:~/' % (sys.argv[1], sys.argv[2], master))
