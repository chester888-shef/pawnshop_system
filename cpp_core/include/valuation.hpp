#pragma once
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

inline void positive(double value, const char* name) {
    if (!std::isfinite(value) || value <= 0) throw std::invalid_argument(name);
}
inline void fraction(double value, const char* name) {
    if (!std::isfinite(value) || value < 0 || value > 1) throw std::invalid_argument(name);
}
class PledgeParams {
    double weightG_ = 0;
    double fineness_ = 0;
    double pricePerGram_ = 0;
    double marketPrice_ = 0;
    double condition_ = 0;
    double wear_ = 0;
    int ageYears_ = 0;
    double annualWear_ = 0;
    int manufactureYear_ = 0;
    double mileage_ = 0;
public:
    double weightG() const { return weightG_; }
    void set_weightG(double value) { weightG_ = value; }
    double fineness() const { return fineness_; }
    void set_fineness(double value) { fineness_ = value; }
    double pricePerGram() const { return pricePerGram_; }
    void set_pricePerGram(double value) { pricePerGram_ = value; }
    double marketPrice() const { return marketPrice_; }
    void set_marketPrice(double value) { marketPrice_ = value; }
    double condition() const { return condition_; }
    void set_condition(double value) { condition_ = value; }
    double wear() const { return wear_; }
    void set_wear(double value) { wear_ = value; }
    int ageYears() const { return ageYears_; }
    void set_ageYears(int value) { ageYears_ = value; }
    double annualWear() const { return annualWear_; }
    void set_annualWear(double value) { annualWear_ = value; }
    int manufactureYear() const { return manufactureYear_; }
    void set_manufactureYear(int value) { manufactureYear_ = value; }
    double mileage() const { return mileage_; }
    void set_mileage(double value) { mileage_ = value; }
};
class Pledge;
class IValuationStrategy {
public:
    virtual ~IValuationStrategy() = default;
    virtual double appraise(const Pledge&) const = 0;
    virtual double loanLimitRatio() const = 0;
    virtual double loanCap() const = 0;
};
class JewelryValuation : public IValuationStrategy {
public:
    double appraise(const Pledge&) const override;
    double loanLimitRatio() const override { return 0.50; }
    double loanCap() const override { return 50000; }
};
class TechValuation : public IValuationStrategy {
public:
    double appraise(const Pledge&) const override;
    double loanLimitRatio() const override { return 0.40; }
    double loanCap() const override { return 30000; }
};
class VehicleValuation : public IValuationStrategy {
public:
    double appraise(const Pledge&) const override;
    double loanLimitRatio() const override { return 0.40; }
    double loanCap() const override { return 300000; }
};
