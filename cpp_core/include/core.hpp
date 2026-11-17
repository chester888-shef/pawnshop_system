#pragma once
#include "pledge.hpp"
#include <vector>

// Dates cross the language boundary as Python date.toordinal() values.
class Payment {
    int date_;
    double interest_, penalty_;
public:
    Payment(int date, double interest, double penalty) : date_(date), interest_(interest), penalty_(penalty) {}
    int date() const { return date_; }
    double interest() const { return interest_; }
    double penalty() const { return penalty_; }
};
class Contract {
    int id_, issuedOn_, periodStart_, dueDate_;
    std::shared_ptr<Pledge> pledge_;
    double principal_;
    std::vector<Payment> payments_;
public:
    Contract(int id, std::shared_ptr<Pledge> pledge, double principal, int issuedOn, int termDays)
        : id_(id), issuedOn_(issuedOn), periodStart_(issuedOn), dueDate_(0),
          pledge_(std::move(pledge)), principal_(principal) {
        constexpr int maxDateOrdinal = 3652059; // Python date.max.toordinal()
        if (!pledge_ || termDays < 1 || termDays > maxDateOrdinal || issuedOn < 1 || issuedOn > maxDateOrdinal - termDays)
            throw std::invalid_argument("Invalid contract parameters");
        positive(principal, "Invalid principal");
        if (principal > pledge_->maxLoan()) throw std::invalid_argument("Loan limit exceeded");
        dueDate_ = issuedOn + termDays;
    }
    int id() const { return id_; }
    int issuedOn() const { return issuedOn_; }
    int periodStart() const { return periodStart_; }
    int dueDate() const { return dueDate_; }
    double principal() const { return principal_; }
    std::shared_ptr<Pledge> pledge() const { return pledge_; }
    const std::vector<Payment>& payments() const { return payments_; }
    void addPayment(Payment payment) { payments_.push_back(payment); }
    void setPeriod(int start, int due) { periodStart_ = start; dueDate_ = due; }
};
class IInterestPolicy {
public:
    virtual ~IInterestPolicy() = default;
    virtual int graceDays() const = 0;
    virtual int termDays() const = 0;
    virtual double dailyRate() const = 0;
    virtual double penaltyRate() const = 0;
};
class StandardInterestPolicy : public IInterestPolicy {
public:
    int graceDays() const override { return 7; }
    int termDays() const override { return 30; }
    double dailyRate() const override { return 0.03 / 30; }
    double penaltyRate() const override { return 0.005; }
};
class InterestCalculator {
    std::shared_ptr<IInterestPolicy> policy_;
    static int elapsed(const Contract& contract, int date) {
        if (date < contract.periodStart() || date > 3652059)
            throw std::invalid_argument("Date is outside the current contract period");
        return date - contract.periodStart();
    }
public:
    explicit InterestCalculator(std::shared_ptr<IInterestPolicy> policy) : policy_(std::move(policy)) {
        if (!policy_) throw std::invalid_argument("Null interest policy");
    }
    double accruedInterest(const Contract& contract, int date) const {
        int days = elapsed(contract, date);
        return days <= policy_->graceDays() ? 0 : contract.principal() * policy_->dailyRate() * days;
    }
    double penalty(const Contract& contract, int date) const {
        elapsed(contract, date);
        return std::min(contract.principal() * policy_->penaltyRate() * std::max(0, date - contract.dueDate()), contract.principal());
    }
    double redemptionAmount(const Contract& contract, int date) const {
        // Earlier periods were settled at extension. Never subtract those payments again.
        return contract.principal() + accruedInterest(contract, date) + penalty(contract, date);
    }
};
class LombardCore {
    CollateralFactory factory_;
    std::shared_ptr<IInterestPolicy> policy_;
    InterestCalculator calculator_;
    static void validateOperationDate(const Contract& contract, int date) {
        int latest = contract.periodStart();
        if (!contract.payments().empty()) latest = std::max(latest, contract.payments().back().date());
        if (date < latest || date > 3652059) throw std::invalid_argument("Invalid operation date");
    }
    static void expireIfNeeded(Contract& contract, int date) {
        auto pledge = contract.pledge();
        if (date > contract.dueDate() && (pledge->state() == "Accepted" || pledge->state() == "Extended"))
            pledge->handle(Event::expire);
    }
public:
    explicit LombardCore(std::shared_ptr<IInterestPolicy> policy = std::make_shared<StandardInterestPolicy>())
        : policy_(std::move(policy)), calculator_(policy_) {}
    std::shared_ptr<Pledge> createPledge(const std::string& category, int id, const Client& client, const PledgeParams& params) const {
        return factory_.create(category, id, client, params);
    }
    std::shared_ptr<Contract> openContract(int id, std::shared_ptr<Pledge> pledge, double principal, int date) const {
        if (!pledge || pledge->state() != "Accepted" || pledge->hasContract())
            throw std::invalid_argument("Pledge is not available for a new contract");
        auto contract = std::make_shared<Contract>(id, pledge, principal, date, policy_->termDays());
        pledge->assignContract();
        return contract;
    }
    double accruedInterest(const Contract& contract, int date) const { return calculator_.accruedInterest(contract, date); }
    double penalty(const Contract& contract, int date) const { return calculator_.penalty(contract, date); }
    double redemptionAmount(const Contract& contract, int date) const { return calculator_.redemptionAmount(contract, date); }
    double extendContract(Contract& contract, int date) const {
        validateOperationDate(contract, date);
        if (policy_->termDays() < 1 || policy_->termDays() > 3652059 || date > 3652059 - policy_->termDays())
            throw std::invalid_argument("Extension exceeds the supported date range");
        expireIfNeeded(contract, date);
        auto pledge = contract.pledge();
        if (pledge->state() != "Accepted" && pledge->state() != "Extended")
            throw InvalidTransitionError("Only active contracts can be extended");
        double interest = accruedInterest(contract, date);
        contract.addPayment({date, interest, 0});
        pledge->handle(Event::extend);
        contract.setPeriod(date, date + policy_->termDays());
        return interest;
    }
    double redeem(Contract& contract, int date) const {
        validateOperationDate(contract, date);
        auto pledge = contract.pledge();
        if (pledge->state() != "Accepted" && pledge->state() != "Extended" && pledge->state() != "Expired")
            throw InvalidTransitionError("Contract cannot be redeemed");
        double interest = accruedInterest(contract, date), fee = penalty(contract, date);
        expireIfNeeded(contract, date);
        contract.addPayment({date, interest, fee});
        pledge->handle(Event::redeem);
        return contract.principal() + interest + fee;
    }
    void checkOverdue(Contract& contract, int date) const {
        validateOperationDate(contract, date);
        expireIfNeeded(contract, date);
    }
    void handle(Pledge& pledge, Event event) const { pledge.handle(event); }
};
