#include "account.h"
#include "transaction.pb.h"

namespace Bubi{

Account::Account (uint256 address, double balance, uint256 previous)
: account_address_ (address)
, account_balance_ (balance)
, previous_ledger_hash_ (previous)
{}

std::string
Account::serializer (){
	bubi::Account acc;
	acc.set_account_address (account_address_.to_string ());
	acc.set_account_balance (account_balance_);
	acc.set_previous_ledger_hash (previous_ledger_hash_.to_string ());

	std::string out;
	if(!acc.SerializeToString (&out)){
		std::cerr <<"Accountserializer is erro!" << std::endl;
	}
	return out;
}

void
Account::unserializer (std::string str){
	bubi::Account acc;
	if(!acc.ParseFromString (str)){
		std::cerr <<"Accountunserializer is erro!" << std::endl;
	}
	account_address_.binary_init (acc.account_address ().c_str ());
	account_balance_ = acc.account_balance ();
	previous_ledger_hash_.binary_init (acc.previous_ledger_hash ().c_str ());
}

uint256&
Account::get_account_address (){
	return account_address_;
}

double&
Account::get_account_balance (){
	return account_balance_;
}

uint256&
Account::get_previous_ledger_hash (){
	return previous_ledger_hash_;
}

}
