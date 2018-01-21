import sys
import re

from pyspark import SparkContext

def flat_map(document):
    """Returns back all of the words in LINE """
    return re.findall(r"\w+", document[1])

def map(word):
    """ Create pairs where the word is the key and 1 is the value """
    return (word, 1)

def reduce(a, b):
    """ Add up the counts for a word """
    return a + b

def wordcount(file_name, output="spark-wc-out-wordcount"):
    sc = SparkContext("local[8]", "WordCount")
    """ Reads in a sequence file FILE_NAME to be manipulated """
    file = sc.sequenceFile(file_name)

    """
    - flatMap takes in a function that will take one input and outputs 0 or more
      items
    - map takes in a function that will take one input and outputs a single item
    - reduceByKey takes in a function, groups the dataset by keys and aggregates
      the values of each key
    """
    counts = file.flatMap(flat_map) \
                 .map(map) \
                 .reduceByKey(reduce)

    """ Takes the dataset stored in counts and writes everything out to OUTPUT """
    counts.coalesce(1).saveAsTextFile(output)

""" Do not worry about this """
if __name__ == "__main__":
    argv = sys.argv
    if len(argv) == 2:
        wordcount(argv[1])
    else:
        wordcount(argv[1], argv[2])
