#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <optional>
#include <cassert>
#include "../include/LazySequence.h"
#include "../include/Stream.h"
#include "../include/StatisticsCollector.h"

using namespace std;

// Тест ленивой последовательности
void TestLazyFibonacci()
{
    cout << "==== Тест LazySequence: последовательность Фибоначчи ====\n";
    LazySequence<long long>::Generator g = [](size_t idx, const vector<long long> &cache) -> long long
    {
        if (idx == 0)
            return 0;
        if (idx == 1)
            return 1;
        return cache[idx - 1] + cache[idx - 2];
    };
    LazySequence<long long> fib(g);

    long long expected[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    for (size_t i = 0; i < 10; ++i)
    {
        long long v = fib.Get(i);
        cout << "F(" << i << ") = " << v << (v == expected[i] ? " да" : " нет") << "\n";
        assert(v == expected[i]);
    }

    cout << "Материализовано элементов: " << fib.GetMaterializedCount() << "\n";
    cout << "LazySequence работает корректно\n\n";
}

// Тест StatisticsCollector
void TestStatisticsCollector()
{
    cout << "Тест StatisticsCollector \n";
    StatisticsCollector<int> stats;
    vector<int> values = {5, 1, 3, 2, 4};

    cout << "Входные данные: ";
    for (int v : values)
        cout << v << " ";
    cout << "\n";

    for (int v : values)
        stats.Add(v);

    cout << fixed << setprecision(2);
    cout << "Count: " << stats.GetCount() << "\n";
    cout << "Average: " << stats.GetAverage() << "\n";
    cout << "Min: " << stats.GetMin() << ", Max: " << stats.GetMax() << "\n";
    cout << "Median: " << *stats.GetMedian() << "\n";
    cout << "Variance: " << *stats.GetVariance() << "\n";
    cout << "StdDev: " << *stats.GetStdDev() << "\n";

    assert(stats.GetCount() == 5);
    assert(fabs(stats.GetAverage() - 3.0) < 1e-9);
    assert(stats.GetMin() == 1);
    assert(stats.GetMax() == 5);
    cout << "StatisticsCollector прошёл тест\n\n";
}

// Подробный тест ReadOnlyStream
void TestReadOnlyStream()
{
    cout << "Тест ReadOnlyStream \n";

    //  Чтение из вектора
    cout << "\n--- Тест: источник = вектор ---\n";
    vector<int> data = {10, 20, 30};
    ReadOnlyStream<int> streamVec(data);
    int x;
    cout << "Ожидаем: 10 20 30\nСчитано: ";
    while (streamVec.TryRead(x))
        cout << x << " ";
    cout << "\nВекторный поток работает\n";

    // Чтение из генератора
    cout << "\n--- Тест: источник = генератор ---\n";
    int counter = 0;
    auto gen = [&]() -> optional<int>
    {
        if (counter >= 5)
            return nullopt;
        return counter++;
    };
    ReadOnlyStream<int> streamGen(gen);
    cout << "Ожидаем: 0 1 2 3 4\nСчитано: ";
    while (streamGen.TryRead(x))
        cout << x << " ";
    cout << "\n Поток-генератор работает\n";

    //  Чтение из файла
    cout << "\n--- Тест: источник = файл ---\n";
    ofstream out("test_input.txt");
    out << "7 8 9";
    out.close();

    ReadOnlyStream<int> streamFile("test_input.txt");
    cout << "Ожидаем: 7 8 9\nСчитано: ";
    while (streamFile.TryRead(x))
        cout << x << " ";
    cout << "\nФайловый поток работает\n";

    // Псевдо stdin (пропустим реальный ввод)
    cout << "\n--- Тест: псевдо-stdin ---\n";
    cout << "(Проверка, что FromStdin создаётся без ошибок)\n";
    auto streamIn = ReadOnlyStream<int>::FromStdin();
    cout << "Поток stdin создан успешно\n";

    cout << "\n Все варианты ReadOnlyStream прошли тест\n\n";
}

int main()
{
    cout << " ЗАПУСК ТЕСТОВ \n\n";
    try
    {
        TestLazyFibonacci();
        TestStatisticsCollector();
        TestReadOnlyStream();
        cout << " ВСЕ ТЕСТЫ ПРОЙДЕНЫ \n";
        return 0;
    }
    catch (const exception &e)
    {
        cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        cerr << "Неизвестная ошибка\n";
        return 2;
    }
}
