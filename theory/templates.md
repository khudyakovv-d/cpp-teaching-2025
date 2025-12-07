# Семинар: Шаблоны в C++

## План

1. Шаблоны функций  
2. Шаблоны классов  
3. Как компилятор инстанцирует шаблоны  
4. Специализация шаблонов  
5. Рекурсивные шаблоны и частичная специализация  
6. Автоматический вывод типов шаблонов: детали  
7. Отличия шаблонов функций и шаблонов классов  
8. Шаблоны с переменным числом аргументов  
9. `std::tuple` как пример вариадического шаблона  

---

## 1. Шаблоны функций

### 1.1. Мотивация

Три очень похожие функции:

```cpp
int max_int(int a, int b) {
    return (a < b) ? b : a;
}

double max_double(double a, double b) {
    return (a < b) ? b : a;
}

std::string max_string(const std::string& a, const std::string& b) {
    return (a < b) ? b : a;
}
```

Код один и тот же, меняется только тип → хочется **одну** реализацию.

### 1.2. Базовый синтаксис

```cpp
template <typename T>        // или template <class T>
T max_value(T a, T b) {
    return (a < b) ? b : a;
}

int main() {
    int    a = max_value(1, 2);          // T = int
    double b = max_value(1.5, 2.3);      // T = double
}
```

- `template <typename T>` — объявление параметра шаблона;
- при вызове `max_value(1, 2)` компилятор подбирает `T = int`;
- шаблон функции сам по себе не функция, это «заготовка».

### 1.3. Вывод параметров шаблона

```cpp
template <typename T>
T add(T a, T b) {
    return a + b;
}

int main() {
    int x = add(1, 2);          // T выведен как int
    auto y = add(1.0, 2.5);     // T выведен как double

    auto z = add<double>(1, 2.5); // Явно: T = double
}
```

- обычно тип `T` выводится автоматически из аргументов;
- тип можно указать явно: `add<int>(...)`.

### 1.4. Перегрузка шаблонов и обычных функций

```cpp
void print(int x) {
    std::cout << "int: " << x << '\n';
}

template <typename T>
void print(const T& value) {
    std::cout << "T: " << value << '\n';
}

int main() {
    print(10);     // выбрана не-шаблонная версия
    print(3.14);   // выбрана шаблонная версия
}
```

- обычная (не-шаблонная) функция выигрывает, если подходит по типам не хуже шаблонной;
- можно иметь несколько шаблонов с разными параметрами → перегрузка.

---

## 2. Шаблоны классов

### 2.1. Базовый пример

```cpp
template <typename T>
class Box {
public:
    explicit Box(const T& value) : value_(value) {}

    const T& get() const { return value_; }
    void set(const T& v) { value_ = v; }

private:
    T value_;
};

int main() {
    Box<int>    bi(10);    // Box<int>
    Box<double> bd(3.14);  // Box<double>

    std::cout << bi.get() << '\n';
}
```

- `Box<int>` и `Box<double>` — два разных типа;
- шаблон класса задаёт целое семейство типов.

### 2.2. Определения методов вне класса

```cpp
template <typename T>
class Box {
public:
    Box(const T& value);
    const T& get() const;

private:
    T value_;
};

template <typename T>
Box<T>::Box(const T& value) : value_(value) {}

template <typename T>
const T& Box<T>::get() const {
    return value_;
}
```

- перед каждым определением снаружи писать `template <typename T>`;
- использовать `Box<T>`, а не просто `Box`.

### 2.3. Нетиповые параметры шаблонов

```cpp
template <typename T, std::size_t N>
class StaticArray {
public:
    T& operator[](std::size_t i) {
        return data_[i];
    }

    const T& operator[](std::size_t i) const {
        return data_[i];
    }

    std::size_t size() const { return N; }

private:
    T data_[N];
};

int main() {
    StaticArray<int, 10> a;     // 10 int
    StaticArray<double, 5> b;   // 5 double
}
```

- типовые параметры: `typename T`;
- нетиповые: `std::size_t N`, `int N` и т.п.

