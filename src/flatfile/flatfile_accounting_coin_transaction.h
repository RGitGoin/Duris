#ifndef DURIS_FLATFILE_ACCOUNTING_COIN_TRANSACTION_H
#define DURIS_FLATFILE_ACCOUNTING_COIN_TRANSACTION_H
#include "persistence/critical_command_completion.h"
#include <string>
// Wallet-wallet root only; one authority bundle retains both native effects,
// child identity reservations and the exact result. Gameplay admission is separate.
class flatfile_accounting_coin_transaction
{
    public:
	static critical_apply_result apply(const std::string &, const critical_command &);
};
#endif
