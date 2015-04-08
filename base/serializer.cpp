#include "serializer.h"

#include <openssl/sha.h>

namespace Bubi{

std::vector <unsigned char>&
Serializer::peek_data (){
	return data_;
}

uint256
Serializer::get_prefix_hash (const unsigned char* ch, int len){
	uint256 j[2];
	SHA512_CTX  ctx;
	SHA512_Init (&ctx);
	SHA512_Update (&ctx, ch, len);
	SHA512_Final (reinterpret_cast <unsigned char *> (&j[0]), &ctx);
	return j[0];
}

bool
Serializer::add_raw (const unsigned char *ch, int len){
	for (int i=0; i<len; i++){
		data_.push_back (ch[i]);
	}
	return true;
}

bool
Serializer::add256 (uint256 &hash){
	data_.insert (data_.end(), hash.begin(),hash.end());
	return true;
}

uint256
Serializer::get_sha512_half (){
	uint256 j[2];
	SHA512 ( &(data_.front()), data_.size(), reinterpret_cast <unsigned char *> (&j[0]) );
	return j[0];
}

}