`StaticArray<int, 10>` и `StaticArray<int, 20>` — разные типы.

### 2.4. Параметры по умолчанию у шаблонов

У параметров шаблона можно задавать значения по умолчанию, как у аргументов функций.

Пример: шаблон пары, где второй тип по умолчанию совпадает с первым:

```cpp
template <typename T, typename U = T>
class Pair {
public:
    Pair(const T& first, const U& second)
        : first_(first), second_(second) {}

    const T& first() const  { return first_; }
    const U& second() const { return second_; }

private:
    T first_;
    U second_;
};

int main() {
    Pair<int>         p1(1, 2);       // Pair<int, int>
    Pair<int, double> p2(1, 2.5);     // Pair<int, double>
}
```

- в объявлении `template <typename T, typename U = T>` у параметра `U` задано значение по умолчанию — `T`;
- `Pair<int>` разворачивается в `Pair<int, int>`;
- при необходимости можно явно указать второй тип: `Pair<int, double>`.

---

## 3. Как компилятор инстанцирует шаблоны

### 3.1. Шаблон как «чертёж»

```cpp
template <typename T>
T max_value(T a, T b) {
    return (a < b) ? b : a;
}

int main() {
    max_value(1, 2);        // нужна версия для int
    max_value(1.0, 2.0);    // нужна версия для double
}
```

Логика примерно такая:

```cpp
int max_value(int a, int b) {
    return (a < b) ? b : a;
}

double max_value(double a, double b) {
    return (a < b) ? b : a;
}
```

Для каждой комбинации параметров шаблона компилятор порождает свою реализацию.

---

### 3.2. Когда именно происходит инстанцирование

Инстанцирование происходит, когда шаблон **используется**:

- вызов шаблонной функции;
- создание объекта шаблонного класса;
- обращение к статическому методу и т.д.

```cpp
template <typename T>
T safe_div(T a, T b) {
    if (b == 0) {
        throw std::runtime_error("division by zero");
    }
    return a / b;
}

int main() {
    safe_div(4, 2); // инстанцируется safe_div<int>

    // safe_div(std::string("a"), std::string("b"));
    // ↑ если раскомментировать, будет ошибка компиляции,
    //   потому что для std::string нет оператора /
}
```

Пока вы не используете шаблон с каким-то типом, компилятору не нужно проверять этот случай.

---

### 3.3. Этапы компиляции и проверки шаблонов

Для шаблонов важно понимать, **когда именно** что проверяется.

1. **Этап объявления шаблона** (компилятор видит сам шаблон):

   ```cpp
   template <typename T>
   T foo(T x) {
       int a[10];
       return x + 1;
   }
   ```

   На этом этапе компилятор:

   - проверяет синтаксис;
   - проверяет всё, что **не зависит от `T`** (недепендентный код):
     - что `int a[10];` — корректный массив;
     - что `return` есть во всех ветках и т.п.

   Но он **не проверяет**, есть ли у `T` оператор `+` с `int`.  
   Это зависит от конкретного типа, значит будет проверено позже — при инстанцировании.

2. **Этап инстанцирования** (подстановка конкретного типа):

   ```cpp
   int main() {
       foo(10);               // T = int → ОК, int + int существует
       foo(1.5);              // T = double → ОК, double + int существует
       // foo(std::string("hi")); // T = std::string → ОШИБКА, string + int не определён
   }
   ```

   Для каждой конкретной инстанции (`foo<int>`, `foo<double>`, `foo<std::string>`) компилятор:

   - подставляет `T`;
   - **проверяет весь код, который зависит от `T`**:
     - наличие нужных операторов (`+`, `<`, `[]`, `()` и т.д.);
     - корректность вызовов методов (`x.method()` — существует ли такой метод у типа `T`?);
     - соответствие аргументов шаблона нетиповым параметрам и т.д.

Из этого следуют важные выводы:

- Шаблон может содержать код, который **не компилируется для некоторых типов**, и это нормально,  
  пока вы не пытаетесь его инстанцировать с этими типами.
- Ошибка может появиться не в месте объявления шаблона, а **в месте использования** с конкретным типом.

