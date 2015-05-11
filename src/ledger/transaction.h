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
	Transaction (uint256	s_address,
				uint256		d_address,
				double		amount);
	~Transaction ();

	std::string serializer ();
	void unserializer (std::string str);

	uint256&	get_source_address ();
	uint256&	get_destination_address ();
	double		get_payment_amount ();
	std::uint32_t&	get_source_previous_ledger_seq ();
	uint256&	get_source_previous_tx_hash ();
	std::uint32_t&	get_destination_previous_ledger_seq ();
	uint256&	get_destination_previous_tx_hash ();

	void		set_source_previous_ledger_seq (std::uint32_t seq){
		source_previous_ledger_seq_ = seq;
	}
	void		set_destination_previous_ledger_seq (std::uint32_t seq){
		destination_previous_ledger_seq_ = seq;
	}
	void		set_source_previous_tx_hash (uint256& hash){
		source_previous_tx_hash_ = hash;
	}
	void		set_destination_previous_tx_hash (uint256& hash){
		destination_previous_tx_hash_ = hash;
	}

private:
	uint256		source_address_;
	uint256		destination_address_;
	double		payment_amount_;
	std::uint32_t source_previous_ledger_seq_;
	uint256		source_previous_tx_hash_;
	std::uint32_t destination_previous_ledger_seq_;
	uint256		destination_previous_tx_hash_;
};

}

#endif
