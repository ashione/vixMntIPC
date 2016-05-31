#include <vixMntMsgOp.h>
#include <cstring>
#include <string>
#include <memory>

const char*
VIXMNT_MSG_OP_STR[OP_MODE_NUM] = {
    "MntInit",
    "MntInitDone",
    "MntWrite",
    "MntWriteDone",
    "MntRead",
    "MntReadDone",
};

bool
operator== (VixMntMsgOp op, const char* cstr_op){

    if(op != VixMntMsgOp::ERROR)
        return !strcmp(cstr_op,getOpValue(op));
    return false;
}

bool
operator== (VixMntMsgOp op, std::string str_op) {
    return operator==(op, str_op.c_str());
}

const char*
getOpValue(VixMntMsgOp op){

    if( op == VixMntMsgOp::ERROR )
        return NULL;
    return VIXMNT_MSG_OP_STR[(short)op];
}

VixMntMsgOp
getOpIndex(const char* str_op){

    VixMntMsgOp test_op[OP_MODE_NUM] = {
        VixMntMsgOp::MntInit,
        VixMntMsgOp::MntInitDone,
        VixMntMsgOp::MntWrite,
        VixMntMsgOp::MntWriteDone,
        VixMntMsgOp::MntRead,
        VixMntMsgOp::MntReadDone
    };

    for(int i = 0 ; i < OP_MODE_NUM ; ++i){
        if(test_op[i] == str_op)
            return test_op[i];
    }
    //if( VixMntMsgOp::MntInit  == str_op)
    //    return VixMntMsgOp::MntInit;

    return VixMntMsgOp::ERROR;

}

VixMntMsgData::VixMntMsgData(VixMntMsgOp msg_op,
        size_t msg_datasize,
        char* msg_buff)
{
   this->msg_op = msg_op;
   this->msg_datasize = msg_datasize;
   memcpy(this->msg_buff,msg_buff,this->msg_datasize);
}

// using this only when msg_buff is string
VixMntMsgData::VixMntMsgData(VixMntMsgOp msg_op,
        char* msg_buff)
{
    size_t msg_len = strlen(msg_buff);
    VixMntMsgData(msg_op,msg_len,msg_buff);
}
