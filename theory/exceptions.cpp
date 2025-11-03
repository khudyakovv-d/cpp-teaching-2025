# Семинар: обработка ошибок и исключения в C/C++

## 1) Ошибки в C: `return`, «поднятие» ошибок наверх

### Инструменты в C

* Коды возврата (`int`/`enum`), обычно `0` — успех.
* **Специальные значения-маркеры**: `NULL`, `-1`, `SIZE_MAX` и т.п.
* Out-параметры через указатели.

### Типовые проблемы

* Повторяющиеся проверки статуса на каждом уровне вызовов.
* Легко пропустить проверку и «проглотить» ошибку.
* Трудно добавлять контекст: где и почему сломалось.
* Ручной `free/close` на всех путях выхода → риск утечек.

### Паттерны

**Несколько ************`return`************ с локальной очисткой:**

```c
#include <stdio.h>
#include <stdlib.h>

int read_config(const char* path, char** out_buf) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;

    char* buf = malloc(1024);
    if (!buf) { fclose(f); return -2; }

    if (!fgets(buf, 1024, f)) {
        free(buf);
        fclose(f);
        return -3;
    }

    fclose(f);
    *out_buf = buf;
    return 0;
}
```

**Единая «сборка» ресурсов и единая функция освобождения:**

```c
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    FILE* f;
    char* buf;
} ctx_t;

static void release(ctx_t* c) {
    if (c->buf) { free(c->buf); c->buf = NULL; }
    if (c->f)   { fclose(c->f); c->f = NULL; }
}

int read_config2(const char* path, char** out_buf) {
    ctx_t c = {0};
    c.f = fopen(path, "r");              if (!c.f)   { release(&c); return -1; }
    c.buf = malloc(1024);                if (!c.buf) { release(&c); return -2; }
    if (!fgets(c.buf, 1024, c.f))        { release(&c); return -3; }

    *out_buf = c.buf;                    // передаём владение
    c.buf = NULL;                        // чтобы release не освободил
    release(&c);                         // закроет файл
    return 0;
}
```

**Единый enum статусов + вывод сообщений:**

```c
typedef enum { ERR_OK=0, ERR_OPEN, ERR_NOMEM, ERR_IO } err_t;

const char* err_str(err_t e) {
    switch (e) {
        case ERR_OK:    return "ok";
        case ERR_OPEN:  return "cannot open";
        case ERR_NOMEM: return "out of memory";
        case ERR_IO:    return "io error";
        default:        return "unknown";
    }
}
```

**«Поднятие» ошибок по стеку вызовов:**

```c
err_t init_app(void) {
    char* cfg = NULL;
    err_t rc = (err_t)read_config("config.ini", &cfg);
    if (rc != ERR_OK) return rc;   // проверка на каждом уровне
    // ... работа ...
    free(cfg);
    return ERR_OK;
}
```

---

## 2) Исключения в C++: механика, конструкции, сравнение с C

### Механика

* `throw` сигнализирует об ошибочной ситуации; управление передаётся к ближайшему подходящему `catch`.
* Сопоставление происходит по типу; срабатывает первый подходящий обработчик.
* При отсутствии обработчика происходит аварийное завершение программы (например, через `std::terminate`).
* Исключения отделяют основной код от обработки ошибок и позволяют реагировать на проблему на подходящем уровне вызовов.

### Сравнение одного кейса (C → C++)

```cpp
// C++
#include <fstream>
#include <stdexcept>
#include <string>

std::string read_config_cpp(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("open failed: " + path);

    std::string line;
    if (!std::getline(f, line))
        throw std::runtime_error("read failed: " + path);

    return line;
}

int main() {
    try {
        auto cfg = read_config_cpp("config.ini");
        // ... работа ...
    } catch (const std::exception& e) {
        // единая точка обработки/логирования/метрик
    }
}
```

### Рекомендации

* Бросать по значению, ловить по `const&`.
* `catch (...)` — только как последний «щит».
* Для повторного проброса текущего исключения — `throw;`.

---

## 3) Иерархия стандартных исключений и порядок `catch`

### Практичный срез

* База: `std::exception` — корневой класс.

**Дерево (упрощённо):**

* `std::logic_error` ← `std::exception`

  * `std::invalid_argument`
  * `std::domain_error`
  * `std::length_error`
  * `std::out_of_range`
  * `std::bad_optional_access` *(C++17)*
  * `std::future_error`
* `std::runtime_error` ← `std::exception`

  * `std::range_error`
  * `std::overflow_error`
  * `std::underflow_error`
  * `std::regex_error`
  * `std::system_error`

    * `std::ios_base::failure` *(C++11+, в ранних стандартах наследование отличалось)*
    * `std::filesystem::filesystem_error` *(C++17)*
