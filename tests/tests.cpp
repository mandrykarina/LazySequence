#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <map>
#include <string>
#include "../include/LazySequence.h"
#include "../include/Generator.h"

using namespace std;

// Подсчёт частот подстрок фиксированной длины (1 проход)
std::map<std::string, size_t> CountSubstringFrequencies(LazySequence<char> &seq, size_t k)
{
    if (k == 0)
        throw std::invalid_argument("длина > 0");
    Generator<char> gen(&seq);
    std::map<std::string, size_t> freq;
    std::string window;

    while (gen.HasNext())
    {
        char c = gen.GetNext();
        window.push_back(c);
        if (window.size() > k)
            window.erase(window.begin());
        if (window.size() == k)
            ++freq[window];
    }
    return freq;
}

// ТЕСТ 1 — Проверка работы генератора и ленивой последовательности (Фибоначчи)
void TestFibonacciGenerator()
{
    cout << "фибоначи генератор\n";
    LazySequence<long long>::GenFunc fibGen = [](size_t idx, const vector<long long> &cache) -> long long
    {
        if (idx == 0)
            return 0;
        if (idx == 1)
            return 1;
        return cache[idx - 1] + cache[idx - 2];
    };

    LazySequence<long long> seq(fibGen);
    Generator<long long> gen(&seq, 0);

    vector<long long> got;
    for (int i = 0; i < 12; ++i)
        got.push_back(gen.GetNext());

    long long expected[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
    for (size_t i = 0; i < got.size(); ++i)
        assert(got[i] == expected[i]);

    cout << "Первые 12 чисел Фибоначчи:\n";
    for (auto v : got)
        cout << v << " ";
    cout << "\nМатериализовано элементов: " << seq.GetMaterializedCount() << "\n";
}

// ТЕСТ 2 — Проверка модификаций (вставка, удаление, добавление)
void TestModifyMaterializedSequence()
{
    cout << " проверка модификаций \n";
    vector<int> initial = {10, 20, 30};
    LazySequence<int> seq(initial);
    Generator<int> gen(&seq, 1);

    gen.Insert(15);
    gen.Append(40);
    gen.Remove(20);

    vector<int> values;
    for (size_t i = 0; i < seq.GetMaterializedCount(); ++i)
        values.push_back(seq.Get(i));

    vector<int> expected = {10, 15, 30, 40};
    assert(values == expected);

    cout << "После модификаций (Insert/Append/Remove): ";
    for (auto v : values)
        cout << v << " ";
}

// ТЕСТ 3 — Проверка Map / Reduce
void TestMapReduce()
{
    cout << "   TestMapReduce \n";
    vector<int> data = {1, 2, 3, 4};
    LazySequence<int> seq(data);

    auto squares = seq.Map<int>([](const int &x)
                                { return x * x; });
    int sum = seq.Reduce<int>([](int acc, const int &v)
                              { return acc + v; }, 0);
    assert(sum == 10);

    cout << "начальная: ";
    for (size_t i = 0; i < seq.GetMaterializedCount(); ++i)
        cout << seq.Get(i) << " ";

    cout << "\nв квадрате: ";
    for (size_t i = 0; i < seq.GetMaterializedCount(); ++i)
        cout << squares.Get(i) << " ";

    cout << "\nSum (Reduce): " << sum;
}

// ТЕСТ 4 — Прикладная задача: подсчёт частот подстрок в тексте
void TestSubstringFrequency()
{
    cout << "прикладная задача\n";
    string text = "ababba";
    vector<char> chars(text.begin(), text.end());
    LazySequence<char> seq(chars);

    cout << "Исходная строка: \"" << text << "\"\n";
    cout << "Задача: посчитать, сколько раз встречаются подстроки длины 2\n";
    cout << "и сделать это за один проход, не сохраняя все подстроки заранее.\n\n";

    size_t k = 2;
    auto freq = CountSubstringFrequencies(seq, k);

    cout << "Результаты анализа:\n";
    for (auto &[sub, count] : freq)
        cout << "  '" << sub << "' : " << count << "\n";

    // Проверка ожидаемых результатов
    assert(freq["ab"] == 2);
    assert(freq["ba"] == 2);
}

int main()
{
    cout << fixed << setprecision(6);
    try
    {
        TestFibonacciGenerator();
        TestModifyMaterializedSequence();
        TestMapReduce();
        TestSubstringFrequency();
        cout << "ALL TESTS PASSED\n";
        return 0;
    }
    catch (const std::exception &ex)
    {
        cerr << "TEST FAILURE: " << ex.what() << "\n";
        return 2;
    }
    catch (...)
    {
        cerr << "UNKNOWN TEST FAILURE\n";
        return 3;
    }
}
