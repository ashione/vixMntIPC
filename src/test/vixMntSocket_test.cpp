#include <vixMntSocket.h>
#include <stdio.h>

int main(){
    if(fork() == 0){

        VixMntSocketServer* vser = new VixMntSocketServer();
        vser->serverListen();

        delete vser;

    }
    else{
        sleep(1);
        VixMntSocketClient* vclt = new VixMntSocketClient();
        char buf[0xff];
        char result[0xff];
        while(scanf("%s",buf)!=EOF){

            vclt->rawWrite(buf,strlen(buf));
            vclt->rawRead(result,0xff);
            ILog(" msg : %s ",result);
            memset(result,0,0xff);
        }
        delete vclt;
    }
    return 0;
}
