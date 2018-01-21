#!/bin/bash

# Just run this file and you can test your circ files! Make sure your files are in the directory above this one though! From William Huang
cp alu.circ test-files
cp regfile.circ test-files
cd test-files
./autograder_limited.py -here
cd ..