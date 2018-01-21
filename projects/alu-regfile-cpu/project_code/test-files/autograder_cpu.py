#!/usr/bin/env python

import autograder_base
import os.path

from autograder_base import file_locations, AbsoluteTestCase, FractionalTestCase, main

tests = [
  ("CPU add test",FractionalTestCase(os.path.join(file_locations,'CPU-add.circ'),os.path.join(file_locations,'out-files/CPU-add.out'),1)),
  ("CPU addiu test",FractionalTestCase(os.path.join(file_locations,'CPU-addiu.circ'),os.path.join(file_locations,'out-files/CPU-addiu.out'),1)),
  ("CPU lui test",FractionalTestCase(os.path.join(file_locations,'CPU-lui.circ'),os.path.join(file_locations,'out-files/CPU-lui.out'),1)),
  ("CPU ori test",FractionalTestCase(os.path.join(file_locations,'CPU-ori.circ'),os.path.join(file_locations,'out-files/CPU-ori.out'),1)),
  ("CPU slt test",FractionalTestCase(os.path.join(file_locations,'CPU-slt.circ'),os.path.join(file_locations,'out-files/CPU-slt.out'),1)),
  ("CPU jr test",FractionalTestCase(os.path.join(file_locations,'CPU-jr.circ'),os.path.join(file_locations,'out-files/CPU-jr.out'),1)),
  ("CPU halt test",FractionalTestCase(os.path.join(file_locations,'CPU-halt.circ'),os.path.join(file_locations,'out-files/CPU-halt.out'),1)),
  ("CPU branches test",FractionalTestCase(os.path.join(file_locations,'CPU-branches.circ'),os.path.join(file_locations,'out-files/CPU-branches.out'),1)),
  ("CPU divsubu and mfhi test",FractionalTestCase(os.path.join(file_locations,'CPU-divMfhi.circ'),os.path.join(file_locations,'out-files/CPU-divMfhi.out'),1)),
]

if __name__ == '__main__':
  main(tests)