* Прямые потомки `std::exception`:

  * `std::bad_alloc`

    * `std::bad_array_new_length` *(C++11)*
  * `std::bad_cast`

    * `std::bad_any_cast` *(C++17)*
  * `std::bad_typeid`
  * `std::bad_function_call`
  * `std::bad_weak_ptr`
  * `std::bad_exception`
  * `std::bad_variant_access` *(C++17)*

*Стрелка «←» читается как «наследуется от».*

### Порядок обработчиков

* Срабатывает первый подходящий по типу.
* Обработчики располагать от более конкретных к более общим.

```cpp
#include <stdexcept>
#include <iostream>

void check_index(int i) {
    if (i < 0)  throw std::invalid_argument("i < 0");   // logic_error
    if (i > 10) throw std::out_of_range("i > 10");      // logic_error -> out_of_range
}

int main() {
    try {
        check_index(42);
    } catch (const std::out_of_range& e) {               // самый узкий
        std::cout << "out_of_range: " << e.what() << '
';
    } catch (const std::logic_error& e) {                // шире (база)
        std::cout << "logic_error: " << e.what() << '
';
    } catch (const std::exception& e) {                  // самый общий
        std::cout << "exception: " << e.what() << '
';
    }
}
```

**Антипример (плохой порядок):**

```cpp
#include <stdexcept>
#include <iostream>

void check_index(int i) {
    if (i < 0)  throw std::invalid_argument("i < 0");
    if (i > 10) throw std::out_of_range("i > 10");
}

int main() {
    try {
        check_index(42);
    } catch (const std::exception& e) {                  // слишком общий — сработает первым
        std::cout << "caught std::exception: " << e.what() << '
';
    } catch (const std::out_of_range& e) {               // недостижимо
        std::cout << "out_of_range: " << e.what() << '
';
    }
}
```

**Замечание:** ловля пользовательских типов по значению приводит к срезанию (object slicing) — использовать `const&`.

---

## 4) Раскрутка стека: гарантия вызова деструкторов и утечки

### Гарантии

* При исключении деструкторы локальных объектов вызываются в обратном порядке.
* Ресурсы на стеке освобождаются автоматически.
* Ресурсы в куче освобождаются только если ими владеет объект, который освобождает их в своём деструкторе.

### Нюансы

* Исключение из деструктора во время раскрутки приводит к `std::terminate()`; деструкторы должны быть «no-throw».
* В конструкторе разрушатся уже сконструированные поля; если объект не создан полностью, его деструктор не вызывается.

### Пример утечки памяти при исключении

```cpp
#include <stdexcept>

struct Payload { int data[256]; };

void leak() {
    Payload* p = new Payload;         // выделено в куче
    // ... работа ...
    throw std::runtime_error("fail"); // до delete не дойдём → утечка
    delete p;
}
```

---

## з5) Тизер: один  пример с `std::unique_ptr`

```cpp
#include <memory>
#include <stdexcept>

struct Payload { int data[256]; };

void process() {
    std::unique_ptr<Payload> p(new Payload); // выделение в куче (явное new) + владение
    // ... работа ...
    if (/* ошибка */) throw std::runtime_error("fail");
    // delete не нужен: память освободится автоматически при выходе из области видимости
}
```

---

## 6) Исключения и конструкторы/деструкторы

### Конструкторы: частично сконструированные объекты

* Если конструктор бросает, объект считается **не сконструированным**, его деструктор **не вызывается**.
* Уже успевшие сконструироваться поля/базы будут разрушены автоматически (в обратном порядке).
* Поэтому ресурсы, которые захватываются в конструкторе, должны храниться в полях‑владельцах (объектах, которые освобождают ресурс в деструкторе); иначе возможны утечки.

**Плохой вариант (утечка при броске):**

```cpp
#include <stdexcept>

struct Widget {
    int* data; // сырое владение
    Widget(std::size_t n) : data(new int[n]) { // выделили
        if (n > 1'000'000) {
            throw std::runtime_error("too big"); // до delete не дойдём → утечка
        }
    }
};
```

**Хороший вариант (поле‑владелец):**

```cpp
#include <vector>
#include <stdexcept>

struct Widget {
    std::vector<int> data; // владеет памятью
    Widget(std::size_t n) : data(n) {
        if (n > 1'000'000) {
            throw std::runtime_error("too big"); // vector сам освободит ресурсы
        }
    }
};
```

### Порядок конструирования/разрушения

* Конструирование: **сначала базы**, затем поля **в порядке объявления** в классе.
* Разрушение: в обратном порядке — **сначала поля (обратно объявлению)**, затем база.
* Опирайтесь на этот порядок при владении ресурсами и зависимости между полями.

### Деструкторы и исключения

