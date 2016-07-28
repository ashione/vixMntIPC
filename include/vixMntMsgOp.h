#ifndef VIXMNT_MSG_OP_H
#define VIXMNT_MSG_OP_H

#ifdef __cplusplus

#include <string>
extern "C" {

#endif

#define OP_MODE_NUM 7
#define OP_DATA_MAX_SIZE 0x400
#define OP_RESPONSE_Q_SIZE 0x40

#if defined(__cplusplus) && __cplusplus >= 201103L
enum class VixMntMsgOp : short {
   MntInit = 0,
   MntInitDone,
   MntWrite,
   MntWriteDone,
   MntRead,
   MntReadDone,
   HALT,
   ERROR,
};
#else
enum VixMntMsgOp {
   MntInit = 0,
   MntInitDone,
   MntWrite,
   MntWriteDone,
   MntRead,
   MntReadDone,
   HALT,
   ERROR,
};
#endif

class VixMntMsgData {
public:
   VixMntMsgOp msg_op;
   size_t msg_datasize;
   char msg_buff[OP_DATA_MAX_SIZE];
   char msg_response_q[OP_RESPONSE_Q_SIZE];

public:
   VixMntMsgData() {
#if defined(__cplusplus) && __cplusplus >= 201103L
      this->msg_op = VixMntMsgOp::ERROR;
#else
      this->msg_op = ERROR;
#endif
   }

   VixMntMsgData(VixMntMsgOp, size_t, char *);
   VixMntMsgData(VixMntMsgOp, char *);
   VixMntMsgData(VixMntMsgOp, size_t, const char *, char *);
};

const char *getOpValue(VixMntMsgOp op);
VixMntMsgOp getOpIndex(const char *str_op);
bool operator==(VixMntMsgOp op, const char *cstr_op);

#if defined(__cplusplus) && __cplusplus >= 201103L
#define VixMntOp(op) getOpIndex(#op)
#else
#define VixMntOp(op) (op)
#endif

#ifdef __cplusplus
}

bool operator==(VixMntMsgOp op, std::string str_op);

#endif

#endif // end vixMNT_MSG_OP_H
