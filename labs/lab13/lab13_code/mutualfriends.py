from pyspark import SparkContext, SparkConf, StorageLevel
import sys
import os
import re

from pyspark import SparkContext

"""CS61C Summer 2015 Lab 13, MapReduce II: Finding Mutual Friends"""

def flatmapper1(friend_pair):
    """
    In this flatmap fn, you should find a way to fix the fact that 
    for any given friendship, we may only have one edge listed in our graph

    e.g, if 1 and 2 are friends, our graph may contain only the pair (1, 2),
    when it should really contain both (1, 2) and (2, 1).

    The fact that this is called as a flatMap rather than a map is a hint.
    (See the Spark API docs for the difference)
    """

    """ *** YOUR CODE HERE *** """
    pass

def reduce_to_friends_list(a, b):
    """
    At the end of this reduce stage, your code should produce a list of
    friends for each person in the social network graph. For example, if you
    get the input
    
    1, 2
    1, 2
    1, 3
    1, 4
    2, 1
    2, 3
    3, 1
    3, 2
    4, 1

    You should produce the output (the outputs do not need to be in sorted
    order):
    (1, [2, 2, 3, 4])
    (2, [1, 3])
    (3, [1, 2])
    (4, [1])
    """

    """ *** YOUR CODE HERE *** """
    pass

def flatmapper2(d):
    """
    This is the trickiest part to implement. Here, we need to produce keys
    such that we get friends lists for two friends placed into the same 
    final stage (flatmapper3). Remember that the input of this function might
    have duplicate entries in the value. Note that we call groupByKey on the 
    output of this stage, which will do the "shuffle" before the data is 
    passed on to flatmapper3.

    A clean solution will probably require using the sorted and filter 
    functions.

    Here is an example output for this stage, given that 2 is friends with 
    1, 3, and 4:
    [ [(1, 2), [3, 4]], [(2, 3), [1, 4]], [(2, 4), [1, 3]] ]
    """

    """ *** YOUR CODE HERE *** """
    pass

"""You should not modify anything beneath this line, but you will want
to read the code to give you an idea of the general computation."""

def flatmapper3(d):
    """
    At this point, our input key should be a pair of friends - of the form 
    (A, B) - for whom we need to compute a mutual friends list. There should
    be exactly two values that map to any such key - the friends list of A
    and the friends list of B. Considering that we have these two values,
    at this point we simply need to treat the two lists as sets and 
    perform a set intersection to produce the output.

    Thus, the output from this stage should be the final output, for example:
    [(1, 2), [3, 4, 5, 6]]
    if 3, 4, 5, 6 are mutual friends of 1 and 2.

    We have already implemented this for you below:
    """
    if len(d[1]) < 2:
      ls = []
      l1 = set()
      l2 = set()
    else:
      ls = list(d[1])
      l1 = set(ls[0])
      l2 = set(ls[1])

    return [d[0], list(l1.intersection(l2))]

def mutualfriends(file_name, output="spark-mutualfriends-out", onEC2=False):
    if not onEC2: 
        """ setup for local """
        sc = SparkContext("local[8]", "MutualFriends")
        f = sc.sequenceFile(file_name)
    else:
        """ setup for EC2 """
        master = open("/root/spark-ec2/cluster-url").read().strip()
        slaves = sum(1 for line in open("/root/spark-ec2/slaves"))
        conf = SparkConf()
        conf.set("spark.serializer", "org.apache.spark.serializer.KryoSerializer")
        conf.set("spark.eventLog.enabled", "TRUE")
        conf.set("spark.default.parallelism", str(slaves * 2))
        conf.set("spark.akka.frameSize", "50")
        sc = SparkContext(master=master, environment={'PYTHONPATH':os.getcwd()}, conf=conf)
        f = sc.sequenceFile("s3n://cs61cMR2/" + file_name, minSplits=slaves*32)\
                      .persist(StorageLevel.MEMORY_AND_DISK_SER)

    """ The diagram below shows the general flow of the computation we are
    performing. You should not need to change this - you will only need to 
    implement the functions above.

    Input data (pairs representing edges in the input social network graph)
    |-> flatMap
        |-> reduceByKey
            |-> flatMap
                |-> groupByKey
                    |-> flatMap
                        |-> Result

    You can see the spark API docs to read about how these transformations
    operate. The particularly interesting one you may not have seen before
    is groupByKey:

    In our skeleton, the groupByKey allows the flatMap that follows
    to get access to a list of values (all at once) rather than the "streaming" 
    nature of the standard reduceByKey transformation. We can use it in this
    case with little performance penalty because at that point in the 
    computation, we will have at most 2 values mapping to any given key.
    """

    c = f.flatMap(flatmapper1) \
            .reduceByKey(reduce_to_friends_list) \
            .flatMap(flatmapper2) \
            .groupByKey() \
            .flatMap(flatmapper3)

    """ You can ignore this, this simply collapses all of the output you 
    produce into a single file at the end if we're on a local machine,
    or just saves a text file per node if we're on an EC2 cluster. """
    if not onEC2:
        """ If we're not on EC2, make sure we coalesce into one node and then
        save as text file for debugging purposes. """
        c.coalesce(1).saveAsTextFile(output)
    else:
        """ If we're running on EC2, no need to coalesce - in reality we'd 
        probably just distribute the output data onto different nodes anyway 
        for replication/performance. 
        
        As a result of skipping the coalesce, each final-stage reducer node
        will write its own separate output file."""
        c.saveAsTextFile(output)

    # stop the run
    sc.stop()

"""You do not need to understand or read the following code."""
if __name__ == "__main__":
    argv = sys.argv
    input_filename = argv[1]
    output_directory = argv[2]
    on_ec2 = argv[3] == "True"

    mutualfriends(input_filename, output_directory, on_ec2)
