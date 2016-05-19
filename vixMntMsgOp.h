#ifndef VIXMNT_MSG_OP_H
#define VIXMNT_MSG_OP_H

#include <string>
#include <cstring>
#include <memory>

#define OP_MODE_NUM 6
#define OP_DATA_MAX_SIZE 0x400
enum
class VixMntMsgOp : short {
    MntInit = 0,
    MntInitDone,
    MntWrite,
    MntWriteDone,
    MntRead,
    MntReadDone,
    ERROR,
};

class VixMntMsgData {
    public :
        VixMntMsgOp msg_op;
        size_t msg_datasize;
        char msg_buff[OP_DATA_MAX_SIZE];

    public :
        VixMntMsgData(){
            this->msg_op = VixMntMsgOp::ERROR;
        }

        VixMntMsgData(VixMntMsgOp, size_t, char*);
        VixMntMsgData(VixMntMsgOp, char*);
};


const char* getOpValue(VixMntMsgOp op);
VixMntMsgOp getOpIndex(const char* str_op);
bool operator== (VixMntMsgOp op, const char* cstr_op);
bool operator== (VixMntMsgOp op, std::string str_op);



#endif
