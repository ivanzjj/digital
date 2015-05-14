#ifndef _BUBI_INTERFACE_H_
#define _BUBI_INTERFACE_H_

#include <string>
#include <vector>

#include "transaction.h"
#include "ledger.h"

namespace Bubi {

extern Ledger::pointer last_ledger;

int create_transaction (std::string, std::string, double);

int create_account (std::string);

double get_balance (std::string);

std::vector <Transaction::pointer> get_transaction_history (std::string);


}

#endif
