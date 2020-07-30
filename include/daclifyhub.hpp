#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/action.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <eosio/permission.hpp>
#include <eosio/multi_index.hpp>

#include <json.hpp>

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
    TABLE settings {
      extended_asset system_token;
    };
    typedef eosio::singleton<"settings"_n, settings> settings_table;
    
    ACTION versioning(name modulename, checksum256 codehash, checksum256 abihash, string json_src, string info, uint64_t update_key);
    ACTION versionstate(name modulename, uint64_t version, uint8_t status);
    ACTION setgrpstate(name groupname, uint8_t newstate);
    ACTION setsettings(settings new_settings, bool remove);

    ACTION compreg(name owner, string src, checksum256 hash, string info_json );
    ACTION compupdatesr(uint64_t comp_id, string new_src, checksum256 new_hash );
    ACTION compupdatein(uint64_t comp_id, string info_json );
    ACTION compapprove(uint64_t comp_id, checksum256 hash);
    ACTION compunapprov(uint64_t comp_id );
    ACTION compdelete(uint64_t comp_id );

    ACTION creategroup(name groupname, name creator, asset resource_estimation);
    ACTION activate(name groupname, name creator);

    ACTION linkgroup(name groupname, name creator, groupmeta meta, uiconf ui, uint8_t state, vector<name> tags, uint64_t claps, time_point_sec creation_date);//add an existing group to the hub
    ACTION unlinkgroup(name groupname);


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

    ACTION clear();
    //notification handlers
    [[eosio::on_notify("*::transfer")]]
    void on_transfer(name from, name to, asset quantity, string memo);



  private:

    TABLE state {
        uint64_t comp_id = 100;
    };
    typedef eosio::singleton<"state"_n, state> state_table;


    //scoped table by account
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
      uint64_t claps;
      uint64_t r1;
      uint64_t r2;
      uint64_t r3;
      time_point_sec creation_date = time_point_sec(current_time_point() );
      auto primary_key() const { return groupname.value; }
      uint64_t by_claps() const { return static_cast<uint64_t>(UINT64_MAX - claps);}
      uint64_t by_creator() const { return creator.value;}
    };
    typedef multi_index<name("groups"), groups,
      eosio::indexed_by<"byclaps"_n, eosio::const_mem_fun<groups, uint64_t, &groups::by_claps >>,
      eosio::indexed_by<"bycreator"_n, eosio::const_mem_fun<groups, uint64_t, &groups::by_creator >>
    > groups_table;
    
    //scoped core/elections etc
    TABLE versions {
      uint64_t version = 1;
      checksum256 codehash;
      checksum256 abihash;
      string json_src;//block_num and trxid OR urls, only used in the frontend
      string info;
      uint8_t status = 0;
      uint64_t r2;
      time_point_sec creation_date = time_point_sec(current_time_point() );
      auto primary_key() const { return version; }
    };
    typedef multi_index<name("versions"), versions> versions_table;

    TABLE components {
        uint64_t comp_id;
        name owner;
        //PAIR=> 0: holds the approved src, 1: holds unapproved src (ie update). when approved the new src will replace the old on 0. 1 will be empty again
        vector<string>src; 
        vector<checksum256>hash;
        string info_json;
        uint8_t approve_level = 0;
        time_point_sec last_update = time_point_sec(current_time_point() );
        auto primary_key() const { return comp_id; }
    };
    typedef multi_index<name("components"), components> components_table;

    //FUNCTIONS
    void create_group_account(name groupname, name creator, asset resource_estimation);
    void set_owner_permission(name groupname, name creator);
    void sub_deposit( const name& account, const extended_asset& value);
    void add_deposit( const name& account, const extended_asset& value);
    struct sort_authorization_by_name{
      inline bool operator() (const eosiosystem::permission_level_weight& plw1, const eosiosystem::permission_level_weight& plw2){
        return (plw1.permission.actor < plw2.permission.actor);
      }
    };

    bool is_checksum256_populated(const checksum256& hash){
      checksum256 test;
      return test != hash;
    }

    std::string hash_to_str(const checksum256& hash) {
      static const char table[] = "0123456789abcdef";
      auto              bytes   = hash.extract_as_byte_array();
      std::string       result;
      for (uint8_t byte : bytes) {
         result += table[byte >> 4];
         result += table[byte & 0xf];
      }
      return result;
    }     
    

};



