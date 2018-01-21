root_dir := /root
spark_dir := $(root_dir)/spark/bin
ec2_env := $(root_dir)/ec2-environment.sh

all: run-local

run-local: clean
	spark-submit mutualfriends.py /home/ff/cs61c/data/mrlab2/small.seq spark-mutualfriends-out False

run-ec2-small:
	source $(ec2_env) && \
		PYTHONWARNINGS="ignore" $(spark_dir)/spark-submit --name smallmfriends mutualfriends.py small.seq spark-mutualfriends-out-ec2-small True

run-ec2-large:
	source $(ec2_env) && \
		PYTHONWARNINGS="ignore" $(spark_dir)/spark-submit --name largemfriends mutualfriends.py large.seq spark-mutualfriends-out-ec2-large True

clean:
	rm -rf spark-mutualfriends-out

.PHONY: clean
