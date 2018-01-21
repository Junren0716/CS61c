CFLAGS=-Wno-unused-result -mavx -O3 -std=c99 -fopenmp
all: cnn cnnModule.so

cnn: src/cnn.c src/util.c src/main.c src/timestamp.c
	gcc $(CFLAGS) src/cnn.c -lm -o cnn

cnnModule.so: src/cnn.c src/python.c src/util.c src/timestamp.c
	gcc $(CFLAGS) -shared  -fPIC -I/usr/include/python2.7 -o cnnModule.so src/python.c src/cnn.c

run: cnnModule.so
	@python cnn.py $(port)

benchmark: cnn
	@cd test ; ../cnn benchmark 2400

benchmark-small: cnn
	@cd test ; ../cnn benchmark 1200

benchmark-large: cnn
	@cd test ; ../cnn benchmark 12000

benchmark-huge: cnn
	@cd test ; ../cnn benchmark 24000

test: cnn
	@cd test ; bash run_test.sh

test-huge: cnn
	@cd test ; bash huge_test.sh

clean:
	rm cnn cnnModule.so

.PHONY: run clean benchmark benchmark-small benchmark-large benchmark-huge test
