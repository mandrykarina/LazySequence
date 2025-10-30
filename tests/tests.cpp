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
        throw std::invalid_argument("Substring length must be > 0");
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

// === Тесты ===

void TestFibonacciGenerator()
{
    cout << "=== TestFibonacciGenerator ===\n";
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

    cout << "Materialized count: " << seq.GetMaterializedCount() << "\n";
    cout << "TestFibonacciGenerator passed\n\n";
}

void TestModifyMaterializedSequence()
{
    cout << "=== TestModifyMaterializedSequence ===\n";
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
    cout << "Modified sequence: ";
    for (auto v : values)
        cout << v << " ";
    cout << "\nTestModifyMaterializedSequence passed\n\n";
}

void TestMapReduce()
{
    cout << "=== TestMapReduce ===\n";
    vector<int> data = {1, 2, 3, 4};
    LazySequence<int> seq(data);

    auto squares = seq.Map<int>([](const int &x)
                                { return x * x; });
    int sum = seq.Reduce<int>([](int acc, const int &v)
                              { return acc + v; }, 0);
    assert(sum == 10);

    cout << "Original: ";
    for (size_t i = 0; i < seq.GetMaterializedCount(); ++i)
        cout << seq.Get(i) << " ";
    cout << "\nSquares: ";
    for (size_t i = 0; i < squares.GetMaterializedCount(); ++i)
        cout << squares.Get(i) << " ";
    cout << "\nSum: " << sum << "\nTestMapReduce passed\n\n";
}

void TestSubstringFrequency()
{
    cout << "=== TestSubstringFrequency ===\n";
    string text = "ababa";
    vector<char> chars(text.begin(), text.end());
    LazySequence<char> seq(chars);

    size_t k = 2;
    auto freq = CountSubstringFrequencies(seq, k);

    assert(freq["ab"] == 2);
    assert(freq["ba"] == 2);

    cout << "Text: " << text << "\n";
    cout << "Substrings of length " << k << ":\n";
    for (auto &[sub, count] : freq)
        cout << "'" << sub << "' : " << count << "\n";

    cout << "TestSubstringFrequency passed\n\n";
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
