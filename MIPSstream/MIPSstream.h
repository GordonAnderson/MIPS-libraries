#ifndef MIPSstream_h
#define MIPSstream_h

#include <inttypes.h>
#include "Stream.h"
#include "Print.h"

class MIPSstream : public Serial_
{
   public:
//     using Print::println;
     size_t printlnt(void);
     MIPSstream() {}
};

#endif