Пример:

```cpp
template <typename T>
void print_first(const T& container) {
    // предполагаем, что у container есть operator[]
    std::cout << container[0] << '\n';
}

int main() {
    std::vector<int> v{1, 2, 3};
    print_first(v);   // ОК: у std::vector есть operator[]

    // int x = 10;
    // print_first(x); // ОШИБКА при инстанцировании print_first<int>:
                      // у int нет operator[]
}
```

---

### 3.4. Почему шаблоны обычно в заголовках

- Для инстанцирования компилятору нужно видеть **тело** шаблона.
- Поэтому реализация шаблонов почти всегда находится в `.h` / `.hpp`.
- Если спрятать реализацию шаблона в `.cpp` и не инстанцировать его там явно, другие модули не смогут создать свои инстанции — линковщик не найдёт нужный код.

---

## 4. Специализация шаблонов

### 4.1. Полная специализация

Общий шаблон:

```cpp
template <typename T>
class TypeName {
public:
    static std::string get() {
        return "unknown";
    }
};
```

Специализации для некоторых типов:

```cpp
template <>
class TypeName<int> {
public:
    static std::string get() {
        return "int";
    }
};

template <>
class TypeName<double> {
public:
    static std::string get() {
        return "double";
    }
};

int main() {
    std::cout << TypeName<char>::get()   << '\n'; // unknown
    std::cout << TypeName<int>::get()    << '\n'; // int
    std::cout << TypeName<double>::get() << '\n'; // double
}
```

- полная специализация — отдельная реализация для конкретного набора параметров;
- для этих параметров общий шаблон больше не используется.

### 4.2. Частичная специализация

Частичная специализация есть только у классов (у функций — нет).

Общий шаблон:

```cpp
template <typename T>
class Wrapper {
public:
    static void print(const T& value) {
        std::cout << "value: " << value << '\n';
    }
};
```

Частичная специализация, если `T` — `std::vector<...>`:

```cpp
#include <vector>

template <typename T>
class Wrapper<std::vector<T>> {
public:
    static void print(const std::vector<T>& v) {
        std::cout << "vector, size = " << v.size() << '\n';
    }
};

int main() {
    Wrapper<int>::print(10);              // value: 10

    std::vector<int> v{1, 2, 3};
    Wrapper<std::vector<int>>::print(v);  // vector, size = 3
}
```

- общий шаблон `Wrapper<T>` подходит для любого типа `T`;
- специализация `Wrapper<std::vector<T>>` срабатывает, когда параметр имеет вид `std::vector<что-то>`;
- параметр `T` внутри специализации — тип элемента вектора (`int` в примере).

### 4.3. Функции: специализация vs перегрузка

Для функций частичной специализации нет. Обычно делают просто перегрузку:

```cpp
template <typename T>
void foo(T) {
    std::cout << "generic\n";
}

void foo(int) {
    std::cout << "int overload\n";
}

int main() {
    foo(1);     // int overload
    foo(3.14);  // generic
}
```

---

## 5. Рекурсивные шаблоны и частичная специализация

### 5.1. Зачем это нужно

Типичный паттерн:

- общий шаблон описывает **шаг рекурсии**;
- частичная (или полная) специализация описывает **базовый случай**.

Так можно выразить рекурсию по параметру шаблона (например, по числу `N`).

### 5.2. Пример: сумма элементов массива

Сделаем класс `ArraySum<T, N>`, который считает сумму N элементов массива:

- общий случай для `N > 1`: первый элемент + сумма оставшихся `N - 1`;
- специализация для `N = 1`: просто вернуть единственный элемент.

```cpp
#include <cstddef>
#include <iostream>

template <typename T, std::size_t N>
class ArraySum {
public:
    static T sum(const T (&arr)[N]) {
        // шаг рекурсии: первый элемент + сумма остальных
        return arr[0] + ArraySum<T, N - 1>::sum(arr + 1);
    }
};

// Частичная специализация для N = 1
template <typename T>
class ArraySum<T, 1> {
public:
    static T sum(const T (&arr)[1]) {
        return arr[0];
    }
};

int main() {
    int data[3] = {1, 2, 3};

    int result = ArraySum<int, 3>::sum(data);
    std::cout << "sum = " << result << '\n'; // sum = 6
}
```

