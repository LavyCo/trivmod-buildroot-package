#ifndef MYMODULE_H
#define MYMODULE_H

#include <linux/ioctl.h>

struct mystats {
    unsigned int open_count;
    unsigned int release_count;
    unsigned int active_count;
};
#define MYTYPE 'm'

#define MY_STATSGET _IOR(MYTYPE, 1, struct mystats)
#define MY_STATSCLR _IO(MYTYPE, 2)

#endif