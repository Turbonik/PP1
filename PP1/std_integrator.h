
#ifndef STD_INTEGRATOR_H
#define STD_INTEGRATOR_H

void integrate_stdthreads(long double a,
    long double b,
    int num_intervals,
    int num_threads,
    long double& result,
    long double& elapsed_seconds);

#endif 