- общий шаблон `ArraySum<T, N>` описывает рекурсивный шаг (использует `ArraySum<T, N - 1>`);
- частичная специализация `ArraySum<T, 1>` задаёт базовый случай;
- специализация частичная, потому что фиксирован только `N = 1`, но `T` остаётся шаблонным.

### 5.3. Как компилятор инстанцирует этот пример

Вызов:

```cpp
int result = ArraySum<int, 3>::sum(data);
```

Последовательность:

1. Нужен `ArraySum<int, 3>` → используется общий шаблон `ArraySum<T, N>` с `T = int`, `N = 3`.
2. Внутри `ArraySum<int, 3>::sum` есть:

   ```cpp
   return arr[0] + ArraySum<int, 2>::sum(arr + 1);
   ```

   → инстанцируется `ArraySum<int, 2>` по тому же шаблону.

3. Внутри `ArraySum<int, 2>::sum`:

   ```cpp
   return arr[0] + ArraySum<int, 1>::sum(arr + 1);
   ```

   → нужен `ArraySum<int, 1>`.

4. Для `ArraySum<int, 1>` есть два варианта:

   - общий шаблон `ArraySum<T, N>`;
   - частичная специализация `ArraySum<T, 1>`.

   Специализация `ArraySum<T, 1>` более узкая и подходит лучше → выбирается она.

5. В `ArraySum<T, 1>` рекурсивных вызовов нет — рекурсия завершается.

Инстанцируются:

- `ArraySum<int, 3>`
- `ArraySum<int, 2>`
- `ArraySum<int, 1>` (по частичной специализации)

### 5.4. Что будет без базового случая

Если убрать специализацию `ArraySum<T, 1>`, общий шаблон будет порождать:

- `ArraySum<int, 3>` → `ArraySum<int, 2>` → `ArraySum<int, 1>` → `ArraySum<int, 0>` → `ArraySum<int, -1>` → …

Рекурсия по шаблонам не остановится:

- компилятор скорее всего выдаст ошибку вида  
  «recursive template instantiation exceeds maximum depth»;
- или наткнётся на некорректный код (например, массив отрицательного размера).

> В рекурсивных шаблонах обязательно должен быть тип/параметр, на котором рекурсия останавливается — обычно через отдельную специализацию.

### 5.5. Ещё один классический пример — факториал

```cpp
template <int N>
class Factorial {
public:
    static constexpr int value = N * Factorial<N - 1>::value;
};

template <>
class Factorial<0> {
public:
    static constexpr int value = 1;
};

int main() {
    int x = Factorial<5>::value; // 120
}
```

- общий случай: `Factorial<N>` — рекурсивный шаг;
- полная специализация `Factorial<0>` — базовый случай.

---

## 6. Автоматический вывод типов шаблонов: детали

Важен не только факт, что тип выводится, но и **как именно** он выводится из аргументов.

### 6.1. Передача по значению

```cpp
template <typename T>
void foo(T value) {
    // T — "сырой" тип без ссылок и cv-квалификаторов
    // (cv = const/volatile, например: const int, volatile int, const volatile int)
}

int main() {
    int x = 10;
    const int cx = 20;

    foo(x);   // T -> int, параметр: int
    foo(cx);  // T -> int, параметр: int (const "срезался")
    foo(5);   // T -> int, параметр: int
}
```

- `T` не будет `const` или `&`, даже если вы передали `const int` или `int&`;
- значение копируется/перемещается в параметр функции.

### 6.2. Передача по ссылке

```cpp
template <typename T>
void bar(T& value) {
    // T — "реальный" тип без &, но с cv-квалификаторами,
    // если они были (cv = const/volatile)
}

int main() {
    int x = 10;
    const int cx = 20;

    bar(x);   // T -> int,        параметр: int&
    // bar(5); // ошибка: нельзя привязать lvalue-ссылку к временной
    bar(cx);  // T -> const int,  параметр: const int&
}
```

