import numpy as np 
import time
xmin = 0
xmax = 10
nx = 512 
dx = (xmax - xmin) / nx 
x = np.linspace(xmin, xmax, nx)
tmin = 0
tmax = 10
nt = 1000000
dt = (tmax - tmin) / nt 
times = np.linspace(tmin, tmax, nt)
y = np.array([np.e**(-(i - 5)**2) for i in x])
v = np.zeros_like(x)
a = np.zeros_like(x)
gamma = 1
def wave_simulation(times, x, y, v, a, gamma, dx, dt):
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
time_taken = runtime_analysis(wave_simulation, [times, x, y, v, a, gamma, dx, dt])
print(time_taken)