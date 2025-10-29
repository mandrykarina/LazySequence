#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cassert>
#include "../include/LazySequence.h"
#include "../include/Stream.h"
#include "../include/StatisticsCollector.h"

using namespace std;

void TestLazyFibonacci()
{
    cout << " Тест LazySequence: последовательность Фибоначчи \n";

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
        cout << "F(" << i << ") = " << v;
        if (v == expected[i])
            cout << "\n";
        else
            cout << " (ожидалось " << expected[i] << ")\n";
        assert(v == expected[i]);
    }

    cout << "Материализовано элементов: " << fib.GetMaterializedCount() << "\n";
    cout << "Тест LazySequence завершён успешно\n\n";
}

void TestStatisticsCollector()
{
    cout << "Тест StatisticsCollector\n";

    StatisticsCollector<int> stats;
    vector<int> values = {5, 1, 3, 2, 4};

    cout << "Входные данные: ";
    for (int v : values)
        cout << v << " ";
    cout << "\n";

    for (int v : values)
        stats.Add(v);

    cout << fixed << setprecision(2);
    cout << "Количество: " << stats.GetCount() << "\n";
    cout << "Среднее: " << stats.GetAverage() << "\n";
    cout << "Мин: " << stats.GetMin() << " | Макс: " << stats.GetMax() << "\n";

    auto median = stats.GetMedian();
    auto var = stats.GetVariance();
    auto sd = stats.GetStdDev();

    if (median)
        cout << "Медиана: " << *median << "\n";
    if (var)
        cout << "Дисперсия: " << *var << "\n";
    if (sd)
        cout << "Стандартное отклонение: " << *sd << "\n";

    assert(fabs(stats.GetAverage() - 3.0) < 1e-9);
    assert(median && fabs(*median - 3.0) < 1e-9);
    cout << "Тест StatisticsCollector завершён успешно\n\n";
}

void TestStreamFromVector()
{
    cout << "Тест ReadOnlyStream (чтение из вектора) \n";

    vector<int> data = {10, 20, 30};
    ReadOnlyStream<int> stream(data);

    cout << "Ожидаемые данные: 10 20 30\n";
    cout << "Реально считано:  ";

    int x;
    while (stream.TryRead(x))
    {
        cout << x << " ";
    }
    cout << "\nТест StreamFromVector завершён успешно\n\n";
}

int main()
{
    cout << " ЗАПУСК ТЕСТОВ\n\n";
    try
    {
        TestLazyFibonacci();
        TestStatisticsCollector();
        TestStreamFromVector();
        cout << "ВСЕ ТЕСТЫ ПРОЙДЕНЫ \n";
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
