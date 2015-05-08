#include "base_uint.h"

namespace Bubi {

int hex_to_decimal (char a){
	if (a <= '9' && a >= '0')	return a - '0';
	else if (a <= 'f' && a >= 'a')	return a - 'a' + 10;
	else if (a <= 'F' && a >= 'A')	return a - 'A' + 10;
}

}
