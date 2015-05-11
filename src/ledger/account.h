#ifndef _BUBI_ACCOUNT_H_
#define _BUBI_ACCOUNT_H_

#include "utils.h"

#include <string>
#include <memory>

namespace Bubi{

class Account {
public:
	typedef std::shared_ptr<Account>	pointer;

	Account (){}
	Account (uint256 address, double balance, uint256 previous);
	~Account (){}

	std::string serializer ();
	void		unserializer (std::string str);
	uint256&	get_account_address ();
	double&		get_account_balance ();
	uint256&	get_previous_ledger_hash ();

private:
	uint256		account_address_;
	double		account_balance_;
	uint256		previous_ledger_hash_;
};

}

#endif
