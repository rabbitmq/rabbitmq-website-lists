#!/bin/bash

NUM_QS=100

export RABBITMQ_NODE_PORT=5671
export RABBITMQ_NODENAME="rabbit1@localhost"

for n in `seq -w 1 ${NUM_QS}`; do
    Q="testq_$n"
    ./populate $Q &
done
echo "Waiting for queues to be populated..."
wait