- если параметр — `T&`, то в `T` попадает вся информация о `const`/`volatile`;
- `bar(const int&)` уже не сможет менять аргумент.

Похожая история с `const T&`:

```cpp
template <typename T>
void baz(const T& value) {
    // можно передать как lvalue, так и временное
}

int main() {
    int x = 10;
    baz(x);    // T -> int, параметр: const int&
    baz(5);    // T -> int, параметр: const int&
}
```

`const T&` удобно, когда нужно:

- не копировать объект;
- принимать и обычные объекты, и временные.

### 6.3. Массивы и автоматический вывод размера

Обычно массивы при передаче в функцию **превращаются в указатель**:

```cpp
void f(int arr[]);   // на самом деле: void f(int* arr);
```

Через шаблоны можно сохранить и тип элемента, и размер массива:

```cpp
template <typename T, std::size_t N>
void print_array(const T (&arr)[N]) {
    for (std::size_t i = 0; i < N; ++i) {
        std::cout << arr[i] << ' ';
    }
    std::cout << '\n';
}

int main() {
    int data[3] = {1, 2, 3};
    print_array(data); // T -> int, N -> 3
}
```

- `T` выводится как тип элементов массива (`int`);
- `N` — как размер массива (`3`);
- мы не теряем размер и можем использовать его внутри функции.

---

## 7. Отличия шаблонов функций и шаблонов классов

Коротко соберём в одном месте, чем шаблоны **функций** отличаются от шаблонов **классов**.

### 7.1. Вывод параметров шаблона

- **Функции**  
  Типы почти всегда выводятся автоматически по аргументам:
  ```cpp
  template <typename T>
  T add(T a, T b) { return a + b; }

  int x = add(1, 2);       // T -> int
  auto y = add(1.0, 2.5);  // T -> double
  ```
  Тип можно указать явно, но обычно не нужно: `add<int>(1, 2)`.

- **Классы**  
  Типы почти всегда задаются явно:
  ```cpp
  template <typename T>
  class Box { /* ... */ };

  Box<int>    bi(10);
  Box<double> bd(3.14);
  ```
  Без `Box<int>` компилятор не знает, какой конкретный тип нужен.

### 7.2. Частичная специализация

- **Функции**
  - Частичная специализация **запрещена**.
  - Вместо неё используют **перегрузку** или `if constexpr` внутри общего шаблона.
  ```cpp
  template <typename T>
  void foo(T) { std::cout << "generic\n"; }

  void foo(int) { std::cout << "int overload\n"; }
  ```

- **Классы**
  - Частичная специализация **разрешена и активно используется**:
  ```cpp
  template <typename T>
  class Wrapper {
  public:
      static void print(const T& value) {
          std::cout << "value: " << value << '\n';
      }
  };

  template <typename T>
  class Wrapper<std::vector<T>> {
  public:
      static void print(const std::vector<T>& v) {
          std::cout << "vector, size = " << v.size() << '\n';
      }
  };
  ```

### 7.3. Перегрузка

- **Функции**
  - Шаблонные функции можно перегружать:
    - другими шаблонными функциями;
    - обычными (не-шаблонными) функциями.
  - Не-шаблонная версия выигрывает, если подходит по типам.

- **Классы**
  - У классов понятия «перегрузка» нет, есть разные **шаблоны** и их **специализации**:
    - `Box<T>`, `Optional<T>`, `Pair<T, U>` — разные шаблоны;
    - `Box<int>` и `Box<double>` — разные инстанции одного шаблона.

### 7.4. Инстанцирование и результат

- **Функции**
  - Для каждого `T` компилятор генерирует свою версию функции (`add<int>`, `add<double>` и т.д.).

- **Классы**
  - Для каждого набора параметров получается **отдельный тип**:
    - `Box<int>`, `Box<double>`, `StaticArray<int, 10>` и т.д.

### 7.5. Использование в коде

- **Шаблоны функций** — чаще всего «обёртки» над алгоритмом:
  - общая логика одна;
  - типы меняются;
  - удобно, когда хочется одну реализацию для разных типов аргументов.