* Деструкторы должны быть фактически **no‑throw**. Исключение из деструктора во время раскрутки приводит к `std::terminate()`.
* Начиная с C++11, деструкторы по умолчанию имеют вычисляемую спецификацию исключений: они `noexcept(true)`, если все члены/базы уничтожаются без бросков; иначе — потенциально бросающие. На практике — не бросайте из деструкторов.

**Шаблон безопасной очистки:**

```cpp
struct Guard {
    ~Guard() noexcept {
        try {
            // освободить/закрыть; если что‑то может бросить — перехватить
        } catch (...) { /* лог/счётчик, но не пробрасывать */ }
    }
};
```

---

## 7) Минусы исключений и когда они не подходят

### Почему это может нарушать Open/Closed Principle (OCP)

* **Появляется новый тип ошибки → приходится менять существующий код.**
  Обработчики завязаны на конкретные типы (`catch (const X&)`). Когда добавляем новый тип ошибки с особой реакцией, нужно править уже написанные блоки `catch` (и их порядок), вместо того чтобы просто «расширить систему» без модификаций.
* **Нелокальность последствий.** Исключение «проскакивает» через слои. Добавив бросок глубоко внизу, нередко приходится пересматривать гарантии исключений и обработчики на верхних уровнях.

**Мини‑пример (нарушение OCP):**

```cpp
#include <stdexcept>
#include <iostream>

// Было: любые проблемы конфигурации считали runtime_error
std::string load_config();

int main() {
    try {
        auto cfg = load_config();
        std::cout << cfg << "
";
    } catch (const std::runtime_error& e) { // общий путь
        std::cerr << "recover: " << e.what() << "
"; // делаем общий фоллбэк
    }
}
```

Позже появляется новый сценарий, который **требует другой реакции** — например, «файл найден, но формат битый», и нужно предложить пользователю выбрать другой файл.

```cpp
struct ConfigFormatError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

std::string load_config() {
    // ... обнаружили битый формат
    throw ConfigFormatError{"invalid config format"};
}

int main() {
    try {
        auto cfg = load_config();
    } catch (const ConfigFormatError& e) {          // НУЖНО ДОБАВИТЬ (изменение старого кода)
        std::cerr << "ask user to re-pick file: " << e.what() << "
";
    } catch (const std::runtime_error& e) {         // остаётся общий обработчик
        std::cerr << "recover: " << e.what() << "
";
    }
}
```

Чтобы ввести новую логику для `ConfigFormatError`, мы вынуждены **менять существующий обработчик** (и порядок `catch`) — это и есть нарушение OCP.

### Другие минусы и ограничения

* **Скрытый поток управления.** Сложнее читать и анализировать: бросок в одной точке переносит управление далеко вверх.
* **Стоимость при броске.** «Счастливый путь» обычно дешёвый (zero‑cost модель), но сам бросок и раскрутка стека стоят дороже, чем возврат значения.
* **Требования к безопасностям.** Нужно продумывать basic/strong/no‑throw гарантии, особенно при изменении контейнеров и транзакционных обновлениях.
* **Границы ************`noexcept`************.** Изменение семантики (например, сняли `noexcept`) — ломающее API решение; оставили `noexcept`, но пробросили исключение — `std::terminate()`.
* **Плохая совместимость с некоторыми средами.** В низкоуровневом/реал‑тайм/embedded коде исключения могут быть запрещены или нежелательны из‑за стоимости и непредсказуемости путей.
* **Исключения из деструкторов.** Во время раскрутки приводят к `std::terminate()`, поэтому деструкторы должны быть «no‑throw».
* **Интероп с C‑API.** C‑функции не понимают исключений; границы нужно оборачивать и преобразовывать в коды ошибок.

**Когда рассмотреть альтернативы:**

* Ожидаемые, частые ошибки, которые являются частью бизнес‑логики (валидация ввода, «нет данных») — подойдут **коды возврата/результаты** или `std::expected` (C++23) / `tl::expected`.
* Низкоуровневый/RT‑код, критичные секции, библиотеки со стабильным ABI — предпочесть безисключённый стиль.

---

## Дополнительно (коротко)

* Производительность исключений: «zero-cost» в обычном пути; стоимость — при броске и обработке.
* `noexcept`: исключение, «пролетевшее» через границу `noexcept(true)`, вызывает `std::terminate()`.
* Гарантии исключений: basic / strong / no-throw; связь с проектированием контейнеров и транзакционными изменениями.

---

## Краткое резюме

* В C ошибки — коды возврата; много шаблонной рутины и ручного управления ресурсами.
* В C++ исключения дают централизованную обработку, а деструкторы обеспечивают автоматический cleanup при раскрутке стека.
* Порядок `catch`: от частного к общему.
* Чтобы исключения не приводили к утечкам, ресурсы должны быть обёрнуты в объекты с деструктором; минимальная практика — `std::unique_ptr`.