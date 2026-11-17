"""Integration checks of the compiled C++ core through its Python facade."""
import math
import unittest
from datetime import date, timedelta

import pawnshop_core as core
from python_app.service import InMemoryRepository, PawnshopService


class DomainTests(unittest.TestCase):
    def setUp(self):
        self.core = core.LombardCore()
        self.client = core.Client()
        self.client.id = 1
        self.client.fullName, self.client.document, self.client.contact = "Demo", "DEMO", "demo"
        self.start = date(2026, 11, 1)
        self.service = PawnshopService(self.core, InMemoryRepository())

    def params(self, **values):
        result = core.PledgeParams()
        for name, value in values.items():
            setattr(result, name, value)
        return result

    def jewelry(self, **values):
        params = dict(weightG=5.2, fineness=585, pricePerGram=70)
        params.update(values)
        return self.core.createPledge("jewelry", 1, self.client, self.params(**params))

    def contract(self):
        return self.service.openContract("jewelry", self.client,
            self.params(weightG=5.2, fineness=585, pricePerGram=70), 100, self.start)

    def test_appraisal_and_category_caps(self):
        ring = self.jewelry()
        self.assertIsInstance(ring, core.JewelryPledge)
        self.assertAlmostEqual(ring.appraise(), 212.94)
        self.assertAlmostEqual(ring.maxLoan(), 106.47)
        self.assertEqual(self.jewelry(weightG=1000, pricePerGram=7000).maxLoan(), 50000)
        tech = self.core.createPledge("tech", 2, self.client,
            self.params(marketPrice=10000, condition=0.55, wear=0.1))
        self.assertIsInstance(tech, core.TechPledge)
        self.assertAlmostEqual(tech.appraise(), 4950)
        self.assertAlmostEqual(tech.maxLoan(), 1980)
        self.assertEqual(self.core.createPledge("tech", 3, self.client,
            self.params(marketPrice=1000000, condition=0.70)).maxLoan(), 30000)
        vehicle = self.core.createPledge("vehicle", 4, self.client,
            self.params(marketPrice=1000000, annualWear=0.1, ageYears=2))
        self.assertIsInstance(vehicle, core.VehiclePledge)
        self.assertEqual(vehicle.appraise(), 800000)
        self.assertEqual(vehicle.maxLoan(), 300000)
        self.assertEqual(self.core.createPledge("vehicle", 5, self.client,
            self.params(marketPrice=1000000, annualWear=0.1, ageYears=20)).appraise(), 0)

    def test_invalid_inputs_and_loan_limit(self):
        for value in (-1, 0, math.nan, math.inf):
            with self.subTest(weight=value), self.assertRaises(ValueError):
                self.jewelry(weightG=value)
        with self.assertRaises(ValueError):
            self.jewelry(fineness=1001)
        with self.assertRaises(ValueError):
            self.core.createPledge("unknown", 1, self.client, self.params())
        with self.assertRaises(ValueError):
            self.core.createPledge("tech", 1, self.client,
                self.params(marketPrice=100, condition=0.8))
        with self.assertRaises(ValueError):
            self.core.openContract(1, self.jewelry(), 107, self.start.toordinal())

    def test_grace_days_and_penalty_cap(self):
        contract = self.contract()
        day = self.start.toordinal()
        self.assertEqual(self.core.accruedInterest(contract, day + 7), 0)
        self.assertAlmostEqual(self.core.accruedInterest(contract, day + 8), 0.8)
        self.assertEqual(self.core.penalty(contract, day + 30), 0)
        self.assertEqual(self.core.penalty(contract, day + 31), 0.5)
        self.assertEqual(self.core.penalty(contract, day + 230), 100)
        self.assertEqual(self.core.penalty(contract, day + 500), 100)
        with self.assertRaises(ValueError):
            self.core.redemptionAmount(contract, day - 1)

    def test_extension_does_not_subtract_previous_payments(self):
        contract = self.contract()
        self.assertAlmostEqual(self.service.extendContract(contract.id, self.start + timedelta(days=10)), 1)
        self.assertEqual(contract.pledge.state, "Extended")
        self.assertEqual(contract.dueDate, (self.start + timedelta(days=40)).toordinal())
        self.assertAlmostEqual(self.service.extendContract(contract.id, self.start + timedelta(days=20)), 1)
        self.assertAlmostEqual(self.service.redeem(contract.id, self.start + timedelta(days=28)), 100.8)
        self.assertEqual(contract.pledge.state, "Redeemed")
        self.assertEqual(len(contract.payments), 3)
        self.assertAlmostEqual(self.service.incomeForPeriod(self.start, self.start + timedelta(days=28)), 2.8)
        self.assertEqual(self.service.portfolio(), {})
        with self.assertRaises(core.InvalidTransitionError):
            self.service.redeem(contract.id, self.start + timedelta(days=29))
        self.assertEqual(len(contract.payments), 3)

    def test_all_state_event_pairs(self):
        paths = {
            "Accepted": [], "Extended": [core.Event.extend], "Redeemed": [core.Event.redeem],
            "Expired": [core.Event.expire], "ForSale": [core.Event.expire, core.Event.putOnSale],
            "Sold": [core.Event.expire, core.Event.putOnSale, core.Event.sell],
        }
        allowed = {
            "Accepted": {"extend": "Extended", "redeem": "Redeemed", "expire": "Expired"},
            "Extended": {"extend": "Extended", "redeem": "Redeemed", "expire": "Expired"},
            "Expired": {"redeem": "Redeemed", "putOnSale": "ForSale"},
            "ForSale": {"sell": "Sold"}, "Redeemed": {}, "Sold": {},
        }
        for state, path in paths.items():
            for event_name in ("extend", "redeem", "expire", "putOnSale", "sell"):
                with self.subTest(state=state, event=event_name):
                    pledge = self.jewelry()
                    for event in path:
                        self.core.handle(pledge, event)
                    if event_name in allowed[state]:
                        self.core.handle(pledge, getattr(core.Event, event_name))
                        self.assertEqual(pledge.state, allowed[state][event_name])
                    else:
                        with self.assertRaises(core.InvalidTransitionError):
                            self.core.handle(pledge, getattr(core.Event, event_name))
                        self.assertEqual(pledge.state, state)

    def test_overdue_reports_and_redemption(self):
        contract = self.contract()
        overdue = self.start + timedelta(days=31)
        report = self.service.overdueContracts(overdue)
        self.assertEqual(report, [{"contract_id": contract.id, "days": 1, "penalty": 0.5}])
        self.assertEqual(contract.pledge.state, "Accepted")  # Reports do not mutate data.
        self.service.checkOverdue(overdue)
        self.assertEqual(contract.pledge.state, "Expired")
        with self.assertRaises(core.InvalidTransitionError):
            self.service.extendContract(contract.id, overdue)
        self.assertAlmostEqual(self.service.redeem(contract.id, overdue), 103.6)
        self.assertAlmostEqual(self.service.incomeForPeriod(overdue, overdue), 3.6)

    def test_opening_validation_and_identity(self):
        first, second = self.contract(), self.contract()
        self.assertNotEqual(first.id, second.id)
        self.assertEqual(self.service.portfolio()["jewelry"]["count"], 2)
        with self.assertRaises(ValueError):
            self.core.openContract(99, first.pledge, 100, self.start.toordinal())
        available = self.jewelry()
        with self.assertRaises(ValueError):
            self.core.openContract(99, available, 107, self.start.toordinal())
        self.core.openContract(99, available, 100, self.start.toordinal())
        self.client.document = ""
        with self.assertRaises(ValueError):
            self.contract()
        self.assertEqual(len(self.service.repository.all()), 2)

    def test_search_by_combined_fields_without_mutation(self):
        first = self.contract()
        self.client.fullName = "Ірина Шевченко"
        second = self.contract()
        self.service.extendContract(second.id, self.start + timedelta(days=8))
        self.assertEqual([c.id for c in self.service.searchContracts(client_name="шЕвЧ")], [second.id])
        self.assertEqual([c.id for c in self.service.searchContracts(category="jewelry", state="Extended")], [second.id])
        self.assertEqual([c.id for c in self.service.searchContracts(contract_id=first.id)], [first.id])
        self.assertEqual(self.service.searchContracts(contract_id=first.id, state="Extended"), [])
        self.assertEqual(first.pledge.state, "Accepted")


if __name__ == "__main__":
    unittest.main()
