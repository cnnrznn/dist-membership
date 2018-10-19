#!/bin/bash

N=$1

sudo docker run \
	--net mynet \
	--ip "172.18.0.1$N" \
        -a stdout -a stderr -a stdin \
	p2 \
	./prj2 -h hostfile -p 10000 -i $N
