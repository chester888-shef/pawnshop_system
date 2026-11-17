#pragma once
#include "valuation.hpp"
#include <map>
#include <utility>

class Client {
    int id_ = 0;
    std::string fullName_, document_, contact_;
public:
    Client() = default;
    Client(int id, std::string fullName, std::string document, std::string contact)
        : id_(id), fullName_(std::move(fullName)), document_(std::move(document)), contact_(std::move(contact)) {}
    int id() const { return id_; }
    void set_id(int value) { id_ = value; }
    const std::string& fullName() const { return fullName_; }
    void set_fullName(std::string value) { fullName_ = std::move(value); }
    const std::string& document() const { return document_; }
    void set_document(std::string value) { document_ = std::move(value); }
    const std::string& contact() const { return contact_; }
    void set_contact(std::string value) { contact_ = std::move(value); }
};
enum class Event { extend, redeem, expire, putOnSale, sell };
class InvalidTransitionError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
class IState {
public:
    virtual ~IState() = default;
    virtual std::string name() const = 0;
    virtual std::unique_ptr<IState> handle(Pledge&, Event) const = 0;
};
#define PLEDGE_STATE(Type) \
class Type : public IState { \
public: \
    std::string name() const override { return #Type; } \
    std::unique_ptr<IState> handle(Pledge&, Event) const override; \
};
PLEDGE_STATE(Accepted)
PLEDGE_STATE(Extended)
PLEDGE_STATE(Redeemed)
PLEDGE_STATE(Expired)
PLEDGE_STATE(ForSale)
PLEDGE_STATE(Sold)
#undef PLEDGE_STATE

class Pledge {
    int id_;
    Client client_;
    PledgeParams params_;
    std::unique_ptr<IState> state_ = std::make_unique<Accepted>();
    std::unique_ptr<IValuationStrategy> strategy_;
    bool assigned_ = false;
protected:
    Pledge(int id, Client client, PledgeParams params, std::unique_ptr<IValuationStrategy> strategy)
        : id_(id), client_(std::move(client)), params_(params), strategy_(std::move(strategy)) {}
public:
    virtual ~Pledge() = default;
    virtual std::string category() const = 0;
    int id() const { return id_; }
    const Client& client() const { return client_; }
    const PledgeParams& params() const { return params_; }
    std::string state() const { return state_->name(); }
    bool hasContract() const { return assigned_; }
    void assignContract() {
        if (assigned_) throw std::invalid_argument("Pledge already has a contract");
        assigned_ = true;
    }
    double appraise() const { return strategy_->appraise(*this); }
    double maxLoan() const { return std::min(appraise() * strategy_->loanLimitRatio(), strategy_->loanCap()); }
    void handle(Event event) { state_ = state_->handle(*this, event); }
};
class JewelryPledge : public Pledge {
public:
    JewelryPledge(int id, Client client, PledgeParams params)
        : Pledge(id, std::move(client), params, std::make_unique<JewelryValuation>()) { appraise(); }
    std::string category() const override { return "jewelry"; }
};
class TechPledge : public Pledge {
public:
    TechPledge(int id, Client client, PledgeParams params)
        : Pledge(id, std::move(client), params, std::make_unique<TechValuation>()) { appraise(); }
    std::string category() const override { return "tech"; }
};
class VehiclePledge : public Pledge {
public:
    VehiclePledge(int id, Client client, PledgeParams params)
        : Pledge(id, std::move(client), params, std::make_unique<VehicleValuation>()) { appraise(); }
    std::string category() const override { return "vehicle"; }
};
class PledgeCreator {
public:
    virtual ~PledgeCreator() = default;
    virtual std::shared_ptr<Pledge> create(int, const Client&, const PledgeParams&) const = 0;
};
#define PLEDGE_CREATOR(Type, Product) \
class Type : public PledgeCreator { \
public: \
    std::shared_ptr<Pledge> create(int id, const Client& client, const PledgeParams& params) const override { \
        return std::make_shared<Product>(id, client, params); \
    } \
};
PLEDGE_CREATOR(JewelryCreator, JewelryPledge)
PLEDGE_CREATOR(TechCreator, TechPledge)
PLEDGE_CREATOR(VehicleCreator, VehiclePledge)
#undef PLEDGE_CREATOR
class CollateralFactory {
    std::map<std::string, std::shared_ptr<PledgeCreator>> creators_;
public:
    CollateralFactory() {
        registerCreator("jewelry", std::make_shared<JewelryCreator>());
        registerCreator("tech", std::make_shared<TechCreator>());
        registerCreator("vehicle", std::make_shared<VehicleCreator>());
    }
    void registerCreator(const std::string& category, std::shared_ptr<PledgeCreator> creator) {
        if (!creator) throw std::invalid_argument("Null creator");
        creators_[category] = std::move(creator);
    }
    std::shared_ptr<Pledge> create(const std::string& category, int id, const Client& client, const PledgeParams& params) const {
        auto it = creators_.find(category);
        if (it == creators_.end()) throw std::invalid_argument("Unknown pledge category");
        return it->second->create(id, client, params);
    }
};
