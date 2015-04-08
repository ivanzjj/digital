#ifndef _BUBI_SERIALIZER_H_
#def 	_BUBI_SERIALIZER_H_

#include <vector>

class Serializer{
public:
	typedef std::shared_ptr <Serializer> pointer;
	
	std::vector <unsigned char>& peek_data ();
	static uint256 get_prefix_hash (const unsigned char *ch, int len);
	
	bool add_raw (const unsigned char* ch, int len);
	bool add256 (uint256 &hash);
	uint256 get_sha512_half ();
	
private:
	std::vector <unsigned char> data_;
};

#endif