- **Шаблоны классов** — задают семейства типов:
  - контейнеры (`std::vector<T>`, `std::array<T, N>`);
  - обёртки (`std::optional<T>`, `std::unique_ptr<T>`);
  - вспомогательные структуры (`std::pair<T, U>`, `std::tuple<...>`).

---

## 8. Шаблоны с переменным числом аргументов

### 8.1. Идея

Иногда заранее неизвестно, сколько аргументов надо передать в шаблон.

Примеры:

- функция `print_all(...)`, которая печатает любое количество аргументов;
- `std::tuple`, который может хранить произвольное количество полей разных типов.

Для этого есть **variadic templates**:

```cpp
template <typename... Ts>
void func(Ts... args);
```

`Ts...` — **пакет параметров типа**, `args...` — **пакет параметров функции**.

### 8.2. Пример: печать всех аргументов (fold-expression, C++17)

```cpp
template <typename... Ts>
void print_all(Ts... args) {
    (std::cout << ... << args) << '\n';
}

int main() {
    print_all(1, 2, 3);
    print_all("hello", 3.14, std::string("world"));
}
```

- `(std::cout << ... << args)` разворачивается в  
  `((std::cout << arg1) << arg2) << arg3` и т.д.;
- не нужно писать рекурсию вручную.

### 8.3. Пример: сумма всех аргументов

```cpp
template <typename... Ts>
auto sum_all(Ts... args) {
    return (args + ...); // ((arg1 + arg2) + arg3) + ...
}

int main() {
    auto s = sum_all(1, 2, 3, 4); // 10
}
```

Ограничение:

- типы должны поддерживать оператор `+` в такой комбинации.

### 8.4. Класс с переменным числом параметров

```cpp
template <typename... Ts>
class MyTupleLike {
    // просто пример: несколько полей разных типов
};
```

Такая схема лежит в основе `std::tuple`, `std::variant` и многих других шаблонов стандартной библиотеки.

---

## 9. `std::tuple` как пример вариадического шаблона

### 9.1. Объявление

В стандартной библиотеке `std::tuple` объявлен примерно так:

```cpp
template <class... Types>
class tuple;
```

То есть это **класс-шаблон с переменным числом типовых параметров**.

Примеры:

```cpp
#include <tuple>
#include <string>

int main() {
    std::tuple<int, std::string, double> t{42, "hello", 3.14};

    // доступ по индексу
    std::cout << std::get<0>(t) << '\n'; // 42
    std::cout << std::get<1>(t) << '\n'; // "hello"
    std::cout << std::get<2>(t) << '\n'; // 3.14
}
```

### 9.2. Создание и вывод типов

Часто используют `std::make_tuple`:

```cpp
auto t = std::make_tuple(1, 2.5, std::string("hi"));
// тип t: std::tuple<int, double, std::string>
```

Типы параметров выводятся автоматически (как с шаблонными функциями).

### 9.3. Использование `tuple` для возврата нескольких значений

```cpp
std::tuple<int, double> div_mod(int a, int b) {
    return std::make_tuple(a / b, a % b);
}

int main() {
    auto [q, r] = div_mod(10, 3); // structured binding (C++17)
    std::cout << "q = " << q << ", r = " << r << '\n';
}
```

- `div_mod` возвращает `std::tuple<int, double>`;
- в `main` результат разбирается на две переменные через structured bindings.

### 9.4. `std::tie` и «игнорирование» части значений

```cpp
#include <tuple>

int main() {
    int q;
    double r;

    std::tie(q, r) = div_mod(10, 3);

    // можно игнорировать часть значений
    std::tie(std::ignore, r) = div_mod(10, 3);
}
```

### 9.5. Связь с variadic templates

- `std::tuple` — это variadic template (`template <class... Types>`).
- Внутри он реализован через variadic templates + специализации:
  - рекурсивные шаблонные структуры;
  - частичные специализации для «головы» и «хвоста» списка типов;
  - вычисления на этапе компиляции (метапрограммирование).
