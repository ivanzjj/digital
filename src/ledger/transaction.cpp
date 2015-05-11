#include "transaction.pb.h"
#include <iostream>
#include <string>

#include "transaction.h"

namespace Bubi {

Transaction::Transaction (){}

Transaction::Transaction (uint256	s_address,
						uint256		d_address,
						double		amount)
	:source_address_ (s_address)
	 ,destination_address_ (d_address)
	 ,payment_amount_ (amount)
{
}

Transaction::~Transaction (){}

std::string
Transaction::serializer (){
	bubi::Transaction tx;
	tx.set_source_address (source_address_.to_string());
	tx.set_destination_address (destination_address_.to_string());
	tx.set_payment_amount (payment_amount_);
	tx.set_source_previous_ledger_seq (source_previous_ledger_seq_);
	tx.set_source_previous_tx_hash (source_previous_tx_hash_.to_string ());
	tx.set_destination_previous_ledger_seq (destination_previous_ledger_seq_);
	tx.set_destination_previous_tx_hash (destination_previous_tx_hash_.to_string ());
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
	source_previous_ledger_seq_ = tx.source_previous_ledger_seq ();
	source_previous_tx_hash_.binary_init (tx.source_previous_tx_hash ().c_str ());
	destination_previous_ledger_seq_ = tx.destination_previous_ledger_seq ();
	destination_previous_tx_hash_.binary_init (tx.destination_previous_tx_hash ().c_str ());
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

std::uint32_t&
Transaction::get_source_previous_ledger_seq (){
	return source_previous_ledger_seq_;
}

uint256&
Transaction::get_source_previous_tx_hash (){
	return source_previous_tx_hash_;
}

std::uint32_t&
Transaction::get_destination_previous_ledger_seq (){
	return destination_previous_ledger_seq_;
}

uint256&
Transaction::get_destination_previous_tx_hash (){
	return destination_previous_tx_hash_;
}
}
