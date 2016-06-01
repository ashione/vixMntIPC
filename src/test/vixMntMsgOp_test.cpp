#include <vixMntMsgOp.h>
#include <iostream>
#include <cstring>
#include <assert.h>

using namespace std;

int main(){
    VixMntMsgOp testop = VixMntMsgOp::MntInit;
    //cout<<(short)testop2<<endl;
    assert(testop == "MntInit");
    assert(!(testop == "MntWrite"));
    assert(!(testop == "MntWrite22"));
    assert(strcmp(getOpValue(testop),"MntInit") == 0);
    assert(getOpIndex("MntInit") == testop);
    assert(getOpIndex("MntWrite") != testop);
    assert(getOpIndex("ffft") != testop);

    testop = VixMntMsgOp::MntRead;
    assert(getOpIndex("MntRead") == testop);

    cout<<"Test successful"<<endl;
    return 0;
}
