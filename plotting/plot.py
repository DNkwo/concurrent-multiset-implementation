import matplotlib.pyplot as plt

# Sample data
threads = [1, 2, 4, 8, 16]
throughput_lock = [1000, 1900, 3500, 4800, 5000]  
throughput_optimistic = [1000, 2000, 4000, 7900, 15800] 
throughput_lock_free = [1000, 2100, 4200, 8400, 16800] 

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(threads, throughput_lock, marker='o', label='Single Lock')
plt.plot(threads, throughput_optimistic, marker='s', label='Optimistic')
plt.plot(threads, throughput_lock_free, marker='^', label='Lock Free')

# Labeling
plt.xlabel('Number of Threads')
plt.ylabel('Throughput (ops/sec)')
plt.title('Throughput vs. Number of Threads')
plt.legend()
plt.grid(True)

# Show plot
plt.show()