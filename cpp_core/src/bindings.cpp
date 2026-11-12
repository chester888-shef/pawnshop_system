#include <pybind11/pybind11.h>
#include "valuation.hpp"
namespace py = pybind11;

PYBIND11_MODULE(pawnshop_core, m) {
    m.doc() = "Pawnshop C++ core";
    py::class_<IValuationStrategy>(m, "IValuationStrategy")
        .def("appraise", &IValuationStrategy::appraise)
        .def("max_loan", &IValuationStrategy::maxLoan);
    py::class_<JewelryValuation, IValuationStrategy>(m, "JewelryValuation")
        .def(py::init<double, double, double>(),
             py::arg("weight_g"), py::arg("fineness"), py::arg("price_per_gram"));
}