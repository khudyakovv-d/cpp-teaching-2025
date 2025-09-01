

## План занятия
1. Жизненный цикл программы  
2. Компиляция  
3. Линковка  
4. Что такое CMake  
5. Что такое CMakeLists.txt  
6. CMake Presets (CMakePresets.json)  
7. Цели (targets) в CMake  
8. Ключевые инструкции CMake  
9. Минимальный проект на CMake  



## Жизненный цикл программы

- Исходный код: `.cpp`, заголовки `.h/.hpp`  
- Компиляция: каждый `.cpp` → объектный файл (`.o/.obj`)  
- Линковка: объектники + библиотеки → исполняемый файл или библиотека  
- Исполнение: бинарник загружается ОС, стартует `main()`  
- Артефакты: Linux — `app/.so/.a`, Windows — `.exe/.dll/.lib`, macOS — `.app/.dylib/.a`



## Компиляция

- Конвейер: препроцессор → компилятор → ассемблер  
- Работает по единице трансляции: один `.cpp` + подключённые заголовки  
- На выходе: объектный файл с машинным кодом и таблицей символов

**Заголовки и include‑пути**  
- Заголовки содержат интерфейсы (объявления), исходники — реализации  
- Локальные `"header.hpp"` и системные `<vector>`  
- Пути настраиваются на цели:  
```cmake
target_include_directories(app PRIVATE include)
```

**Пример (CLI)**  
```bash
# Один файл
g++ main.cpp -o app
./app

# Несколько файлов
g++ -c main.cpp   # → main.o
g++ -c math.cpp   # → math.o
g++ main.o math.o -o app
./app
```



## Линковка

- Складывает объектные файлы и библиотеки в один артефакт  
- Соединяет вызовы функций с их реализациями (разрешает символы)  
- Результат: программа или библиотека (static/shared)

**Статические и динамические библиотеки**  
- STATIC (`.a/.lib`): код встраивается в итоговый бинарник  
  + проще запуск, − больше размер  
- SHARED (`.so/.dll/.dylib`): библиотека отдельным файлом  
  + делится между программами, − должна быть доступна при запуске

**Пример (STATIC/SHARED)**  
```bash
# Статическая
g++ -c hello.cpp && ar rcs libhello.a hello.o
g++ -c main.cpp  && g++ main.o -L. -lhello -o app && ./app

# Динамическая
g++ -fPIC -shared hello.cpp -o libhello.so
g++ main.cpp -L. -lhello -o app
LD_LIBRARY_PATH=. ./app
```



## Что такое CMake

- Кроссплатформенная система сборки  
- Генерирует файлы для разных инструментов: Make, Ninja, Visual Studio, Xcode  
- Описывает проект декларативно: какие есть цели и как они связаны  
- Позволяет воспроизводимо собирать проект на разных машинах



## Что такое CMakeLists.txt

- Главный файл конфигурации CMake в проекте  
- Содержит инструкции:  
  - минимальная версия (`cmake_minimum_required`)  
  - параметры проекта (`project`)  
  - цели (`add_executable`, `add_library`)  
  - зависимости (`target_link_libraries`)  
- Обычно есть один основной `CMakeLists.txt` + вложенные в подпроектах

**Пример простого CMakeLists.txt**  
```cmake
cmake_minimum_required(VERSION 3.20)
project(demo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(app main.cpp)
```



## CMake Presets (CMakePresets.json)

- Единая конфигурация/сборка на всех машинах и CI  
- Избавляют от длинных CLI‑флагов; хранятся в репозитории  
- Типы: configure, build, test пресеты  
- Файлы: `CMakePresets.json` (в проекте), `CMakeUserPresets.json` (локальные настройки)

**Как использовать**  
```bash
cmake --list-presets
cmake --preset debug
cmake --build --preset debug
```

**Пример CMakePresets.json**  
```json
{
  "version": 6,
  "configurePresets": [
    {
        "name": "make",
        "displayName": "Makefiles Build",
        "generator": "Unix Makefiles",
        "binaryDir": "${sourceDir}/build",
        "cacheVariables": {
          "CMAKE_BUILD_TYPE": "Debug"
        }
    }
  ],
  "buildPresets": [
    { "name": "debug",   "configurePreset": "debug" }
  ]
}
```



## Цели (targets) в CMake

- Цель — именованная сущность сборки: executable, library, interface  
- У цели есть свойства: include‑пути, флаги, зависимости  
- Цели образуют граф; CMake распространяет свойства по зависимостям

**Видимость свойств**  
- PRIVATE — только для текущей цели  
- PUBLIC — для цели и её потребителей  
- INTERFACE — только для потребителей

**Мини‑пример**  
```cmake
add_library(math STATIC math.cpp)
target_include_directories(math PUBLIC include)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE math)
```
- `app` зависит от `math`; PUBLIC‑заголовки `math` доступны в `app`



## Ключевые инструкции CMake — add_executable

```cmake
add_executable(app src/main.cpp)
```
**Что делает:** создаёт исполняемую цель (бинарник).  
**Зачем:** точка входа приложения.  
**Нюанс:** можно перечислять несколько исходников и каталоги.



## Ключевые инструкции CMake — add_library

```cmake
add_library(hello STATIC src/hello.cpp)
```
**Что делает:** создаёт библиотеку нужного типа.  
**Зачем:** модульность, переиспользование кода.  
**Нюанс:** тип влияет на способ распространения и запуск.



## Ключевые инструкции CMake — target_include_directories

```cmake
target_include_directories(app PRIVATE include)
# для библиотеки, чтобы потребители видели заголовки:
target_include_directories(hello PUBLIC include)
```
**Что делает:** добавляет пути поиска заголовков (`-I`).  
**Зачем:** разделение интерфейса и реализации.  
**Нюанс:** корректно подбирайте PUBLIC/PRIVATE/INTERFACE.



## Ключевые инструкции CMake — target_link_libraries

```cmake
target_link_libraries(app PRIVATE hello)
```
**Что делает:** связывает цели и передаёт их свойства.  
**Зачем:** подключение библиотек и зависимостей.  
**Нюанс:** видимость влияет на наследование зависимостей.



## Ключевые инструкции CMake — add_subdirectory

```cmake
add_subdirectory(libs/hello)
```
**Что делает:** подключает подпроект с его `CMakeLists.txt`.  
**Зачем:** модульная структура, совместная сборка библиотек и приложения.  
**Нюанс:** путь относителен текущего `CMakeLists.txt`.



## Минимальный проект на CMake

**Структура проекта**  
```
project/
  src/main.cpp
  libs/hello/ (src/*.cpp, include/*.hpp, CMakeLists.txt)
  CMakeLists.txt
```

**CMakeLists.txt**  
```cmake
cmake_minimum_required(VERSION 3.20)
project(demo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/hello)

add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE hello)
```

**Сборка**  
```bash
# без пресетов
mkdir build && cd build
cmake ..
cmake --build .
./app

# с пресетами
cmake --preset debug
cmake --build --preset debug
./build/debug/app
```
