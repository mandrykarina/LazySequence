#pragma once
#include <queue>
#include <functional>
#include <limits>
#include <cmath>
#include <optional>

template <typename T>
class StatisticsCollector
{
public:
    StatisticsCollector() : count(0), sum(0.0), mean(0.0), m2(0.0), minVal(std::numeric_limits<T>::max()), maxVal(std::numeric_limits<T>::lowest()) {}

    // Добавить элемент и обновить все статистики онлайн
    void Add(const T &value)
    {
        ++count;
        double dv = static_cast<double>(value);
        sum += dv;

        // онлайн среднее и дисперсия (Welford)
        double delta = dv - mean;
        mean += delta / count;
        double delta2 = dv - mean;
        m2 += delta * delta2;

        // min/max
        if (value < minVal)
            minVal = value;
        if (value > maxVal)
            maxVal = value;

        // медиана — две кучи
        if (lower.empty() || value <= lower.top())
        {
            lower.push(value);
        }
        else
        {
            upper.push(value);
        }
        // баланс
        if (lower.size() > upper.size() + 1)
        {
            upper.push(lower.top());
            lower.pop();
        }
        else if (upper.size() > lower.size())
        {
            lower.push(upper.top());
            upper.pop();
        }
    }

    size_t GetCount() const { return count; }
    // среднее
    double GetAverage() const { return count ? sum / static_cast<double>(count) : 0.0; }
    // дисперсия
    std::optional<double> GetVariance() const { return (count > 1) ? std::optional<double>(m2 / (count - 1)) : std::nullopt; }
    // стандартное отклонение
    std::optional<double> GetStdDev() const
    {
        auto v = GetVariance();
        return v ? std::optional<double>(std::sqrt(*v)) : std::nullopt;
    }
    T GetMin() const { return minVal; }
    T GetMax() const { return maxVal; }

    // Медиана как double (если нечётное - середина lower.top(), если чётное - среднее двух)
    std::optional<double> GetMedian() const
    {
        if (count == 0)
            return std::nullopt;
        if (lower.size() == upper.size())
        {
            double a = static_cast<double>(lower.top());
            double b = static_cast<double>(upper.top());
            return (a + b) / 2.0;
        }
        else
        {
            return static_cast<double>(lower.top());
        }
    }

private:
    size_t count;                                                  // сколько элементов
    double sum;                                                    // сумма всех значений
    double mean, m2;                                               // для среднего и дисперсии
    T minVal, maxVal;                                              // минимумы и максимумы
    std::priority_queue<T> lower;                                  // куча-максимум
    std::priority_queue<T, std::vector<T>, std::greater<T>> upper; // куча-минимум
};
