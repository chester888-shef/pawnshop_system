#pragma once
#include <memory>
#include <stdexcept>

class IValuationStrategy {
public:
    virtual ~IValuationStrategy() = default;
    virtual double appraise() const = 0;        // оціночна вартість
    virtual double loanLimitRatio() const = 0;  // частка від оцінки, яку можна видати
    double maxLoan() const { return appraise() * loanLimitRatio(); }
};

class JewelryValuation : public IValuationStrategy {
    double weightG_, fineness_, pricePerGram_;
public:
    JewelryValuation(double weightG, double fineness, double pricePerGram)
        : weightG_(weightG), fineness_(fineness), pricePerGram_(pricePerGram) {
        if (weightG <= 0 || fineness <= 0 || fineness > 1000 || pricePerGram <= 0)
            throw std::invalid_argument("Invalid jewelry parameters");
    }
    // проба 585 -> 0.585 чистого металу
    double appraise() const override { return weightG_ * (fineness_ / 1000.0) * pricePerGram_; }
    double loanLimitRatio() const override { return 0.8; }
};