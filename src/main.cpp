#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include "../include/LazySequence.h"
#include "../include/Stream.h"
#include "../include/StatisticsCollector.h"

// небольшой helper: ленивый генератор случайных чисел
std::function<std::optional<int>()> RandomGenerator(int count, int minv = 0, int maxv = 100)
{
    auto mt = std::make_shared<std::mt19937>(std::random_device{}());
    auto dist = std::make_shared<std::uniform_int_distribution<int>>(minv, maxv);
    auto remaining = std::make_shared<int>(count);
    return [mt, dist, remaining]() -> std::optional<int>
    {
        if (*remaining <= 0)
            return std::nullopt;
        --(*remaining);
        return (*dist)(*mt);
    };
}

// ленивый генератор Фибоначчи (как демонстрация LazySequence)
LazySequence<long long> MakeFibonacciSequence()
{
    LazySequence<long long>::Generator g = [](size_t idx, const std::vector<long long> &cache) -> long long
    {
        if (idx == 0)
            return 0;
        if (idx == 1)
            return 1;
        return cache[idx - 1] + cache[idx - 2];
    };
    LazySequence<long long> seq(g);
    // обязательно не materialize заранее
    return seq;
}

void RunStreamDemo()
{
    using T = int;
    std::cout << "Выберите источник:\n1) Файл\n2) Ввод с клавиатуры (stdin)\n3) Генератор случайных чисел (100 элементов)\n4) Вектор-пример\n> ";
    int choice;
    std::cin >> choice;

    std::unique_ptr<ReadOnlyStream<T>> streamPtr;
    if (choice == 1)
    {
        std::cout << "Введите имя файла: ";
        std::string fname;
        std::cin >> fname;
        try
        {
            streamPtr = std::make_unique<ReadOnlyStream<T>>(fname);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Ошибка открытия файла: " << e.what() << std::endl;
            return;
        }
    }
    else if (choice == 2)
    {
        // FromStdin uses std::cin
        streamPtr = std::make_unique<ReadOnlyStream<T>>(ReadOnlyStream<T>::FromStdin());
        std::cout << "Введите числа через пробел/enter. Чтобы остановить - нажмите Ctrl+D (или Ctrl+Z в Windows).\n";
    }
    else if (choice == 3)
    {
        auto gen = RandomGenerator(100, 0, 200);
        streamPtr = std::make_unique<ReadOnlyStream<T>>(gen);
    }
    else
    {
        std::vector<T> v = {5, 3, 8, 1, 9, 10, 2, 2, 7, 6, 4};
        streamPtr = std::make_unique<ReadOnlyStream<T>>(v);
    }

    StatisticsCollector<T> stats;
    LazySequence<T> lazySeq; // без генератора, мы будем Append'ить прочитанные значения

    std::cout << "Запуск чтения потока...\n";
    T value;
    while (streamPtr->TryRead(value))
    {
        lazySeq.Append(value); // materialize on demand (we append as we read)
        stats.Add(value);

        auto med = stats.GetMedian();
        std::cout << "Прочитано: " << value
                  << " | count=" << stats.GetCount()
                  << " | avg=" << stats.GetAverage();
        if (med)
            std::cout << " | median=" << *med;
        auto var = stats.GetVariance();
        if (var)
            std::cout << " | var=" << *var;
        std::cout << " | min=" << stats.GetMin() << " max=" << stats.GetMax();
        std::cout << "\n";

        // имитируем поток: небольшой sleep только если источник генератор
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\n--- Конец потока ---\nИтог: count=" << stats.GetCount()
              << " avg=" << stats.GetAverage();
    auto med = stats.GetMedian();
    if (med)
        std::cout << " median=" << *med;
    auto var = stats.GetVariance();
    if (var)
        std::cout << " variance=" << *var;
    std::cout << " min=" << stats.GetMin() << " max=" << stats.GetMax() << "\n";

    std::cout << "Материализовано в ленивой последовательности: " << lazySeq.GetMaterializedCount() << " элементов.\n";
}

void RunFibonacciDemo()
{
    auto fib = MakeFibonacciSequence();
    std::cout << "Введите сколько первых чисел Фибоначчи вывести: ";
    size_t n;
    std::cin >> n;
    for (size_t i = 0; i < n; ++i)
    {
        std::cout << i << ": " << fib.Get(i) << "\n";
    }
    std::cout << "Materialized count: " << fib.GetMaterializedCount() << "\n";
}

int main(int argc, char **argv)
{
    std::cout << "=== LazyStreamStats Demo ===\n";
    while (true)
    {
        std::cout << "\nВыберите режим:\n1) Демонстрация потока и сбор статистики\n2) Демонстрация ленивой последовательности (Fibonacci)\n3) Запустить автотесты (tests executable)\n0) Выйти\n> ";
        int cmd;
        if (!(std::cin >> cmd))
            break;
        if (cmd == 0)
            break;
        if (cmd == 1)
            RunStreamDemo();
        else if (cmd == 2)
            RunFibonacciDemo();
        else if (cmd == 3)
        {
            std::cout << "Автотесты запускаются отдельно (см. tests executable). Если собрали через CMake, выполните ./tests\n";
        }
        else
        {
            std::cout << "Неизвестная команда\n";
        }
    }
    std::cout << "Bye\n";
    return 0;
}
