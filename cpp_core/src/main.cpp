#include <iomanip>
#include <iostream>
#include "core.hpp"

int main() {
    LombardCore core;
    PledgeParams params;
    params.set_weightG(5.2);
    params.set_fineness(585);
    params.set_pricePerGram(70);
    auto pledge = core.createPledge("jewelry", 1, Client{1, "Demo client", "DEMO", "demo"}, params);
    std::cout << std::fixed << std::setprecision(2)
              << "Appraisal: " << pledge->appraise() << '\n'
              << "Max loan: " << pledge->maxLoan() << '\n';
}
