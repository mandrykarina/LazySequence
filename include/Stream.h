#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <optional>

// ReadOnlyStream<T> — несколько конструкторов: из файла, из vector, из ленивого генератора (callable)
template <typename T>
class ReadOnlyStream
{
public:
    // from file: items separated by whitespace, one by one
    explicit ReadOnlyStream(const std::string &filename)
        : sourceType(Type::File), file(filename), vecPtr(nullptr), gen(nullptr)
    {
        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + filename);
    }

    // from vector (in-memory)
    explicit ReadOnlyStream(const std::vector<T> &v)
        : sourceType(Type::Vector), file(), vec(std::make_shared<std::vector<T>>(v)), vecPtr(vec.get()), gen(nullptr), vecPos(0)
    {
    }

    // from generator function: std::function<std::optional<T>()> returning next value or nullopt for end
    explicit ReadOnlyStream(std::function<std::optional<T>()> generator)
        : sourceType(Type::Generator), file(), vecPtr(nullptr), gen(generator)
    {
    }

    // from stdin
    static ReadOnlyStream<T> FromStdin()
    {
        ReadOnlyStream<T> s;
        s.sourceType = Type::Stdin;
        return s;
    }

    // TryRead: возвращает true и записывает в out, если есть элемент; иначе false
    bool TryRead(T &out)
    {
        switch (sourceType)
        {
        case Type::File:
            if (!(file >> out))
                return false;
            return true;
        case Type::Vector:
            if (!vecPtr)
                return false;
            if (vecPos >= vecPtr->size())
                return false;
            out = (*vecPtr)[vecPos++];
            return true;
        case Type::Generator:
            if (!gen)
                return false;
            {
                std::optional<T> maybe = gen();
                if (!maybe.has_value())
                    return false;
                out = *maybe;
                return true;
            }
        case Type::Stdin:
            if (!(std::cin >> out))
                return false;
            return true;
        }
        return false;
    }

    bool IsEndOfStream()
    {
        // best-effort
        return false;
    }

private:
    ReadOnlyStream() : sourceType(Type::Stdin), vecPtr(nullptr), gen(nullptr) {}

    enum class Type
    {
        File,
        Vector,
        Generator,
        Stdin
    };
    Type sourceType;
    std::ifstream file;
    std::shared_ptr<std::vector<T>> vec;
    std::vector<T> *vecPtr;
    size_t vecPos = 0;
    std::function<std::optional<T>()> gen;
};
