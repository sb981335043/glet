#!/bin/bash

export CUDA_VISIBLE_DEVICES=0,1,2,3
#export CUDA_VISIBLE_DEVICES=0

sudo nvidia-smi -i 0 -c EXCLUSIVE_PROCESS
sudo nvidia-smi -i 1 -c EXCLUSIVE_PROCESS
sudo nvidia-smi -i 2 -c EXCLUSIVE_PROCESS
sudo nvidia-smi -i 3 -c EXCLUSIVE_PROCESS

export CUDA_MPS_PIPE_DIRECTORY=/tmp/nvidia-mps # Select a location that’s accessible to the given $UID

export CUDA_MPS_LOG_DIRECTORY=/tmp/nvidia-log # Select a location that’s accessible to the given $UID


nvidia-cuda-mps-control -d # Start the daemon


echo "start_server -uid $(id -u scc)" |  nvidia-cuda-mps-control # start server manually

