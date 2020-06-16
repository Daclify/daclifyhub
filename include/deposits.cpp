ACTION daclifyhub::opendeposit(name account, name ram_payer, extended_asset amount) {
  require_auth(ram_payer);
  check(is_account(account), "Account doesn't exist");
  deposits_table _deposits( get_self(), account.value);
  const auto& itr = _deposits.find( amount.quantity.symbol.raw() );
  
  if(itr == _deposits.end() ){//not yet in table
    amount.quantity.amount = 0;
    _deposits.emplace(ram_payer, [&](auto& n) {
        n.balance = amount;
    });
  }
}

ACTION daclifyhub::withdraw(name account, extended_asset amount) {
  require_auth(account);
  check(account != get_self(), "Can't withdraw to self.");
  check(amount.quantity.amount > 0, "Amount must be greater then zero.");

  sub_deposit(account, amount);
  
  action(
    permission_level{get_self(), "active"_n},
    amount.contract, "transfer"_n,
    make_tuple(get_self(), account, amount.quantity, string("withdraw from user account"))
  ).send();
  
}

ACTION daclifyhub::movedeposit(name from, name to, extended_asset amount) {
  require_auth(from);
  check(amount.quantity.amount > 0, "Amount must be greater then zero");
  sub_deposit(from, amount);
  add_deposit(to, amount);
}

void daclifyhub::sub_deposit( const name& account, const extended_asset& value) {
    deposits_table _deposits( get_self(), account.value);
    const auto& itr = _deposits.get( value.quantity.symbol.raw(), "No balance with this symbol.");
    check( itr.balance >= value, "Overdrawn balance");

    if(account != get_self() && itr.balance == value){
        _deposits.erase(itr);
        return;
    }

    _deposits.modify( itr, same_payer, [&]( auto& a) {
        a.balance -= value;
    });

}


void daclifyhub::add_deposit( const name& account, const extended_asset& value){
    deposits_table _deposits( get_self(), account.value);
    auto itr = _deposits.find( value.quantity.symbol.raw() );

    if(account != get_self() ){
        check(itr != _deposits.end(), "Receiver doesn't have an open deposit account.");
        _deposits.modify( itr, same_payer, [&]( auto& a) {
            a.balance += value;
        });
    }
    else{
        if(itr == _deposits.end()){
            _deposits.emplace( get_self(), [&]( auto& a){
                a.balance = value;
            });     
        }
        else{
            _deposits.modify( itr, same_payer, [&]( auto& a) {
                a.balance += value;
            });
        }

    }
}