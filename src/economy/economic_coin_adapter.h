#ifndef DURIS_ECONOMIC_COIN_ADAPTER_H
#define DURIS_ECONOMIC_COIN_ADAPTER_H

#include "economy/coin_transfer_command.h"
#include "economy/economic_currency_adapter.h"

constexpr uint32_t ECONOMIC_WRITER_COIN_WALLETS = 3;
constexpr size_t ECONOMIC_COIN_WALLET_FACT_BYTES = 48;

// Both snapshots precede the root transaction's first mutation. Repositories
// must establish the native/lifetime mapping and locks; these are value inputs.
using economic_coin_wallet_authority = std::array<economic_currency_authority, 2>;

class economic_prepared_coin_wallets
{
    public:
	const std::array<currency_prepared_mutation, 2> &mutations() const { return mutations_; }
	const economic_accounting_plan &plan() const { return plan_; }
	economic_accounting_error agrees_with(const economic_accounting_plan &candidate) const;

    private:
	std::array<currency_prepared_mutation, 2> mutations_;
	economic_accounting_plan plan_;
	std::vector<uint8_t> encoded_;
	economic_prepared_coin_wallets(std::array<currency_prepared_mutation, 2> mutations,
				       economic_accounting_plan plan, std::vector<uint8_t> encoded);
	friend economic_accounting_error economic_coin_wallets_prepare(
		const critical_command &, const economic_frozen_intent &,
		const economic_coin_wallet_authority &, currency_revision_policy,
		std::optional<economic_prepared_coin_wallets> *, critical_failure_stage *);
};

// Wallet-to-wallet coin commands only. Pile endpoints need custody evidence and
// a separate typed adapter. No source/sink/issuance capabilities are accepted.
economic_accounting_error
economic_coin_wallets_intent(const critical_command &command, const critical_operation_id &epoch,
			     const std::array<economic_account_key, 2> &wallets,
			     const std::array<economic_account_key, 2> &banks,
			     std::vector<uint8_t> *encoded);
economic_accounting_error
economic_coin_wallets_prepare(const critical_command &command, const economic_frozen_intent &intent,
			      const economic_coin_wallet_authority &authority,
			      currency_revision_policy policy,
			      std::optional<economic_prepared_coin_wallets> *prepared,
			      critical_failure_stage *failure_stage = nullptr);

#endif
