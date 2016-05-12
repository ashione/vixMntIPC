#include <vixMntMsgOp.h>

bool
operator== (VixMntMsgOp& op, const char* cstr_op){

    if(op != VixMntMsgOp::ERROR)
        return !strcmp(cstr_op,getOpValue(op));
    return false;
}

bool
operator== (VixMntMsgOp& op, std::string str_op) {
    return operator==(op, str_op.c_str());
}

inline const char*
getOpValue(VixMntMsgOp& op){
    return VIXMNT_MSG_OP_STR[(short)op];
}

inline VixMntMsgOp
getOpIndex(const char* str_op){

    VixMntMsgOp test_op[OP_MODE_NUM] = { VixMntMsgOp::MntInit, VixMntMsgOp::MntWrite, VixMntMsgOp::MntRead };

    for(int i = 0 ; i < OP_MODE_NUM ; ++i){
        if(test_op[i] == str_op)
            return test_op[i];
    }
    //if( VixMntMsgOp::MntInit  == str_op)
    //    return VixMntMsgOp::MntInit;

    return VixMntMsgOp::ERROR;

}

