#include "pledge.hpp"

double JewelryValuation::appraise(const Pledge& pledge) const {
    const auto& p = pledge.params();
    positive(p.weightG(), "Invalid jewelry weight");
    positive(p.fineness(), "Invalid jewelry fineness");
    positive(p.pricePerGram(), "Invalid metal price");
    if (p.fineness() > 1000) throw std::invalid_argument("Fineness exceeds 1000");
    const double value = p.weightG() * (p.fineness() / 1000.0) * p.pricePerGram();
    positive(value, "Invalid appraisal result");
    return value;
}
double TechValuation::appraise(const Pledge& pledge) const {
    const auto& p = pledge.params();
    positive(p.marketPrice(), "Invalid market price");
    fraction(p.wear(), "Invalid wear");
    if (p.condition() != 0.70 && p.condition() != 0.55 && p.condition() != 0.40 && p.condition() != 0.20)
        throw std::invalid_argument("Condition must be 0.70, 0.55, 0.40 or 0.20");
    if (p.ageYears() < 0) throw std::invalid_argument("Negative age");
    const double value = p.marketPrice() * p.condition() * (1 - p.wear());
    if (!std::isfinite(value)) throw std::invalid_argument("Invalid appraisal result");
    return value;
}
double VehicleValuation::appraise(const Pledge& pledge) const {
    const auto& p = pledge.params();
    positive(p.marketPrice(), "Invalid market price");
    fraction(p.annualWear(), "Invalid annual wear");
    if (p.ageYears() < 0 || !std::isfinite(p.mileage()) || p.mileage() < 0 || p.manufactureYear() < 0)
        throw std::invalid_argument("Invalid vehicle parameters");
    return p.marketPrice() * std::max(0.0, 1 - p.annualWear() * p.ageYears());
}
namespace {
std::unique_ptr<IState> activeTransition(Event event) {
    switch (event) {
    case Event::extend: return std::make_unique<Extended>();
    case Event::redeem: return std::make_unique<Redeemed>();
    case Event::expire: return std::make_unique<Expired>();
    default: throw InvalidTransitionError("Event is not allowed for an active pledge");
    }
}
}
std::unique_ptr<IState> Accepted::handle(Pledge&, Event event) const { return activeTransition(event); }
std::unique_ptr<IState> Extended::handle(Pledge&, Event event) const { return activeTransition(event); }
std::unique_ptr<IState> Expired::handle(Pledge&, Event event) const {
    if (event == Event::redeem) return std::make_unique<Redeemed>();
    if (event == Event::putOnSale) return std::make_unique<ForSale>();
    throw InvalidTransitionError("Event is not allowed for an expired pledge");
}
std::unique_ptr<IState> ForSale::handle(Pledge&, Event event) const {
    if (event == Event::sell) return std::make_unique<Sold>();
    throw InvalidTransitionError("Only sale is allowed for a pledge on sale");
}
std::unique_ptr<IState> Redeemed::handle(Pledge&, Event) const { throw InvalidTransitionError("Redeemed is terminal"); }
std::unique_ptr<IState> Sold::handle(Pledge&, Event) const { throw InvalidTransitionError("Sold is terminal"); }
