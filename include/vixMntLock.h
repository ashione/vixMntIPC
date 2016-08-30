#ifndef VIXMNT_MUTEX
#define VIXMNT_MUTEX

#ifdef __cplusplus
extern "C" {
#endif

class VixMntLock {
public:
   virtual void lock() = 0;
   virtual void unlock() = 0;
   virtual bool trylock() = 0;
};

class VixMntMutex : public VixMntLock {
private:
   void *_handle;

public:
   VixMntMutex(void *shmMemMutex, bool recursive = false);
   virtual ~VixMntMutex();

   void lock();
   void unlock();
   bool trylock();
};

#ifdef __cplusplus
}
#endif
#endif // VIXMNT_MUTEX
