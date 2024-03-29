#ifndef _APP_INSTANCE__
#define _APP_INSTANCE__

#include <string>
#include <deque> 
#include <map>
#include <mutex>
#include "app_spec.h"

using namespace std;

typedef struct _input_data{
    torch::Tensor tensor;
    int id; // this is the ID specified in config.json
} input_data;

class AppInstance{  // class for storing data related to executing an app
    public:
        AppInstance(string name, AppSpec* pAppSpec);
        ~AppInstance();
        string getName();
        void setStart(uint64_t time);
        void setStartExec(uint64_t time);
        void setEndExec(uint64_t time);
        void setTaskID(int id);
        int getTaskID();
        void setSocketFD(int fd);
        int getSocketFD();
        void setupScoreBoard();
        void writeToLog(FILE* fp);       
        void makeInputTensor(torch::Tensor tensor, int input_id);
        unsigned int getSizeofInputs();
        void markFinished(int id);
        bool isFinished();
        int getAppSpecID(); //returns globla ID for AppSpece
        torch::Tensor getOutputTensor(int id);
        AppSpec* getAppSpec();
        bool isDropped();
        void setDropped(bool dropped);
    private:
        string _name;
        int _taskID;
        int _fd;
        uint64_t start;
        uint64_t startExec;
        uint64_t endExec;
        bool _reported; // to prevent race conditions
        bool _dropped; // to distinguish normally finished apps and those that didnt
        map<int, torch::Tensor> _input;
        vector<int> outputCheck;
        map<int,bool> scoreBoard;
        mutex finishMtx;
        mutex inputMtx;
        AppSpec *_pAppSpec; // used for testing whther output is finished or not
};
#endif
