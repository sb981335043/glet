#include "thread.h"

extern SysMonitor ServerState; 
pthread_t initRequestThread(int sock){
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024 * 1024); // set memory size, may need to adjust in the future, for now set it to 1MB
    pthread_t tid;
    if (pthread_create(&tid, &attr, handleRequest, (void *)(intptr_t)sock) != 0)
        LOG(ERROR) << "Failed to create a request handler thread.\n";
    return tid;

}

int recvTensor(int SockNum, shared_ptr<AppInstance> tmp, TensorSpec* pSpec, bool isLast, bool isFirst){
        std::vector<int64_t> dims={1};
        bool debug=false;
#ifdef DEBUG
        printf("[recvTensor] start recieving tensor, isfirst: %d, is last: %d \n", isFirst, isLast);
#endif   
        at::Tensor input;
#ifndef NO_NET

        int Datalen = 1;
        for(vector<int>::iterator it = pSpec->dim.begin(); it != pSpec->dim.end(); it++){
            dims.push_back(*it);
            Datalen *= *it;
        }
        int rcvd;
        if (pSpec->dataType==KFLOAT32){
                float *inData = (float *)malloc(Datalen * sizeof(float));
#ifdef DEBUG 
                cout << "Input TYPE: KFLOAT32" << endl;
                uint64_t start,end;
                start = getCurNs();
                debug = true;
#endif 
                rcvd = SOCKET_receive(SockNum, (char *)inData,Datalen * sizeof(float) ,debug);
                if (rcvd <= 0) {
                        cout << "CLIENT CLOSED1, received: "<< rcvd << endl;
                        return 1;  // Client closed the socket    
                }

                auto options = torch::TensorOptions().dtype(torch::kFloat32).requires_grad(false);
                input= convert_rawdata_to_tensor(inData, dims, options);
#ifdef DEBUG 
                end = getCurNs();
                printf("[recvTensor ]Receiving %d bytes took %lf ms \n", rcvd, double(end-start)/1000000);
#endif 

        }
        else if(pSpec->dataType == KINT64){

                int64_t *inData = (int64_t *)malloc(Datalen * sizeof(int64_t));
#ifdef DEBUG 
                cout << "Input TYPE: KINT64" << endl;
                uint64_t start,end;
                start = getCurNs();
                debug = true;
#endif 
                rcvd = SOCKET_receive(SockNum, (char *)inData,Datalen * sizeof(int64_t) ,debug);
                if (rcvd == 0) {
                        return 1;  // Client closed the socket    
                }
 
#ifdef DEBUG 
                end = getCurNs();
                printf("[recvTensor ] expecting %d bytes \n", Datalen * sizeof(int64_t));
                printf("[recvTensor ] Receiving %d bytes took %lf ms \n", rcvd, double(end-start)/1000000);
#endif 
                auto options = torch::TensorOptions().dtype(torch::kInt64).requires_grad(false);
                input= convert_rawdata_to_tensor(inData, dims, options);
        }

#else
// create a dummy tensor
        input = torch::ones(1);
#endif // NO_NET

        if(isFirst) tmp->setStart(getCurNs());

        int tid = SOCKET_rxsize(SockNum);
        if (tid <= 0) {
                cout << "CLIENT CLOSED2, received: "<< tid << " as tid"<< endl;
                return 1;  // Client closed the socket    
        }

#ifdef DROP_DEBUG
        cout <<"received tid: " << tid << endl;
#endif 
        tmp->setTaskID(tid);
        vector<int> dsts = tmp->getAppSpec()->getNextDsts(pSpec->id);
        for(vector<int>::iterator it2 = dsts.begin(); it2 != dsts.end(); it2++){
                string name = tmp->getAppSpec()->getModelName(*it2);
                addtoModelQueue( name, input,*it2,tmp);
        }
        if(isLast) tmp->setStartExec(getCurNs());
    return 0;
}


void* handleRequest(void* sock){
    int SockNum = (intptr_t)sock;
    char CharName[MAX_REQ_SIZE]; 
    //receive request 
    SOCKET_receive(SockNum, (char*)&CharName, MAX_REQ_SIZE, false);
    int numInput = SOCKET_rxsize(SockNum);
    #ifdef FRONTEND_DEBUG
    cout << CharName << "will receive " << numInput << " inputs" << endl;
    #endif

    string AppName(CharName);
    //need to check valid app name
    bool found =false;
    AppSpec *pSpec;
    for(unsigned int i = 0; i< ServerState.AppSpecVec.size(); i++){
       if(ServerState.AppSpecVec[i].getName() == AppName){
               found=true;
               pSpec = &(ServerState.AppSpecVec[i]);
       }
    }
    if(!found){
        printf("app : %s not found \n", CharName);
        return (void*)1;
    }
    else
        LOG(ERROR) << "App " << CharName << " forward pass.";

#ifdef DEBUG
    printf("[handleRequest] %s will recieve %d inputs \n", CharName, numInput);
#endif 
    vector<TensorSpec> InputSpecVec = pSpec->getInputTensorSpecs();
    uint64_t last_timestamp=0; // used for updating intervals
    while (1) {
        shared_ptr<AppInstance> tmp = make_shared<AppInstance>(AppName, pSpec);
        tmp->setSocketFD(SockNum);
        bool isLast=false;
        bool isFirst=true;
        for(unsigned int i  = 0; i< InputSpecVec.size(); i++){
            if(i == InputSpecVec.size()-1 ) isLast=true;
            if (recvTensor(SockNum, tmp, &InputSpecVec[i], isLast, isFirst)) {
                printf("Error in receiving tensor for %s Or Client closed\n", CharName);
                //ServerState.SYS_FLUSH=true;
                return NULL;
            }
            if(isFirst) isFirst=false; // set it to false since first input was recieved
        }
         tmp->setStartExec(getCurNs());  
    }
    close(SockNum);
    return NULL;
}  

