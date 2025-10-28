#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <optional> //значение, которое может быть или не быть

// ReadOnlyStream<T> — несколько конструкторов: из файла, из vector, из ленивого генератора (callable)
template <typename T>
class ReadOnlyStream
{
public:
    // конструктор: чтение из файла
    explicit ReadOnlyStream(const std::string &filename) : sourceType(Type::File), file(filename), vecPtr(nullptr), gen(nullptr)
    {
        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + filename);
    }

    // конструктор: чтение из вектора(“читает” уже готовый массив из памяти — как будто это файл, но без диска)
    explicit ReadOnlyStream(const std::vector<T> &v) : sourceType(Type::Vector), file(), vec(std::make_shared<std::vector<T>>(v)), vecPtr(vec.get()), gen(nullptr), vecPos(0) {}

    // конструктор: чтение из генератора(поток данных, который порождается функцией.Функция возвращает std::optional<T>:если есть новое значение → return value;если всё закончилось → return std::nullopt;)
    explicit ReadOnlyStream(std::function<std::optional<T>()> generator) : sourceType(Type::Generator), file(), vecPtr(nullptr), gen(generator) {}

    // поток, который читает данные с клавиатуры
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

    // говорит, что поток не закончился
    bool IsEndOfStream()
    {
        return false;
    }

private:
    // “пустой” конструктор, используют только из FromStdin()
    ReadOnlyStream() : sourceType(Type::Stdin), vecPtr(nullptr), gen(nullptr) {}

    enum class Type
    {
        File,
        Vector,
        Generator,
        Stdin
    };
    Type sourceType;
    std::ifstream file;                    // для чтения из файла
    std::shared_ptr<std::vector<T>> vec;   // чтобы хранить вектор
    std::vector<T> *vecPtr;                // указатель для удобства
    size_t vecPos = 0;                     // индекс следующего элемента в векторе
    std::function<std::optional<T>()> gen; // генератор функции
};
