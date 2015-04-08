#ifndef _BUBI_UTILS_H_
#define _BUBI_UTILS_H_

namespace Bubi{
	typedef std::make_pair	MP;
	typedef unsigned int	uint;

#define LOG(log)\
	printf("[INFO %s:%d] %s\n",__FUNCTION__,__LINE__,log)
}

#endif
