#include "router.h"

Json::Value ExitWithJSON(int status, std::string message=""){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["status"] = status;
    out["message"] = message;
    return out;
}

Json::Value router::Router::Login(router::Context *ctx){
    if (ctx->user != nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto content = ctx->req["content"];
    if (!content.isObject()){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    auto res = this->repo->Login(
        content["username"].asString(),
        content["password"].asString()
    );

    if (!res){
        return ExitWithJSON(
            router::STATUS_UNAUTHORIZED, 
            "wrong username or password"
        );
    }

    // add user to the map of logged in users
    this->users[ctx->connectionID] = res; 

    return ExitWithJSON(router::STATUS_OK);
}

Json::Value router::Router::Balance(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto balance = this->repo->Balance(ctx->user->ID);

    // return the custom balance response
    Json::Value out;
    out["status"] = router::STATUS_OK;
    out["balance"] = balance;
    
    return out;
}

Json::Value router::Router::Transfer(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto content = ctx->req["content"];
    if (!content.isObject()){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    auto username = content["username"].asString();
    auto amount = content["amount"].asInt();

    auto res =  this->repo->Transfer(
        ctx->user->ID,
        content["to"].asString(),
        content["amount"].asInt()
    );
    
    if (!res){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    return ExitWithJSON(router::STATUS_OK);
}

Json::Value router::Router::History(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto history = this->repo->History(ctx->user->username);

    // return the custom balance history
    Json::Value out;
    out["status"] = router::STATUS_OK;
    out["history"] = Json::arrayValue;
    
    for (const auto& transfer : history) {
        Json::Value jsonTransfer;
        
        jsonTransfer["amount"] = transfer.amount;
        jsonTransfer["from"] = transfer.from;
        jsonTransfer["to"] = transfer.to;

        out["history"].append(jsonTransfer);
    }
    
    return out;
}


void router::Router::Disconnect(int sd){
    auto it = this->users.find(sd);
    if (it == this->users.end()){
        return;
    }

    std::string route = "logout";
    std::cout << "/" << route <<  std::string(10 - route.length(), ' ' ) << 201 << std::endl; 
    this->users.erase(sd);
}


std::string router::Router::Handle(int sd, std::string message){
    Json::Reader reader;
    Json::Value content;
    reader.parse(message, content);

    std::shared_ptr<entity::User> us = nullptr;

    // extract the user from the map
    auto it = this->users.find(sd);

    // if exists set it
    if (it != this->users.end()){
        us = it->second;
    }

    // create the context
    router::Context ctx{
        .user = us,
        .req = content,
        .connectionID = sd,
    };


    auto out = ExitWithJSON(STATUS_NOT_FOUND); 

    auto route = content["route"].asString();

    if (route == "login")           out = this->Login(&ctx);
    else if (route == "balance")    out = this->Balance(&ctx);
    else if (route == "transfer")   out = this->Transfer(&ctx);
    else if (route == "history")    out = this->History(&ctx);

    // log the request
    std::cout << "/" << route <<  std::string(10 - route.length(), ' ' ) << out["status"] << std::endl; 

    Json::StreamWriterBuilder builder;
    std::string str = Json::writeString(builder, out);
    return str;
}

router::Router::Router(router::IRepo* repo){
    if(repo == nullptr){
        std::cerr << "You must specify a repo " << std::endl;
        exit(EXIT_FAILURE);
    }
    this->repo = repo;
}