#ifndef _BUBI_UTILS_H_
#define _BUBI_UTILS_H_

#include "base_uint.h"

namespace Bubi{
	#define MP	std::make_pair
	typedef unsigned int	uint;
	typedef unsigned char	uchar;
	typedef base_uint<256>	uint256;

#define BUBI_LOG(log)\
	printf("[INFO %s:%d] %s\n",__FUNCTION__,__LINE__,log)
}

#endif
