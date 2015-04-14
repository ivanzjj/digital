#ifndef _BUBI_BASE_UINT_H_
#define _BUBI_BASE_UINT_H_

namespace Bubi{

template <std::size_t bits>
class base_uint {
	static_assert ((bits % 32) == 0,
			"The length of the base_uint must be a multiple of 32.");
	typedef unsigned char*	pointer;

public:
	pointer begin (){
		return reinterpret_cast <unsigned char *>(data);
	}
	pointer end (){
		pointer p_begin = reinterpret_cast <unsigned char *>(data);
		return p_begin + WIDTH * 4;
	}
	base_uint (){
		bytes = bits >> 3;
	}
	int get_bytes (){
		return bytes;
	}
	void zero (){
		for (int i = 0; i < bytes; i++){
			*(begin() + i) = 0;
		}		
	}

private:
	enum {WIDTH = bits / 32};
	unsigned int data[WIDTH];
	int bytes;
};

template <std::size_t Bits>
inline bool operator == (base_uint<Bits> &a, base_uint<Bits> &b){
	if (a.get_bytes () != b.get_bytes ()){
		return false;
	}
	int bytes = a.get_bytes ();
	for (int i = 0; i < bytes; i++){
		if (*(a.begin() + i) != *(b.begin() + i)){
			return false;
		}
	}
	return true;
}

template <std::size_t Bits>
inline bool operator != (base_uint<Bits> &a, base_uint<Bits> &b){
	return !(a == b);
}

}

#endif
