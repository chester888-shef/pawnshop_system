import sys
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent.parent / "cpp_core" / "build"
sys.path.insert(0, str(BUILD_DIR))

import pawnshop_core as core

ring = core.JewelryValuation(weight_g=5.2, fineness=585, price_per_gram=70.0)
print("Оцінка:", ring.appraise())
print("Макс. позика:", ring.max_loan())

try:
    core.JewelryValuation(-1, 585, 70)
except ValueError as e:
    print("Помилка:", e)