#pragma once
#include <optional>
#include <stdexcept>
#include <memory>
#include "LazySequence.h"

template <typename T>
class Generator
{
private:
    LazySequence<T> *owner;
    size_t index;

public:
    explicit Generator(LazySequence<T> *owner_, size_t startIndex = 0) : owner(owner_), index(startIndex)
    {
        if (!owner)
            throw std::invalid_argument("Generator: owner is null");
    }

    T GetNext()
    {
        if (!owner)
            throw std::runtime_error("Generator has no owner");
        T v = owner->Get(index);
        ++index;
        return v;
    }

    bool HasNext() const
    {
        if (!owner)
            return false;
        if (owner->HasGenerator())
            return true;
        return index < owner->GetMaterializedCount();
    }

    std::optional<T> TryGetNext()
    {
        if (!HasNext())
            return std::nullopt;
        return GetNext();
    }

    Generator<T> *Append(const T &item) const
    {
        if (!owner)
            throw std::runtime_error("Append: no owner");
        owner->Append(item);
        return const_cast<Generator<T> *>(this);
    }

    Generator<T> *Append(const LazySequence<T> &items) const
    {
        if (!owner)
            throw std::runtime_error("Append(seq): no owner");
        owner->Concat(items);
        return const_cast<Generator<T> *>(this);
    }

    Generator<T> *Insert(const T &item) const
    {
        if (!owner)
            throw std::runtime_error("Insert: no owner");
        owner->InsertAt(item, index);
        return const_cast<Generator<T> *>(this);
    }

    Generator<T> *Remove(const T &item) const
    {
        if (!owner)
            throw std::runtime_error("Remove: no owner");
        owner->RemoveValue(item);
        return const_cast<Generator<T> *>(this);
    }

    size_t GetPosition() const { return index; }
    void Reset(size_t newPos = 0) { index = newPos; }
};

template <typename T>
Generator<T> LazySequence<T>::CreateGenerator(size_t startIndex)
{
    return Generator<T>(this, startIndex);
}
