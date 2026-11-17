"""Reproducible KT2 demo. Run from the repository root."""
import sys
from datetime import date
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent.parent / "cpp_core" / "build"
sys.path.insert(0, str(BUILD_DIR))

import pawnshop_core as core
from service import InMemoryRepository, PawnshopService


def main():
    service = PawnshopService(core.LombardCore(), InMemoryRepository())
    client = core.Client()
    client.id, client.fullName, client.document, client.contact = 1, "Демонстраційний клієнт", "DEMO", "demo"
    params = core.PledgeParams()
    params.weightG, params.fineness, params.pricePerGram = 5.2, 585, 70.0
    contract = service.openContract("jewelry", client, params, 100, date(2026, 11, 1))
    print(f"Оцінка: {contract.pledge.appraise():.2f}")
    print(f"Макс. позика: {contract.pledge.maxLoan():.2f}")
    print(f"Відсотки за продовження: {service.extendContract(contract.id, date(2026, 11, 11)):.2f}")
    print(f"Викуп після продовження: {service.redeem(contract.id, date(2026, 11, 19)):.2f}")
    print(f"Стан: {contract.pledge.state}")
    print(f"Дохід за листопад: {service.incomeForPeriod(date(2026, 11, 1), date(2026, 11, 30)):.2f}")
    try:
        service.extendContract(contract.id, date(2026, 11, 20))
    except core.InvalidTransitionError:
        print("Помилка: викуплену заставу не можна продовжити")


if __name__ == "__main__":
    main()
