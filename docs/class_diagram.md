# Діаграми класів

Діаграми описують **цільову архітектуру** за [робочою копією SDD](SDD_lombard.md). Позначка «план» означає компонент, який ще не реалізовано. Дані між Python і C++ передаються через pybind11; `LombardCore` — фасад ядра, а `PawnshopService` — фасад для інтерфейсу.

## Предметна область і сценарії

```mermaid
classDiagram
    direction LR
    class Client {
        +int id
        +string fullName
        +string document
        +string contact
    }
    class PledgeParams {
        +double weightG
        +double fineness
        +double pricePerGram
        +double marketPrice
        +double condition
        +double wear
        +int ageYears
        +double annualWear
        +int manufactureYear
        +double mileage
    }
    class Pledge {
        <<abstract>>
        +category() string
        +state() string
        +appraise() double
        +maxLoan() double
        +handle(event)
    }
    class JewelryPledge
    class TechPledge
    class VehiclePledge
    class IValuationStrategy {
        <<interface>>
        +appraise(pledge) double
        +loanLimitRatio() double
        +loanCap() double
    }
    class JewelryValuation
    class TechValuation
    class VehicleValuation
    class IState {
        <<interface>>
        +handle(pledge, event) IState
    }
    class Accepted
    class Extended
    class Redeemed
    class Expired
    class ForSale
    class Sold
    class PledgeCreator {
        <<abstract>>
        +create(id, client, params) Pledge
    }
    class JewelryCreator
    class TechCreator
    class VehicleCreator
    class CollateralFactory {
        +registerCreator(category, creator)
        +create(category, id, client, params) Pledge
    }
    class Contract {
        +int id
        +double principal
        +date issuedOn
        +date periodStart
        +date dueDate
        +Payment[] payments
    }
    class Payment {
        +date date
        +double interest
        +double penalty
    }
    class IInterestPolicy {
        <<interface>>
        +graceDays() int
        +termDays() int
        +dailyRate() double
        +penaltyRate() double
    }
    class StandardInterestPolicy
    class InterestCalculator {
        +accruedInterest(contract, date) double
        +penalty(contract, date) double
        +redemptionAmount(contract, date) double
    }
    class LombardCore {
        <<facade>>
        +createPledge()
        +openContract()
        +extendContract()
        +redeem()
        +checkOverdue()
    }
    class PawnshopService {
        <<facade>>
        +openContract()
        +extendContract()
        +redeem()
        +searchContracts()
        +portfolio()
        +overdueContracts()
        +incomeForPeriod()
    }
    class ContractProcess {
        <<abstract>>
        +run()
        +validate()
        +appraise()
        +issue()
        +save()
    }
    class StandardContractProcess
    class Repository {
        <<interface>>
        +save(contract)
        +get(id)
        +all()
    }
    class InMemoryRepository {
        <<prototype>>
    }
    class SqliteRepository {
        <<planned>>
    }
    class PortfolioReport
    class OverdueReport
    class IncomeReport
    class GUI {
        <<planned>>
    }
    Pledge <|-- JewelryPledge
    Pledge <|-- TechPledge
    Pledge <|-- VehiclePledge
    Pledge *-- Client
    Pledge *-- PledgeParams
    Pledge *-- IValuationStrategy
    Pledge *-- IState
    IValuationStrategy <|.. JewelryValuation
    IValuationStrategy <|.. TechValuation
    IValuationStrategy <|.. VehicleValuation
    IState <|.. Accepted
    IState <|.. Extended
    IState <|.. Redeemed
    IState <|.. Expired
    IState <|.. ForSale
    IState <|.. Sold
    PledgeCreator <|-- JewelryCreator
    PledgeCreator <|-- TechCreator
    PledgeCreator <|-- VehicleCreator
    JewelryCreator ..> JewelryPledge : creates
    TechCreator ..> TechPledge : creates
    VehicleCreator ..> VehiclePledge : creates
    CollateralFactory o-- PledgeCreator
    Contract --> Pledge
    Contract *-- Payment
    InterestCalculator --> IInterestPolicy
    IInterestPolicy <|.. StandardInterestPolicy
    InterestCalculator ..> Contract
    LombardCore *-- CollateralFactory
    LombardCore *-- InterestCalculator
    LombardCore ..> Contract
    GUI ..> PawnshopService
    PawnshopService ..> LombardCore : pybind11
    PawnshopService ..> ContractProcess
    PawnshopService ..> Repository
    PawnshopService ..> PortfolioReport
    PawnshopService ..> OverdueReport
    PawnshopService ..> IncomeReport
    ContractProcess <|-- StandardContractProcess
    StandardContractProcess ..> LombardCore
    StandardContractProcess ..> Repository
    Repository <|-- InMemoryRepository
    Repository <|-- SqliteRepository
    PortfolioReport ..> Repository
    OverdueReport ..> Repository
    IncomeReport ..> Repository
```

## Запланований модуль зберігання minorm

```mermaid
classDiagram
    class SqliteRepository {
        <<planned>>
        +save(contract)
        +get(id)
        +all()
    }
    class Model {
        <<planned>>
        +save()
        +delete()
        +get(id)
        +filter(conditions)
    }
    class Field {
        <<descriptor>>
    }
    class IntField
    class StrField
    class FloatField
    class DateField
    class ForeignKey
    class DataMapper
    class IdentityMap
    class UnitOfWork
    class LazyProxy
    class SQLite {
        <<sqlite3>>
    }
    SqliteRepository ..> Model
    Model *-- Field
    Field <|-- IntField
    Field <|-- StrField
    Field <|-- FloatField
    Field <|-- DateField
    Field <|-- ForeignKey
    Model ..> DataMapper
    DataMapper ..> SQLite
    DataMapper ..> IdentityMap
    UnitOfWork ..> DataMapper
    UnitOfWork ..> SQLite
    ForeignKey ..> LazyProxy
    LazyProxy ..> DataMapper
```

Реалізовані класи та їхні відмінності від майбутніх компонентів описані у [README](../README.md). На схемі не показано кожен метод чи атрибут: деталі реалізації наведено у вихідних файлах.
