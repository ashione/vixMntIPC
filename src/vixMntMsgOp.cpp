#include <vixMntMsgOp.h>
#include <vixMntUtility.h>

#include <string.h>
#include <memory>

const char*
VIXMNT_MSG_OP_STR[OP_MODE_NUM] = {
    "MntInit",
    "MntInitDone",
    "MntWrite",
    "MntWriteDone",
    "MntRead",
    "MntReadDone",
    "HALT"
};

bool
operator== (VixMntMsgOp op, const char* cstr_op){

#if defined(__cplusplus) && __cplusplus >= 201103L
    if(op != VixMntMsgOp::ERROR)
#else
    if( op != ERROR )
#endif
        return !strcmp(cstr_op,getOpValue(op));
    return false;
}

bool
operator== (VixMntMsgOp op, std::string str_op) {
    return operator==(op, str_op.c_str());
}

const char*
getOpValue(VixMntMsgOp op){

#if defined(__cplusplus) && __cplusplus >= 201103L
    if( op == VixMntMsgOp::ERROR )
#else
    if( op == ERROR )
#endif
        return NULL;
    return VIXMNT_MSG_OP_STR[(short)op];
}

VixMntMsgOp
getOpIndex(const char* str_op){

    VixMntMsgOp test_op[OP_MODE_NUM] = {
#if defined(__cplusplus) && __cplusplus >= 201103L
        VixMntMsgOp::MntInit,
        VixMntMsgOp::MntInitDone,
        VixMntMsgOp::MntWrite,
        VixMntMsgOp::MntWriteDone,
        VixMntMsgOp::MntRead,
        VixMntMsgOp::MntReadDone,
        VixMntMsgOp::HALT
#else
        MntInit,
        MntInitDone,
        MntWrite,
        MntWriteDone,
        MntRead,
        MntReadDone,
        HALT
#endif

    };


    for(int i = 0 ; i < OP_MODE_NUM ; ++i){
        if(test_op[i] == str_op)
            return test_op[i];
    }

#if defined(__cplusplus) && __cplusplus >= 201103L
    return VixMntMsgOp::ERROR;
#else
    return ERROR;
#endif

}

VixMntMsgData::VixMntMsgData(
        VixMntMsgOp msg_op,
        size_t msg_datasize,
        char* msg_buff)
{
   this->msg_op = msg_op;
   this->msg_datasize = msg_datasize;
   memcpy(this->msg_buff,msg_buff,this->msg_datasize);
}

// using this only when msg_buff is string
VixMntMsgData::VixMntMsgData(
        VixMntMsgOp msg_op,
        char* msg_buff)
{
    size_t msg_len = strlen(msg_buff);
    new(this) VixMntMsgData(msg_op,msg_len,msg_buff);
}

VixMntMsgData::VixMntMsgData(
        VixMntMsgOp msg_op,
        size_t msg_datasize,
        const char* msg_q_name,
        char* msg_buff)
{
    strncpy(msg_response_q,msg_q_name,strlen(msg_q_name));
    msg_response_q [  strlen(msg_q_name) ] = '\0';
    new(this) VixMntMsgData(msg_op,msg_datasize,msg_buff);
}
