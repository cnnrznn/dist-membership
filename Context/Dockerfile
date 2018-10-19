FROM ubuntu:16.04

WORKDIR /home

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y git build-essential make gcc vim net-tools iputils-ping
RUN git clone https://github.com/cnnrznn/dist-membership

WORKDIR /home/dist-membership

RUN make

ADD hostfile .
