void daclifyhub::create_group_account(name groupname, name creator){

    require_auth(creator);
    //pay for resources
    symbol eos_sym = symbol(symbol_code("EOS"), 4);
    deposits_table _deposits(get_self(), creator.value);
    auto deposit = _deposits.get(eos_sym.raw(), "Account doesn't have EOS in deposits. Please transfer EOS to the hub to pay for RAM/NET/CPU." );
    check(deposit.balance.quantity.amount > 500000, "Not enough EOS deposited to pay for RAM/NET/CPU.");
    asset ram_quantity = asset(deposit.balance.quantity.amount - 20000, eos_sym);
    asset net_quantity = asset(10000, eos_sym);
    asset cpu_quantity = asset(10000, eos_sym);
    bool transfer_bw = true;
    sub_deposit(creator, deposit.balance);

    //construct setup authorities for the new group account
    vector<eosiosystem::permission_level_weight> owners;
    vector<eosiosystem::permission_level_weight> actives;

    eosiosystem::permission_level_weight pml_self{
        .permission = permission_level{get_self(), "active"_n},
        .weight = (uint16_t) 1,
    };

    eosiosystem::permission_level_weight pml_code{
        .permission = permission_level{groupname, "eosio.code"_n },
        .weight = (uint16_t) 1,
    };

    eosiosystem::permission_level_weight pml_creator{
        .permission = permission_level{creator, "active"_n },
        .weight = (uint16_t) 1,
    };

    actives.push_back(pml_creator);

    //owners.push_back(pml_creator);
    owners.push_back(pml_code);
    owners.push_back(pml_self);
    std::sort(owners.begin(), owners.end(), sort_authorization_by_name());
    
    eosiosystem::authority setup_owner_authority{
        .threshold = 1,
        .accounts = owners
    };

    eosiosystem::authority setup_active_authority{
        .threshold = 1,
        .accounts = actives
    };


    action(
        permission_level{ get_self(), "active"_n },
        "eosio"_n,
        "newaccount"_n,
        std::make_tuple(get_self(), groupname, setup_owner_authority, setup_active_authority )
    ).send();

    action(
        permission_level{ get_self(), "active"_n },
        "eosio"_n,
        "delegatebw"_n,
        std::make_tuple(get_self(), groupname, net_quantity, cpu_quantity, transfer_bw )
    ).send();

    action(
        permission_level{ get_self(), "active"_n },
        "eosio"_n,
        "buyram"_n,
        std::make_tuple(get_self(), groupname, ram_quantity )
    ).send();

    
}









