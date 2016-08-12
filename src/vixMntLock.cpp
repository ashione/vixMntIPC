#include <vixMntException.h>
#include <vixMntLock.h>

#include <cstdlib>
#include <errno.h>
#include <pthread.h>
#include <string.h>

/**
 * @brief  lock initilaztion
 *
 * @param shmMemMutex [in] global share memory mutex
 * @param recursive [in]
 */

VixMntMutex::VixMntMutex(void *shmMemMutex, bool recursive)
{
   _handle = shmMemMutex;
   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
   pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE_NP
                                              : PTHREAD_MUTEX_FAST_NP);

   if (pthread_mutex_init((pthread_mutex_t *)_handle, &attr)) {
      throw VixMntException("Unable to create mutex");
   }
}

/**
 * @brief  destory lock
 */

VixMntMutex::~VixMntMutex()
{
   pthread_mutex_destroy((pthread_mutex_t *)_handle);
}

/**
 * @brief lock
 */

void
VixMntMutex::lock()
{
   if (pthread_mutex_lock((pthread_mutex_t *)_handle)) {
      throw VixMntException("Unable to lock mutex");
   }
}

/**
 * @brief unlock
 */
void
VixMntMutex::unlock()
{
   int result = (pthread_mutex_unlock((pthread_mutex_t *)_handle));
   if (result) {
      throw VixMntException("Unable to unlock mutex");
      // throw VixMntException(strerror(result));
   }
}

/**
 * @brief trylock
 *
 * @return
 */

bool
VixMntMutex::trylock()
{
   int tryResult = pthread_mutex_trylock((pthread_mutex_t *)_handle);
   if (tryResult) {

      if (EBUSY == tryResult)
         return false;
      throw VixMntException("Unable to lock mutex");
   }
   return true;
}
