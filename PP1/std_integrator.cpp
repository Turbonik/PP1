#include "std_integrator.h"

#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <iostream>
#include <cmath>

// Подынтегральная функция  
static long double f_std(long double x) {
    return x * x * x - x * x;
}

// Функция потока 
static void compute_part_std(long double a,
    long double h,
    int start_segment,
    int end_segment,
    std::promise<long double>* prom)
{
    long double local_sum = 0.0L;

    for (int seg = start_segment; seg < end_segment; ++seg) {
        int i = seg * 3;
        long double x0 = a + i * h;
        long double x1 = x0 + h;
        long double x2 = x1 + h;
        long double x3 = x2 + h;
        local_sum += (3.0L * h / 8.0L) * (f_std(x0) + 3.0L * f_std(x1) + 3.0L * f_std(x2) + f_std(x3));
    }

    // Устанавливаем значение в promise
    prom->set_value(local_sum);
}

void integrate_stdthreads(long double a,
    long double b,
    int num_intervals,
    int num_threads,
    long double& result,
    long double& elapsed_seconds)
{
    result = 0.0L;
    elapsed_seconds = 0.0L;

    if (num_intervals <= 0 || num_intervals % 3 != 0 || b <= a) {
        return;
    }

    long double h = (b - a) / static_cast<long double>(num_intervals);
    int num_segments = num_intervals / 3;

    if (num_threads > num_segments) num_threads = num_segments;
    if (num_threads < 1) num_threads = 1;

    int segments_per_thread = num_segments / num_threads;
    int remainder = num_segments % num_threads;
    int current = 0;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    // promises + futures для получения результата от потоков
    std::vector<std::promise<long double>> promises(num_threads);
    std::vector<std::future<long double>> futures;
    futures.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        futures.push_back(promises[i].get_future());
    }

    auto t_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        int cnt = segments_per_thread + (i < remainder ? 1 : 0);
        int start_segment = current;
        int end_segment = current + cnt;
        current += cnt;

        // создаём поток, передавая все параметры по отдельности 
        threads.emplace_back(compute_part_std,
            a,
            h,
            start_segment,
            end_segment,
            &promises[i]);
    }

    // собираем результаты
    result = 0.0L;
    for (int i = 0; i < (int)threads.size(); ++i) {
        
        if (threads[i].joinable()) threads[i].join();

     
        long double part = 0.0L;
        try {
            part = futures[i].get();
        }
        catch (const std::future_error& fe) {
            std::cerr << "future_error: " << fe.what() << std::endl;
            part = 0.0L;
        }
        result += part;
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    elapsed_seconds = std::chrono::duration<long double>(t_end - t_start).count();
}
