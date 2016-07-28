#include <assert.h>
#include <cstring>
#include <iostream>
#include <vixMntMsgOp.h>

using namespace std;

int main() {
#if defined(__cplusplus) && __cplusplus >= 201103L
   VixMntMsgOp testop = VixMntMsgOp::MntInit;
#else
   VixMntMsgOp testop = MntInit;
#endif
   // cout<<(short)testop2<<endl;
   assert(testop == "MntInit");
   assert(!(testop == "MntWrite"));
   assert(!(testop == "MntWrite22"));
   assert(strcmp(getOpValue(testop), "MntInit") == 0);
   assert(getOpIndex("MntInit") == testop);
   assert(getOpIndex("MntWrite") != testop);
   assert(getOpIndex("ffft") != testop);

#if defined(__cplusplus) && __cplusplus >= 201103L
   testop = VixMntMsgOp::MntRead;
#else
   testop = MntRead;
#endif
   assert(getOpIndex("MntRead") == testop);

   cout << "Test successful" << endl;
   return 0;
}
