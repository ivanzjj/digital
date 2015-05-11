#ifndef _BUBI_TRANSACTION_H_
#define _BUBI_TRANSACTION_H_

#include <string>
#include <memory>

#include "utils.h"

namespace Bubi{

class Transaction{

public:
	typedef std::shared_ptr<Transaction>	pointer;

	Transaction ();
	Transaction (uint256 _source_address_,
				uint256  _destination_address_,
				double	  _payment_amount_,
				uint256  _last_ledger_hash_);
	~Transaction ();

	std::string serializer ();
	void unserializer (std::string str);

	uint256&	get_source_address ();
	uint256&	get_destination_address ();
	double		get_payment_amount ();
	uint256&	get_last_ledger_hash ();

private:
	uint256		source_address_;
	uint256		destination_address_;
	double		payment_amount_;
	uint256		last_ledger_hash_;
};

}

#endif
