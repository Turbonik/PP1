 
#include <iostream>
#include <iomanip>
#include <locale>
#include <vector>

#include "posix_integrator.h"
#include "std_integrator.h"

int main() {
    std::setlocale(LC_ALL, "");  

    long double a, b;
    int num_intervals;

    std::cout << "Введите нижний предел интегрирования (a): ";
    if (!(std::cin >> a)) return 1;
    std::cout << "Введите верхний предел интегрирования (b): ";
    if (!(std::cin >> b)) return 1;
    std::cout << "Введите количество интервалов (кратно 3): ";
    if (!(std::cin >> num_intervals)) return 1;

    if (num_intervals <= 0 || num_intervals % 3 != 0) {
        std::cerr << "Ошибка: количество интервалов должно быть положительным и кратно 3.\n";
        return 1;
    }
    if (b <= a) {
        std::cerr << "Ошибка: верхний предел должен быть больше нижнего.\n";
        return 1;
    }

    const int MAX_THREADS = 8;
    std::vector<double> posix_times; posix_times.reserve(MAX_THREADS);
    std::vector<double> std_times;   std_times.reserve(MAX_THREADS);

    // POSIX pthreads: таблица
    {
        std::cout << "\nРезультаты (POSIX pthreads, метод 3п Ньютона-Котеса):\n";
        std::cout << std::setw(8) << "Потоки"
            << std::setw(20) << "Время (мс)"
            << std::setw(30) << "Результат"
            << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        for (int t = 1; t <= MAX_THREADS; ++t) {
            long double res = 0.0L, tm = 0.0L;
            integrate_posix(a, b, num_intervals, t, res, tm);
            double tm_ms = static_cast<double>(tm * 1000.0L);
            posix_times.push_back(tm_ms);

            std::cout << std::setw(8) << t
                << std::setw(20) << std::fixed << std::setprecision(6) << tm_ms
                << std::setw(30) << std::setprecision(10) << (long double)res
                << std::endl;
        }
    }

    // std::thread: таблица
    {
        std::cout << "\nРезультаты (std::thread, метод 3п Ньютона-Котеса):\n";
        std::cout << std::setw(8) << "Потоки"
            << std::setw(20) << "Время (мс)"
            << std::setw(30) << "Результат"
            << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        for (int t = 1; t <= MAX_THREADS; ++t) {
            long double res = 0.0L, tm = 0.0L;
            integrate_stdthreads(a, b, num_intervals, t, res, tm);
            double tm_ms = static_cast<double>(tm * 1000.0L);
            std_times.push_back(tm_ms);

            std::cout << std::setw(8) << t
                << std::setw(20) << std::fixed << std::setprecision(6) << tm_ms
                << std::setw(30) << std::setprecision(10) << (long double)res
                << std::endl;
        }
    }

    // Печать массивов в формате Python 
    std::cout << "\n# --- Python arrays (copy-paste these lines into your Python script) ---\n";

    // threads array
    std::cout << "threads = [";
    for (int i = 1; i <= (int)posix_times.size(); ++i) {
        std::cout << i;
        if (i != (int)posix_times.size()) std::cout << ", ";
    }
    std::cout << "]\n";

    // posix_times
    std::cout << "posix_times = [";
    for (size_t i = 0; i < posix_times.size(); ++i) {
        std::cout << std::fixed << std::setprecision(6) << posix_times[i];
        if (i + 1 != posix_times.size()) std::cout << ", ";
    }
    std::cout << "]\n";

    // std_times
    std::cout << "std_times = [";
    for (size_t i = 0; i < std_times.size(); ++i) {
        std::cout << std::fixed << std::setprecision(6) << std_times[i];
        if (i + 1 != std_times.size()) std::cout << ", ";
    }
    std::cout << "]\n";

    std::cout << "# -------------------------------------------------------------------\n";

    return 0;
}
