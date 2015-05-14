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
	Account (uint256 address, double balance, std::uint32_t p_ledger, uint256 p_tx);
	~Account (){}

	std::string serializer ();
	void		unserializer (std::string str);
	uint256&	get_account_address ();
	double&		get_account_balance ();
	std::uint32_t&	get_previous_ledger_seq ();
	uint256&	get_previous_tx_hash ();

	bool		add_balance (double amount){
		double tmp = account_balance_ + amount;
		if (tmp < 0)	return false;
		account_balance_ = tmp;
		return true;
	}
	void		set_previous_ledger_seq (std::uint32_t seq){
		previous_ledger_seq_ = seq;
	}
	void		set_previous_tx_hash (uint256& hash){
		previous_tx_hash_ = hash;
	}

private:
	uint256		account_address_;
	double		account_balance_;
	std::uint32_t		previous_ledger_seq_;
	uint256		previous_tx_hash_;
};

}

#endif
