#include "transaction.pb.h"
#include <iostream>
#include <string>

#include "transaction.h"

namespace Bubi {

Transaction::Transaction (){}

Transaction::Transaction (uint256 _source_address_,
						uint256   _destination_address_,
						double	   _payment_amount_,
						uint256   _last_ledger_hash_)
	:source_address_ (_source_address_)
	 ,destination_address_ (_destination_address_)
	 ,payment_amount_ (_payment_amount_)
	 ,last_ledger_hash_ (_last_ledger_hash_)
{
}

Transaction::~Transaction (){}

std::string
Transaction::serializer (){
	bubi::Transaction tx;
	tx.set_source_address (source_address_.to_string());
	tx.set_destination_address (destination_address_.to_string());
	tx.set_payment_amount (payment_amount_);
	tx.set_last_ledger_hash (last_ledger_hash_.to_string());
	std::string out;
	if (!tx.SerializeToString (&out)){
		std::cerr << "Transactionserializer is error!" << std::endl;
	}
	return out;
}

void
Transaction::unserializer (std::string str){
	bubi::Transaction tx;
	if(!tx.ParseFromString (str)){
	 std::cerr <<"Transactionunserializer is erro!" << std::endl;
	}
	source_address_.binary_init (tx.source_address ().c_str());
	destination_address_.binary_init (tx.destination_address ().c_str());
	payment_amount_ = tx.payment_amount ();
	last_ledger_hash_.binary_init (tx.last_ledger_hash ().c_str ());
}

uint256&
Transaction::get_source_address (){
	return source_address_;
}

uint256&
Transaction::get_destination_address (){
	return destination_address_;
}

double 
Transaction::get_payment_amount (){
	return payment_amount_;
}

uint256&
Transaction::get_last_ledger_hash (){
	return last_ledger_hash_;
}

}
