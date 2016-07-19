#ifndef VIXMNT_EXCEPTION
#define VIXMNT_EXCEPTION

#include <exception>
#ifdef __cplusplus
extern "C" {
#endif

class VixMntException : std::exception
{
   private :
     const char* _info;

   public :
     VixMntException(const char* info){
       _info = info;
     }

     virtual const char* what() const throw(){
        return _info;
     }


};

#ifdef __cplusplus
}
#endif

#endif // VIXMNT_EXCEPTION
