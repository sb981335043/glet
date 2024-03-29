#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

ProxyPartitionWriter::ProxyPartitionWriter(string filename, int nGPU){
    assert(!filename.empty());
    file_to_write = fopen(filename.c_str(), "w");
    assert(nGPU>0);
    _num_of_GPU = nGPU;
}

ProxyPartitionWriter::~ProxyPartitionWriter(){
    VecConfigs.clear();
    fclose(file_to_write);
}

bool ProxyPartitionWriter::addResults(task_config &tconfig){ //returns true, if something is wrong
    if(!( 0 <= tconfig.node_id && tconfig.node_id < _num_of_GPU)){
            printf("[ProxyPartitionWriter] recieved invalid node id: %d \n",tconfig.node_id);
            return true;
    }
    if(!( 0 < tconfig.thread_cap && tconfig.thread_cap <= 100)){
            printf("[ProxyPartitionWriter] recieved invalid thread cap: %d \n",tconfig.thread_cap);
            return true;
    }
    if(tconfig.batch_size ==0 ){
            printf("[ProxyPartitionWriter] recieved invalid batch size: %d \n",tconfig.batch_size);
            return true;
    }
    if(tconfig.duty_cycle <0.0 ){
            printf("[ProxyPartitionWriter] recieved invalid duty cycle: %lf \n",tconfig.duty_cycle);
            return true;
    }

    VecConfigs.push_back(tconfig);
    return false;
}
void ProxyPartitionWriter::writeResults(){
    assert(!VecConfigs.empty());
    for(vector<task_config>::iterator it = VecConfigs.begin(); it != VecConfigs.end(); it ++){
        fprintf(file_to_write,"%d,%d,%d,%s,%d,%lf\n", it->node_id, it->thread_cap, it->dedup_num,it->name.c_str(),it->batch_size,it->duty_cycle);
    }    
    fflush(file_to_write);
}

ProxyPartitionReader::ProxyPartitionReader(string filename, int nGPU){
    assert(!filename.empty());
    assert(nGPU>0);
    _num_of_GPU = nGPU;
    //read from file and fill vectora
    ifstream infile(filename);
    string line;
    string token;
    while(getline(infile,line)){
        istringstream ss(line);
		task_config new_config;
        getline(ss,token,',');
        new_config.node_id = stoi(token);  
        getline(ss,token,',');
		new_config.thread_cap = stoi(token);
        getline(ss,token,',');
		new_config.dedup_num = stoi(token);
        getline(ss,token,',');
        new_config.name = token;
        getline(ss,token,',');
        new_config.batch_size = stoi(token);
        getline(ss,token,',');
        new_config.duty_cycle = stof(token);
		VecConfigs.push_back(new_config);
    }    

}

ProxyPartitionReader::~ProxyPartitionReader(){
    VecConfigs.clear();
}

vector<task_config> ProxyPartitionReader::getTaskConfig(int devid, int dedup_num, int thread_cap){
    assert(0<=devid && devid < _num_of_GPU);
    assert(dedup_num ==0 || dedup_num ==1);
    assert(0<thread_cap && thread_cap <=100 );
    //find requested task_configs
    vector<task_config> found_tasks;
    for(vector<task_config>::iterator it = VecConfigs.begin(); it != VecConfigs.end(); it ++){
        if(it->node_id == devid && it->thread_cap == thread_cap && it->dedup_num == dedup_num)
            found_tasks.push_back(*it);
    }
    return found_tasks;
}

vector<task_config> ProxyPartitionReader::getAllTaskConfig(){
    return VecConfigs;
}
