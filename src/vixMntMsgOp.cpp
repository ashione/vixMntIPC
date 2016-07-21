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
operator== (
   VixMntMsgOp op,
   const char* cstr_op) {

#if defined(__cplusplus) && __cplusplus >= 201103L
   if(op != VixMntMsgOp::ERROR)
#else
   if( op != ERROR )
#endif
     return !strcmp(cstr_op,getOpValue(op));
   return false;
}

bool
operator== (
   VixMntMsgOp op,
   std::string str_op) {
   return operator==(op, str_op.c_str());
}

/*
 ****************************************************************************
 * getOpValue
 * convert enum type to string info
 * -------------------------------------------------------------------------
 * input parameters  :
 * op
 * -------------------------------------------------------------------------
 * output paremeters :
 * const char*
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

const char*
getOpValue(VixMntMsgOp op){

#if defined(__cplusplus) && __cplusplus >= 201103L
   if ( op == VixMntMsgOp::ERROR )
#else
   if ( op == ERROR )
#endif
     return NULL;
   return VIXMNT_MSG_OP_STR[(short)op];
}

/*
 ****************************************************************************
 * getOpIndex
 * convert operation string to VixMntMsgOp
 * -------------------------------------------------------------------------
 * input parameters  :
 * str_op
 * -------------------------------------------------------------------------
 * output paremeters :
 * VixMntMsgOp
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

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


   for (int i = 0 ; i < OP_MODE_NUM ; ++i) {
     if (test_op[i] == str_op)
       return test_op[i];
   }

#if defined(__cplusplus) && __cplusplus >= 201103L
   return VixMntMsgOp::ERROR;
#else
   return ERROR;
#endif

}

/*
 ****************************************************************************
 * VixMntMsgData Constructor
 * create a new VixMntMsgData that only ship VixMntMsgOp
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_op
 * msg_datasize
 * msg_buff
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMsgData::VixMntMsgData(
    VixMntMsgOp msg_op,         // IN
    size_t msg_datasize,        // IN
    char* msg_buff )            // IN
{
   this->msg_op = msg_op;
   this->msg_datasize = msg_datasize;
   memcpy(this->msg_buff,msg_buff,this->msg_datasize);
}

/*
 ****************************************************************************
 * VixMntMsgData Constructor
 * using this only when msg_buff is string
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_op
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMsgData::VixMntMsgData(
    VixMntMsgOp msg_op,         // IN
    char* msg_buff)             // IN
{
   size_t msg_len = strlen(msg_buff);
   new (this) VixMntMsgData(msg_op,msg_len,msg_buff);
}

/*
 ****************************************************************************
 * VixMntMsgData Constructor
 * add exra msg_response_q that only work in message queue
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_op
 * msg_datasize
 * msg_buff
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMsgData::VixMntMsgData(
    VixMntMsgOp msg_op,
    size_t msg_datasize,
    const char* msg_q_name,
    char* msg_buff)
{
   strncpy(msg_response_q,msg_q_name,strlen(msg_q_name));
   msg_response_q [  strlen(msg_q_name) ] = '\0';
   new (this) VixMntMsgData(msg_op,msg_datasize,msg_buff);
}
