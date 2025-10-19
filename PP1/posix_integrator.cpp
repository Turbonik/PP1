#include "posix_integrator.h"

#include <vector>
#include <chrono>
#include "pthread.h"
#include <iostream>
#include <cmath>
#pragma comment(lib,"pthreadVCE2.lib")
// Подынтегральная функция
static long double f_posix(long double x) {
    return x * x * x - x * x;
}

// Внутренняя структура 
struct ThreadArgs {
    long double a;
    long double h;
    int start_segment;
    int end_segment;
};

// Функция потока (POSIX)
static void* compute_part_posix(void* arg) {
    ThreadArgs* p = static_cast<ThreadArgs*>(arg);
    long double local_sum = 0.0;

    for (int seg = p->start_segment; seg < p->end_segment; ++seg) {
        int i = seg * 3;
        long double x0 = p->a + i * p->h;
        long double x1 = x0 + p->h;
        long double x2 = x1 + p->h;
        long double x3 = x2 + p->h;
        local_sum += (3.0L * p->h / 8.0L) * (f_posix(x0) + 3.0L * f_posix(x1) + 3.0L * f_posix(x2) + f_posix(x3));
    }

    long double* ret = new long double(local_sum);
    return static_cast<void*>(ret);
}

void integrate_posix(long double a,
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

    std::vector<pthread_t> threads(num_threads);
    std::vector<ThreadArgs> args(num_threads);

    int segments_per_thread = num_segments / num_threads;
    int remainder = num_segments % num_threads;
    int current = 0;

    for (int i = 0; i < num_threads; ++i) {
        int cnt = segments_per_thread + (i < remainder ? 1 : 0);
        args[i].a = a;
        args[i].h = h;
        args[i].start_segment = current;
        args[i].end_segment = current + cnt;
        current += cnt;
    }

    auto t_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        if (pthread_create(&threads[i], nullptr, compute_part_posix, &args[i]) != 0) {
            std::cerr << "Ошибка создания потока POSIX: " << i << std::endl;
         
            for (int j = 0; j < i; ++j) pthread_join(threads[j], nullptr);
            return;
        }
    }

    result = 0.0L;
    for (int i = 0; i < num_threads; ++i) {
        void* ret_ptr = nullptr;
        if (pthread_join(threads[i], &ret_ptr) != 0) {
            std::cerr << "Ошибка pthread_join для потока " << i << std::endl;
            continue;
        }
        if (ret_ptr) {
            long double* part_sum = static_cast<long double*>(ret_ptr);
            result += *part_sum;
            delete part_sum;
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    elapsed_seconds = std::chrono::duration<long double>(t_end - t_start).count();
}
