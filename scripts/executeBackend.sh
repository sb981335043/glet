#!/bin/bash


if [ -z $1 ]
then
    echo "no number of GPU was specified! default 1 will be used"
    NGPUS=1
  else
    NGPUS=$1
fi

MAX_NUM_OF_PROXYS=7 # 20,40,50(x2),60,80,100

RES_DIR=../resource/


BIN=../bin/backend


proxy_config=$RES_DIR/proxy_config.json
exp_config=$RES_DIR/config.json

echo $proxy_config

first_idx=0
last_idx=$(($NGPUS-1))

$BIN --backend_control_portno 8081 --backend_data_portno 8082 \
        --gpu_idxs $first_idx-$last_idx --ngpus $NGPUS --nproxy $MAX_NUM_OF_PROXYS\
        --config_json $exp_config --proxy_json $proxy_config \
	--resource_dir $RES_DIR \
  --full_proxy_dir /home/scc/glet/resource/proxy


