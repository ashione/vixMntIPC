#include <vixMntMsgOp.h>
#include <iostream>
#include <assert.h>

using namespace std;

int main(){
    VixMntMsgOp testop = VixMntMsgOp::MntInit;
    //cout<<(short)testop<<endl;
    assert(testop == "MntInit");
    assert(!(testop == "MntWrite"));
    assert(!(testop == "MntWrite22"));
    return 0;
}
