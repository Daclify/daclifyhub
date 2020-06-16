#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/action.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/permission.hpp>
#include <eosio/multi_index.hpp>

/*

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/singleton.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/symbol.hpp>
#include <eosio/action.hpp>
#include <eosio/time.hpp>
#include <eosio/permission.hpp>
*/

#include <system_structs.hpp>

using namespace std;
using namespace eosio;

CONTRACT daclifyhub : public contract {
  public:

    struct uiconf{
      string logo;
      string hexcolor = "#4DB6AC";
      string custom_ui_url;
    };

    struct link{
      string icon="";
      string label="";
      string url="";
    };

    struct groupmeta{
      string title;
      string about;
      vector <link> links;
    };


    using contract::contract;

    ACTION versioning(name modulename, string codehash, string abi_url, string wasm_url, checksum256 trx_id, uint64_t block_num, uint64_t update_key);

    ACTION creategroup(name groupname, name creator);
    ACTION activate(name groupname, name creator);

    ACTION opendeposit(name account, name ram_payer, extended_asset amount);
    ACTION withdraw(name account, extended_asset amount);
    ACTION movedeposit(name from, name to, extended_asset amount);

    ACTION updateabout(name groupname, string about);
    ACTION updatetitle(name groupname, string title);
    ACTION updatelinks(name groupname, vector <link> newlinks);
    ACTION updatetags(name groupname, vector<name> tags);
    ACTION updatelogo(name groupname, string logo);
    ACTION updatecolor(name groupname, string hexcolor);
    ACTION setcustomui(name groupname, string url);
    
    ACTION migrategrps (uint8_t batch, uint8_t skip);

    ACTION messagebus(name sender_group, name event, string message, vector<name> receivers);

    ACTION deletegroup(name groupname);

    ACTION clear();
    //notification handlers
    [[eosio::on_notify("*::transfer")]]
    void on_transfer(name from, name to, asset quantity, string memo);


  private:

    //scoped table
    TABLE deposits {
      extended_asset balance;
      uint64_t primary_key()const { return balance.quantity.symbol.raw(); }
    };
    typedef multi_index<"deposits"_n, deposits> deposits_table;


    TABLE groups {
      name groupname;
      name  creator;
      groupmeta meta;
      uiconf ui;
      uint8_t state = 0;
      vector <name> tags;
      bool flag = 0;
      uint64_t claps;
      uint64_t r1;
      uint64_t r2;
      time_point_sec creation_date = time_point_sec(current_time_point() );
      auto primary_key() const { return groupname.value; }
      uint64_t by_claps() const { return static_cast<uint64_t>(UINT64_MAX - claps);}
    };
    typedef multi_index<name("groups"), groups,
      eosio::indexed_by<"byclaps"_n, eosio::const_mem_fun<groups, uint64_t, &groups::by_claps >>
    > groups_table;
    
    //scoped core/elections etc
    TABLE versions {
      uint64_t version = 1;
      string codehash;
      string abi_url;
      string wasm_url;
      checksum256 trx_id;
      uint64_t block_num;
      time_point_sec creation_date = time_point_sec(current_time_point() );
      auto primary_key() const { return version; }
    };
    typedef multi_index<name("versions"), versions> versions_table;


    //FUNCTIONS see
    void create_group_account(name groupname, name creator);
    void set_owner_permission(name groupname, name creator);
    void sub_deposit( const name& account, const extended_asset& value);
    void add_deposit( const name& account, const extended_asset& value);

  
    struct sort_authorization_by_name{
      inline bool operator() (const eosiosystem::permission_level_weight& plw1, const eosiosystem::permission_level_weight& plw2){
        return (plw1.permission.actor < plw2.permission.actor);
      }
    };        
    

};



