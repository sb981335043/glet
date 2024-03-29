#ifndef __BREQUEST
#define __BREQUEST
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <torch/script.h>
#include <torch/serialize/tensor.h> 
#include <torch/serialize.h>
#include "request.h"
#include "common_utils.h"

using namespace std;

class batched_request{
private:
    unsigned int mBatchNum; // batch size
    int mGPUID;// gpuid to execute
    char _req_name[MAX_REQ_SIZE];
    string _StrReqName;
  
    unsigned int _max_batch; //maximum number of tasks that can be  batched 
    int mBatchID; // used for debugging
    torch::Tensor batched_tensor;
    vector<shared_ptr<request>> taskRequests;

public:
    batched_request(); //makes empty batched_request
    ~batched_request();
    torch::Tensor getBatchedTensor();
    vector<shared_ptr<request>>& getRequests();
    void setBatchID(int id);
    int getBatchID();
    char* getReqName();
    void setStrName(string name);
    string getStrName();
    void setReqName(const char *net_name);
    unsigned int getBatchNum();
    int getGPUID();
    void setGPUID(int gid);
    void setMaxBatch(unsigned int size);
    unsigned int getMaxBatch();
    int getNTask();
    void addRequestInfo(shared_ptr<request> &t);
    void allocateOutput(torch::Tensor tensor);

};

#endif 
