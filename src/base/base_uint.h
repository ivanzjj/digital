#ifndef _BUBI_BASE_UINT_H_
#define _BUBI_BASE_UINT_H_

#include <string>
#include <iostream>
#include <sstream>

namespace Bubi{

template <std::size_t bits>
class base_uint {
	static_assert ((bits % 32) == 0,
			"The length of the base_uint must be a multiple of 32.");
	typedef char*	pointer;

public:
	pointer begin (){
		return data;
	}

	pointer end (){
		return data + WIDTH;
	}

	int get_bytes (){
		return WIDTH;
	}

	void zero (){
		for (int i = 0; i < WIDTH; i++){
			data[i] = 0;
		}		
	}
	void init (const char *ch){
		for (int i = 0; i < WIDTH; i++){
			data[i] = ch[i];
		}
	}
	std::string to_string (){
		std::ostringstream out;
		for (int i = 0; i < WIDTH; i++){
			out << (char)(data[i]);
		}
		std::string ret = out.str ();
		return ret;
		return out.str ();
	}

private:
	enum {WIDTH = bits / 8};
	char data[WIDTH];
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
