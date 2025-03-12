import numpy as np 
import time

def wave_simulation(tmin, tmax, nt, xmin, xmax, nx, gamma):
    dx = (xmax - xmin) / nx 
    x = np.linspace(xmin, xmax, nx)
    dt = (tmax - tmin) / nt 
    times = np.linspace(tmin, tmax, nt)
    y = np.array([np.e**(-(i - 5)**2) for i in x])
    v = np.zeros_like(x)
    a = np.zeros_like(x)
    gamma_dx2 = gamma / (dx ** 2)
    for t_idx in range(len(times)):
            a[1:-1] = (y[2:] + y[0:-2] - 2 * y[1:-1]) * gamma_dx2
            y += v * dt
            v += a * dt
def runtime_analysis(func,args):
    start_time = time.time()
    func(*args)
    end_time = time.time()
    return end_time - start_time
times = []
for i in range(10):
    time_taken = runtime_analysis(wave_simulation, [0, 10, 1000000, 0, 10, 500, 1])
    times.append(time_taken)
print(np.mean(times))