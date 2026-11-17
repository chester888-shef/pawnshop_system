#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "core.hpp"
namespace py = pybind11;

PYBIND11_MODULE(pawnshop_core, m) {
    m.doc() = "lombard_core: pawnshop domain facade (C++17)";
    py::register_exception<InvalidTransitionError>(m, "InvalidTransitionError");
    py::enum_<Event>(m, "Event")
        .value("extend", Event::extend).value("redeem", Event::redeem)
        .value("expire", Event::expire).value("putOnSale", Event::putOnSale).value("sell", Event::sell);
    py::class_<Client>(m, "Client").def(py::init<>())
        .def_property("id", &Client::id, &Client::set_id).def_property("fullName", &Client::fullName, &Client::set_fullName)
        .def_property("document", &Client::document, &Client::set_document).def_property("contact", &Client::contact, &Client::set_contact);
    py::class_<PledgeParams>(m, "PledgeParams").def(py::init<>())
        .def_property("weightG", &PledgeParams::weightG, &PledgeParams::set_weightG).def_property("fineness", &PledgeParams::fineness, &PledgeParams::set_fineness)
        .def_property("pricePerGram", &PledgeParams::pricePerGram, &PledgeParams::set_pricePerGram).def_property("marketPrice", &PledgeParams::marketPrice, &PledgeParams::set_marketPrice)
        .def_property("condition", &PledgeParams::condition, &PledgeParams::set_condition).def_property("wear", &PledgeParams::wear, &PledgeParams::set_wear)
        .def_property("ageYears", &PledgeParams::ageYears, &PledgeParams::set_ageYears).def_property("annualWear", &PledgeParams::annualWear, &PledgeParams::set_annualWear)
        .def_property("manufactureYear", &PledgeParams::manufactureYear, &PledgeParams::set_manufactureYear).def_property("mileage", &PledgeParams::mileage, &PledgeParams::set_mileage);
    py::class_<Pledge, std::shared_ptr<Pledge>>(m, "Pledge")
        .def_property_readonly("id", &Pledge::id).def_property_readonly("client", &Pledge::client)
        .def_property_readonly("category", &Pledge::category).def_property_readonly("state", &Pledge::state)
        .def("appraise", &Pledge::appraise).def("maxLoan", &Pledge::maxLoan);
    py::class_<JewelryPledge, Pledge, std::shared_ptr<JewelryPledge>>(m, "JewelryPledge");
    py::class_<TechPledge, Pledge, std::shared_ptr<TechPledge>>(m, "TechPledge");
    py::class_<VehiclePledge, Pledge, std::shared_ptr<VehiclePledge>>(m, "VehiclePledge");
    py::class_<Payment>(m, "Payment")
        .def_property_readonly("date", &Payment::date).def_property_readonly("interest", &Payment::interest).def_property_readonly("penalty", &Payment::penalty);
    py::class_<Contract, std::shared_ptr<Contract>>(m, "Contract")
        .def_property_readonly("id", &Contract::id).def_property_readonly("pledge", &Contract::pledge)
        .def_property_readonly("principal", &Contract::principal).def_property_readonly("issuedOn", &Contract::issuedOn)
        .def_property_readonly("periodStart", &Contract::periodStart).def_property_readonly("dueDate", &Contract::dueDate)
        .def_property_readonly("payments", &Contract::payments);
    py::class_<LombardCore>(m, "LombardCore").def(py::init<>())
        .def("createPledge", &LombardCore::createPledge)
        .def("openContract", &LombardCore::openContract)
        .def("accruedInterest", &LombardCore::accruedInterest).def("penalty", &LombardCore::penalty)
        .def("redemptionAmount", &LombardCore::redemptionAmount)
        .def("extendContract", &LombardCore::extendContract).def("redeem", &LombardCore::redeem)
        .def("checkOverdue", &LombardCore::checkOverdue).def("handle", &LombardCore::handle);
}
