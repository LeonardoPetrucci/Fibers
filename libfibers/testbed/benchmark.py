import psutil
from subprocess import PIPE
import time
from matplotlib import pyplot


BASE_FIBERS = 3
ITERATIONS = 2
cpus = psutil.cpu_count()


def benchmark_fibers(exec_file, n):
    samples = 0
    cpu = 0.0
    cpu_times = 0.0
    memory = 0.0
    context_switch = 0

    test = psutil.Popen([exec_file, str(n)], stdout = PIPE)
    cpu += test.cpu_percent() / cpus
    memory += test.memory_percent()
    context_switch += test.num_ctx_switches().voluntary + test.num_ctx_switches().involuntary

    while test.poll() is None:
        time.sleep(1)
        cpu += test.cpu_percent() /cpus
        cpu_times += test.cpu_times().user + test.cpu_times().system + test.cpu_times().children_user + test.cpu_times().children_system
        memory += test.memory_percent()
        context_switch += test.num_ctx_switches().voluntary + test.num_ctx_switches().involuntary
        samples += 1


    cpu = cpu / samples
    cpu_times = cpu_times / samples
    memory = memory / samples
    context_switch = context_switch // samples

    output = test.stdout.readlines()[-1].decode("UTF-8")
    fibers_time = float(output.split(':')[-1].strip())

    return cpu, cpu_times, memory, context_switch, fibers_time


kernel_cpu_list = []
kernel_cpu_time_list = []
kernel_memory_list = []
kernel_context_switch_list = []
kernel_fibers_time_list = []

user_cpu_list = []
user_cpu_time_list = []
user_memory_list = []
user_context_switch_list = []
user_fibers_time_list = []

for i in range(1, ITERATIONS + 1):
    print("Iteration number " + str(i) + ", fibers used: " + str(BASE_FIBERS * i))
    cpu, cpu_time, memory, context_switch, fibers_time = benchmark_fibers("./kernel_test", BASE_FIBERS * i)
    kernel_cpu_list.append(cpu)
    kernel_cpu_time_list.append(cpu_time)
    kernel_memory_list.append(memory)
    kernel_context_switch_list.append(context_switch)
    kernel_fibers_time_list.append(fibers_time)

print("\n")
time.sleep(5)

for i in range(1, ITERATIONS + 1):
    print("Iteration number " + str(i) + ", fibers used: " + str(BASE_FIBERS * i))
    cpu, cpu_time, memory, context_switch, fibers_time = benchmark_fibers("./user_test", BASE_FIBERS * i)
    user_cpu_list.append(cpu)
    user_cpu_time_list.append(cpu_time)
    user_memory_list.append(memory)
    user_context_switch_list.append(context_switch)
    user_fibers_time_list.append(fibers_time)

pyplot.title("CPU usage (on average)")
pyplot.xlabel("Iteration")
pyplot.ylabel("CPU usage")
pyplot.plot(range(1, len(kernel_cpu_list)+1), [pt for pt in kernel_cpu_list], marker='o', linestyle='--', label="Kernel")
pyplot.plot(range(1, len(user_cpu_list)+1), [pt for pt in user_cpu_list], marker='o', linestyle='--', label="User")
pyplot.legend()
pyplot.show()

pyplot.title("CPU time (on average)")
pyplot.xlabel("Iteration")
pyplot.ylabel("CPU time")
pyplot.plot(range(1, len(kernel_cpu_time_list)+1), [pt for pt in kernel_cpu_time_list], marker='o', linestyle='--', label="Kernel")
pyplot.plot(range(1, len(user_cpu_time_list)+1), [pt for pt in user_cpu_time_list], marker='o', linestyle='--', label="User")
pyplot.legend()
pyplot.show()

pyplot.title("Memory usage (on average)")
pyplot.xlabel("Iteration")
pyplot.ylabel("Memory usage")
pyplot.plot(range(1, len(kernel_memory_list)+1), [pt for pt in kernel_memory_list], marker='o', linestyle='--', label="Kernel")
pyplot.plot(range(1, len(user_memory_list)+1), [pt for pt in user_memory_list], marker='o', linestyle='--', label="User")
pyplot.legend()
pyplot.show()

pyplot.title("Context switches (on average)")
pyplot.xlabel("Iteration")
pyplot.ylabel("Context switches")
pyplot.plot(range(1, len(kernel_context_switch_list)+1), [pt for pt in kernel_context_switch_list], marker='o', linestyle='--', label="Kernel")
pyplot.plot(range(1, len(user_context_switch_list)+1), [pt for pt in user_context_switch_list], marker='o', linestyle='--', label="User")
pyplot.legend()
pyplot.show()

pyplot.title("Fibers working time (on average)")
pyplot.xlabel("Iteration")
pyplot.ylabel("working time")
pyplot.plot(range(1, len(kernel_fibers_time_list)+1), [pt for pt in kernel_fibers_time_list], marker='o', linestyle='--', label="Kernel")
pyplot.plot(range(1, len(user_fibers_time_list)+1), [pt for pt in user_fibers_time_list], marker='o', linestyle='--', label="User")
pyplot.legend()
pyplot.show()