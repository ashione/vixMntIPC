#include <vixMntException.h>
#include <vixMntLock.h>
#include <vixMntUtility.h>

#include <pthread.h>

int main() {
   pthread_mutex_t mt;
   VixMntMutex lock(&mt, true);
   try {
      lock.lock();

      lock.lock();
      lock.unlock();

      lock.unlock();
   } catch (VixMntException &e) {
      ILog("%s", e.what());
   }

   return 0;
}
