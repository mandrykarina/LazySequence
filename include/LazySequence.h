#pragma once
#include <vector>
#include <functional>
#include <stdexcept>
#include <memory>
#include <mutex>
#include <algorithm>

template <typename T>
class Generator; // forward declaration

template <typename T>
class LazySequence
{
private:
    mutable std::mutex mtx;
    std::vector<T> cache;
    GenFunc generator;
    friend class Generator<T>;

public:
    using GenFunc = std::function<T(size_t, const std::vector<T> &)>;

    LazySequence() : generator(nullptr) {}
    explicit LazySequence(const std::vector<T> &items)
        : cache(items), generator(nullptr) {}
    explicit LazySequence(GenFunc gen)
        : generator(gen) {}

    LazySequence(const LazySequence &other) = delete;
    LazySequence &operator=(const LazySequence &other) = delete;

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

    // Доступ
    T Get(size_t index)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (index < cache.size())
            return cache[index];
        if (!generator)
            throw std::out_of_range("Index >= materialized and no generator");
        while (cache.size() <= index)
        {
            size_t i = cache.size();
            cache.push_back(generator(i, cache));
        }
        return cache[index];
    }

    T GetFirst() { return Get(0); }

    T GetLast()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (generator)
            throw std::runtime_error("Sequence may be infinite; GetLast undefined when generator exists");
        if (cache.empty())
            throw std::out_of_range("Empty sequence");
        return cache.back();
    }

    size_t GetMaterializedCount() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return cache.size();
    }

    bool HasGenerator() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return static_cast<bool>(generator);
    }

    // Модификации
    void Append(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.push_back(value);
    }

    void Prepend(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.insert(cache.begin(), value);
    }

    void InsertAt(const T &value, size_t index)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (index > cache.size())
            throw std::out_of_range("InsertAt: index > materialized count");
        cache.insert(cache.begin() + static_cast<ptrdiff_t>(index), value);
    }

    bool RemoveValue(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = std::find(cache.begin(), cache.end(), value);
        if (it == cache.end())
            return false;
        cache.erase(it);
        return true;
    }

    void Concat(const LazySequence<T> &other)
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::lock_guard<std::mutex> lock2(other.mtx);
        cache.insert(cache.end(), other.cache.begin(), other.cache.end());
    }

    // === Исправленный Map (снятие const через const_cast) ===
    template <typename U>
    LazySequence<U> Map(std::function<U(const T &)> f) const
    {
        auto *self = const_cast<LazySequence<T> *>(this);
        typename LazySequence<U>::GenFunc g = [self, f](size_t idx, const std::vector<U> &) -> U
        {
            T val = self->Get(idx);
            return f(val);
        };
        return LazySequence<U>(g);
    }

    template <typename U>
    U Reduce(std::function<U(U, const T &)> reducer, U init) const
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (const T &v : cache)
            init = reducer(init, v);
        if (generator)
            throw std::runtime_error("Reduce over potentially infinite sequence is unsafe");
        return init;
    }

    LazySequence<T> Where(std::function<bool(const T &)> pred) const
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<T> res;
        for (const T &v : cache)
            if (pred(v))
                res.push_back(v);
        return LazySequence<T>(res);
    }

    Generator<T> CreateGenerator(size_t startIndex = 0);
    void SetGenerator(GenFunc g)
    {
        std::lock_guard<std::mutex> lock(mtx);
        generator = g;
    }
};
