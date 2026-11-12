# Програмне рішення для організації роботи ломбарду

Курсова робота з дисципліни «Об'єктно-орієнтоване програмування».

- **Студент:** Федунь Максим
- **Група:** ОІ-23
- **Керівник:** ас. Черкас Н. С., кафедра АСУ, НУ «Львівська політехніка»

## Опис

Програмна система для роботи ломбарду. Ядро обчислень реалізовано на C++, застосунок і звіти на Python. Зв'язок між мовами здійснюється через pybind11.

## Структура проєкту

```
pawnshop_system/
├── cpp_core/
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── valuation.hpp     # стратегії оцінки застав
│   └── src/
│       ├── bindings.cpp      # міст C++ → Python (pybind11)
│       └── main.cpp          # тестова консольна програма
├── python_app/
│   └── main.py               # точка входу
├── docs/
│   ├── class_diagram.md      # діаграма класів
│   └── patterns.md           # патерни та обґрунтування
├── .gitignore
└── README.md
```

## Вимоги

- Ubuntu / Linux Mint
- GCC з підтримкою C++17
- CMake 3.15+
- Python 3.9+ з заголовками розробника

```bash
sudo apt update
sudo apt install -y build-essential cmake git python3 python3-venv python3-dev python3-pip
```

## Збірка і запуск

Усі команди виконуються з кореня репозиторію.

```bash
# 1. Віртуальне середовище і pybind11
cd python_app
python3 -m venv venv
source venv/bin/activate
pip install pybind11
cd ..

# 2. Збірка C++ модуля
cmake -S cpp_core -B cpp_core/build -Dpybind11_DIR=$(python -m pybind11 --cmakedir) -Wno-dev
cmake --build cpp_core/build

# 3. Запуск
python python_app/main.py
```

Очікуваний результат:

```
Оцінка: 212.94
Макс. позика: 170.352
Помилка: Invalid jewelry parameters
```

Якщо збірка зламалася, видали `cpp_core/build` (`rm -rf cpp_core/build`) і повтори крок 2.

## Поточний стан

- КТ1: створено репозиторій і каркас проєкту.
- КТ2: діаграма класів, обґрунтування патернів (Strategy, State, Factory Method, Facade), працюючий міст C++ ↔ Python.
- Реалізовано оцінку ювелірних виробів за пробою та масою і розрахунок максимальної позики.

## Автор

Федунь Максим, ОІ-23, 2026.