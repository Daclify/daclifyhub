    
    ACTION daclifyhub::compreg(name owner, string src, checksum256 hash, string info_json ){
        require_auth(owner);
        check(is_checksum256_populated(hash), "must supply a sha256 hash of the component src");
        check(src.rfind("https://", 0) == 0, "Url must be https://");

        vector<checksum256>hashpair;
        hashpair.push_back(hash);

        vector<string>srcpair;
        srcpair.push_back(src);

        state_table _state(get_self(), get_self().value);
        state s = _state.get_or_create(get_self(), state() );

        components_table _components(get_self(), get_self().value );
        _components.emplace(get_self(), [&](auto& c) {
            c.comp_id = s.comp_id;
            c.owner = owner;
            c.hash = hashpair;
            c.src = srcpair;
            c.info_json = info_json;
            c.approve_level = 0;
        });

        s.comp_id++;
        _state.set(s, get_self() );
    }

    ACTION daclifyhub::compupdatesr(uint64_t comp_id, string new_src, checksum256 new_hash ){
        components_table _components(get_self(), get_self().value );
        auto itr = _components.find(comp_id);
        check(itr != _components.end(), "Component with this id doesn't exists.");
        require_auth(itr->owner);

        vector<checksum256>hashpair;
        vector<string>srcpair;
        uint8_t approve_level;

        //component isn't approved yet so replace data on first position
        if(itr->approve_level==0){
            hashpair.push_back(new_hash);
            srcpair.push_back(new_src);
            approve_level = 0;
        }
        //previous version is approved so use second position in map
        else{
            hashpair.push_back(itr->hash[0]);
            srcpair.push_back(itr->src[0]);
            hashpair.push_back(new_hash);
            srcpair.push_back(new_src);
            approve_level = 2; //2 means that the second needs to be reviewed/approved, first is already approved
        }

        _components.modify( itr, same_payer, [&]( auto& c) {
            c.hash = hashpair;
            c.src = srcpair;
            c.approve_level = approve_level;
            //c.last_update = time_point_sec(current_time_point() );
        });
        
    }

    ACTION daclifyhub::compupdatein(uint64_t comp_id, string info_json){
        components_table _components(get_self(), get_self().value );
        auto itr = _components.find(comp_id);
        check(itr != _components.end(), "Component with this id doesn't exists.");
        require_auth(itr->owner);

        _components.modify( itr, same_payer, [&]( auto& c) {
            c.info_json = info_json;
            //c.last_update = time_point_sec(current_time_point() );
        });
    }


    ACTION daclifyhub::compapprove(uint64_t comp_id , checksum256 hash ){
        require_auth(get_self() );
        components_table _components(get_self(), get_self().value );
        auto itr = _components.find(comp_id);
        check(itr != _components.end(), "Component with this id doesn't exists.");
        vector<checksum256>hashpair;
        vector<string>srcpair;


        if( itr->approve_level==0){//approve a new component
            check(itr->hash[0] == hash, ("Hashes don't match, current is " + hash_to_str(itr->hash[0]) + " but received " + hash_to_str(hash)).c_str() );
            hashpair = itr->hash;
            srcpair = itr->src;

        }
        else if(itr->approve_level==1){//already approved
            check(false, "component already approved");
        }
        else if(itr->approve_level==2){//approve a component update
            check(itr->hash[1] == hash, ("Hashes don't match, updated hash is " + hash_to_str(itr->hash[1]) + " but received " + hash_to_str(hash)).c_str() );
            hashpair.push_back(itr->hash[1]);
            srcpair.push_back(itr->src[1]);
        }

        _components.modify( itr, same_payer, [&]( auto& c) {
            c.hash = hashpair;
            c.src = srcpair;
            c.approve_level = 1;
            c.last_update = time_point_sec(current_time_point() );
        }); 
    }

    ACTION daclifyhub::compunapprov(uint64_t comp_id ){
        require_auth(get_self() );
        components_table _components(get_self(), get_self().value );
        auto itr = _components.find(comp_id);
        check(itr != _components.end(), "Component with this id doesn't exists.");

        vector<checksum256>hashpair;
        vector<string>srcpair;
        uint8_t approve_level;

        if(itr->approve_level == 0){//delete the component from the table
            check(false, "Component not approved yet.");
        }
        else if(itr->approve_level == 1){
            hashpair = itr->hash;
            srcpair = itr->src;
            approve_level = 0;
        }
        else if(itr->approve_level == 2){
            //delete the update on position second
            hashpair.push_back(itr->hash[0]);
            srcpair.push_back(itr->src[0]);
            approve_level = 1;
        }

        _components.modify( itr, same_payer, [&]( auto& c) {
            c.hash = hashpair;
            c.src = srcpair;
            c.approve_level = approve_level;
        });   
    }
    ACTION daclifyhub::compdelete(uint64_t comp_id ){
        require_auth(get_self() );
        components_table _components(get_self(), get_self().value );
        auto itr = _components.find(comp_id);
        check(itr != _components.end(), "Component with this id doesn't exists.");
        check(itr->approve_level == 0, "Component can't be deleted, downgrade approve_level first.");
        _components.erase(itr); 
    }
    
    
