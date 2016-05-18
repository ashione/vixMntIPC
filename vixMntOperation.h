#ifndef VIXMNT_OPEARTION_H
#define VIXMNT_OPEARTION_H

struct VixMntOpRead{
    const char* fileName;
    char* buf;
    size_t buf_size;
    off_t offsize;
};
#endif // VIXMNT_OPERATION_H
