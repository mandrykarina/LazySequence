#pragma once
#include <vector>
#include <functional>
#include <stdexcept>
#include <memory>
#include <mutex>

// Простая ленивость: значения вычисляются по запросу и кэшируются.
// Generator: std::function<T(size_t index, const std::vector<T>& cache)>
template <typename T>
class LazySequence
{
public:
    using Generator = std::function<T(size_t, const std::vector<T> &)>;

    // --- Конструкторы ---
    explicit LazySequence(Generator gen = nullptr)
        : generator(gen)
    {
    }

    explicit LazySequence(const std::vector<T> &items)
        : cache(items), generator(nullptr)
    {
    }

    // --- Копирование запрещено (из-за mutex) ---
    LazySequence(const LazySequence &other) = delete;
    LazySequence &operator=(const LazySequence &other) = delete;

    // --- Перемещение разрешено ---
    LazySequence(LazySequence &&other) noexcept
    {
        std::lock_guard<std::mutex> lock(other.mtx);
        cache = std::move(other.cache);
        generator = std::move(other.generator);
    }

    LazySequence &operator=(LazySequence &&other) noexcept
    {
        if (this != &other)
        {
            std::lock_guard<std::mutex> lock1(mtx);
            std::lock_guard<std::mutex> lock2(other.mtx);
            cache = std::move(other.cache);
            generator = std::move(other.generator);
        }
        return *this;
    }

    // --- Методы доступа ---
    // Получить элемент по индексу (вычисляет недостающие)
    T Get(size_t index)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (index < cache.size())
            return cache[index];
        if (!generator)
            throw std::out_of_range("No generator for index >= materialized count");
        while (cache.size() <= index)
        {
            size_t i = cache.size();
            cache.push_back(generator(i, cache));
        }
        return cache[index];
    }

    // Получить первый элемент
    T GetFirst() { return Get(0); }

    // Количество вычисленных элементов
    size_t GetMaterializedCount() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return cache.size();
    }

    // --- Операции ---
    void Append(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.push_back(value);
    }

    // Map: возвращает новую ленивую последовательность, чей генератор вызывает this->Get
    template <typename U>
    LazySequence<U> Map(std::function<U(const T &)> f) const
    {
        const LazySequence<T> *self = this;
        typename LazySequence<U>::Generator g = [self, f](size_t idx, const std::vector<U> &) -> U
        {
            T val = self->Get(idx);
            return f(val);
        };
        return LazySequence<U>(g);
    }

private:
    mutable std::mutex mtx;
    std::vector<T> cache;
    Generator generator;
};
