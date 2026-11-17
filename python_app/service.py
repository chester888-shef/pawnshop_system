from abc import ABC, abstractmethod
from datetime import date


class Repository(ABC):
    @abstractmethod
    def save(self, contract):
        pass

    @abstractmethod
    def get(self, contract_id):
        pass

    @abstractmethod
    def all(self):
        pass


class InMemoryRepository(Repository):
    def __init__(self):
        self._contracts = {}

    def save(self, contract):
        self._contracts[contract.id] = contract

    def get(self, contract_id):
        return self._contracts[contract_id]

    def all(self):
        return list(self._contracts.values())


class ContractProcess(ABC):
    def run(self, category, client, params, principal, opened_on):
        self.validate(client, opened_on)
        pledge = self.appraise(category, client, params)
        contract = self.issue(pledge, principal, opened_on)
        self.save(contract)
        return contract

    @abstractmethod
    def validate(self, client, opened_on):
        pass

    @abstractmethod
    def appraise(self, category, client, params):
        pass

    @abstractmethod
    def issue(self, pledge, principal, opened_on):
        pass

    @abstractmethod
    def save(self, contract):
        pass


class StandardContractProcess(ContractProcess):
    def __init__(self, core, repository):
        self.core, self.repository = core, repository
        self._next_id = max((c.id for c in repository.all()), default=0) + 1

    def validate(self, client, opened_on):
        if not isinstance(opened_on, date):
            raise TypeError("Expected datetime.date")
        if client.id <= 0 or not all((client.fullName.strip(), client.document.strip(), client.contact.strip())):
            raise ValueError("Client ID, name, document and contact are required")

    def appraise(self, category, client, params):
        return self.core.createPledge(category, self._next_id, client, params)

    def issue(self, pledge, principal, opened_on):
        return self.core.openContract(self._next_id, pledge, principal, opened_on.toordinal())

    def save(self, contract):
        self.repository.save(contract)
        self._next_id += 1


class PortfolioReport:
    def __init__(self, repository):
        self.repository = repository

    def build(self):
        result = {}
        for contract in self.repository.all():
            pledge = contract.pledge
            if pledge.state in {"Redeemed", "Sold"}:
                continue
            group = result.setdefault(pledge.category, {"count": 0, "principal": 0.0, "appraisal": 0.0})
            group["count"] += 1
            group["principal"] += contract.principal
            group["appraisal"] += pledge.appraise()
        return result


class OverdueReport:
    def __init__(self, repository, core):
        self.repository, self.core = repository, core

    def build(self, as_of):
        day = as_of.toordinal()
        return [
            {"contract_id": c.id, "days": day - c.dueDate, "penalty": self.core.penalty(c, day)}
            for c in self.repository.all()
            if c.pledge.state in {"Accepted", "Extended", "Expired"} and day > c.dueDate
        ]


class IncomeReport:
    def __init__(self, repository):
        self.repository = repository

    def build(self, start, end):
        if end < start:
            raise ValueError("Invalid reporting period")
        return sum(
            p.interest + p.penalty
            for c in self.repository.all() for p in c.payments
            if start.toordinal() <= p.date <= end.toordinal()
        )


class PawnshopService:
    def __init__(self, core, repository):
        self.core, self.repository = core, repository
        self.process = StandardContractProcess(core, repository)

    def openContract(self, category, client, params, principal, opened_on):
        return self.process.run(category, client, params, principal, opened_on)

    def extendContract(self, contract_id, as_of):
        contract = self.repository.get(contract_id)
        amount = self.core.extendContract(contract, as_of.toordinal())
        self.repository.save(contract)
        return amount

    def redeem(self, contract_id, as_of):
        contract = self.repository.get(contract_id)
        amount = self.core.redeem(contract, as_of.toordinal())
        self.repository.save(contract)
        return amount

    def checkOverdue(self, as_of):
        for contract in self.repository.all():
            if contract.pledge.state in {"Accepted", "Extended"}:
                self.core.checkOverdue(contract, as_of.toordinal())
                self.repository.save(contract)

    def portfolio(self):
        return PortfolioReport(self.repository).build()

    def searchContracts(self, *, contract_id=None, client_name=None, category=None, state=None):
        """Find contracts using combined exact filters and a case-insensitive name substring."""
        name = client_name.strip().casefold() if client_name is not None else None
        return [contract for contract in self.repository.all()
                if (contract_id is None or contract.id == contract_id)
                and (name is None or name in contract.pledge.client.fullName.casefold())
                and (category is None or contract.pledge.category == category)
                and (state is None or contract.pledge.state == state)]

    def overdueContracts(self, as_of):
        return OverdueReport(self.repository, self.core).build(as_of)

    def incomeForPeriod(self, start, end):
        return IncomeReport(self.repository).build(start, end)
