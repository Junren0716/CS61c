# Build file for CS61C MapReduce Lab
# You should not need to edit this file.

# This file requires GNU make and depends on paths on instruction machines.

####

## Variables

# Source files (java code). wildcard selects all files matching a pattern.
SOURCES = $(wildcard *.java)
# Output JAR file
TARGET = wc.jar
# Extra JARs to have on the classpath when compiling.
CLASSPATH = /home/ff/cs61c/hadoop/hadoop-core.jar:/home/ff/cs61c/hadoop/lib/commons-cli.jar
# javac command to use
JAVAC = javac -g -deprecation -cp $(CLASSPATH)
# jar command to use
JAR = jar

## Make targets

# General form is target: dependencies (targets or files), followed by
# commands to run to build the target from the dependencies.

# Default target.
all: $(TARGET)

$(TARGET): classes $(SOURCES)
	$(JAVAC) -d classes $(SOURCES)
	$(JAR) cf $(TARGET) -C classes .

wordcount-small: $(TARGET)
	rm -rf wc-out-wordcount-small
	hadoop jar wc.jar WordCount ~cs61c/data/billOfRights.txt.seq wc-out-wordcount-small

wordcount-medium: $(TARGET)
	rm -rf wc-out-wordcount-medium
	hadoop jar wc.jar WordCount ~cs61c/data/complete-works-mark-twain.txt.seq wc-out-wordcount-medium

wordcount: $(TARGET)
	rm -rf wc-out-wordcount
	hadoop jar wc.jar WordCount $(myinput) wc-out-wordcount

docwordcount-small: $(TARGET)
	rm -rf wc-out-docwordcount-small
	hadoop jar wc.jar DocWordCount ~cs61c/data/billOfRights.txt.seq wc-out-docwordcount-small

docwordcount-medium: $(TARGET)
	rm -rf wc-out-docwordcount-medium
	hadoop jar wc.jar DocWordCount ~cs61c/data/complete-works-mark-twain.txt.seq wc-out-docwordcount-medium

docwordcount: $(TARGET)
	rm -rf wc-out-docwordcount
	hadoop jar wc.jar DocWordCount $(myinput) wc-out-docwordcount


sparkwc-small:
	rm -rf spark-wc-out-wordcount-small
	spark-submit wordcount.py /home/ff/cs61c/data/billOfRights.txt.seq spark-wc-out-wordcount-small

sparkwc-medium:
	rm -rf spark-wc-out-wordcount-medium
	spark-submit wordcount.py /home/ff/cs61c/data/complete-works-mark-twain.txt.seq spark-wc-out-wordcount-medium

sparkwc:
	rm -rf spark-wc-out-wordcount
	spark-submit wordcount.py $(myinput) spark-wc-out-wordcount

sparkdwc-small:
	rm -rf spark-wc-out-docwordcount-small
	spark-submit docwordcount.py /home/ff/cs61c/data/billOfRights.txt.seq spark-wc-out-docwordcount-small

sparkdwc-medium:
	rm -rf spark-wc-out-docwordcount-medium
	spark-submit docwordcount.py /home/ff/cs61c/data/complete-works-mark-twain.txt.seq spark-wc-out-docwordcount-medium

sparkdwc:
	rm -rf spark-wc-out-docwordcount
	spark-submit docwordcount.py $(myinput) spark-wc-out-docwordcount


index-small:
	rm -rf spark-wc-out-index-small
	spark-submit index.py /home/ff/cs61c/data/billOfRights.txt.seq spark-wc-out-index-small

index-medium:
	rm -rf spark-wc-out-index-medium
	spark-submit index.py /home/ff/cs61c/data/complete-works-mark-twain.txt.seq spark-wc-out-index-medium

index:
	rm -rf spark-wc-out-index
	spark-submit index.py $(myinput) spark-wc-out-index


generate-input: $(TARGET)
	hadoop jar wc.jar Importer $(myinput)

classes:
	mkdir classes

clean:
	rm -rf classes $(TARGET)

destroy-all:
	rm -rf wc-out*
	rm -rf spark-wc-out*
	rm -rf classes $(TARGET)

.PHONY: clean all
