CUNIT := -L/home/ff/cs61c/cunit/install/lib -I/home/ff/cs61c/cunit/install/include -lcunit

beargit: main.c beargit.c util.c beargit.h util.h
	gcc -g -std=c99 main.c beargit.c util.c -o beargit

beargit-unittest: main.c beargit.c cunittests.c util.c beargit.h util.h cunittests.h
	gcc -g -DTESTING -std=c99 main.c beargit.c cunittests.c util.c -o beargit-unittest $(CUNIT)

clean:
	rm -rf beargit autotest test beargit-unittest

check: beargit
ifndef INSTMANPATH
	$(error You need to run 'make check' on an instructional machine, such as hive*.cs.berkeley.edu)
endif
	python2.7 tester.pyc beargit.c
