import matplotlib.pyplot as plt


#data compiled into an arrays :)
single_lock_50_50 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [331436, 347713, 344217, 345192, 338710, 339758, 339349, 338997],
    'latency': [0.000786177, 0.00396647, 0.00805666, 0.0120796, 0.0164337, 0.020467, 0.0245776, 0.0286201]
}

optimistic_sync_50_50 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [260006, 548819, 795504, 789245, 788527, 783027, 766889, 774173],
    'latency': [0.00101739, 0.00247372, 0.00341029, 0.00496758, 0.00652125, 0.00796424, 0.00744981, 0.00687366]
}

lock_free_50_50 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [646013, 2006330, 2592890, 2721310, 2571700, 2754920, 2701540, 2808740],
    'latency': [0.000372885, 0.000590152, 0.000830036, 0.00105262, 0.00117439, 0.00107173, 0.000853811, 0.000757914]
}


single_lock_80_20 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [335974, 338260, 339388, 333141, 326897, 333365, 325419, 329048],
    'latency': [0.000773757, 0.00408503, 0.00820168, 0.0125504, 0.0170249, 0.0208938, 0.025648, 0.0296864]
}

optimistic_sync_80_20 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [254482, 563896, 812786, 794761, 811085, 789060, 770529, 781395],
    'latency': [0.00103923, 0.00240075, 0.00335057, 0.00491237, 0.00649947, 0.00801486, 0.007463, 0.00810371]
}

lock_free_80_20 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [654742, 2019250, 2634610, 2682080, 2670350, 2782910, 2804840, 2778370],
    'latency': [0.000366858, 0.000585253, 0.000821671, 0.00106829, 0.00102317, 0.00107091, 0.000853744, 0.000847854]
}


single_lock_20_80 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [335321, 341667, 331407, 336574, 333794, 336320, 335391, 335551],
    'latency': [0.000774452, 0.00401965, 0.00837063, 0.0123725, 0.0166623, 0.0206648, 0.0248609, 0.0289662]
}

optimistic_sync_20_80 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [224370, 511954, 729529, 728610, 735686, 723162, 708026, 711350],
    'latency': [0.00118987, 0.00264163, 0.00371172, 0.00541072, 0.00730744, 0.00857769, 0.00859281, 0.00915816]
}

lock_free_20_80 = {
    'threads': [2, 10, 20, 30, 40, 50, 60, 70],
    'throughput': [626011, 2064700, 2623590, 2845710, 2629140, 2792570, 2746190, 2713580],
    'latency': [0.000378341, 0.000584939, 0.000813189, 0.00116064, 0.000950099, 0.000958218, 0.000840077, 0.000960329]
}

#current selected configuration
single_lock = single_lock_50_50
optimistic_sync = optimistic_sync_50_50
lock_free = lock_free_50_50

# experimented with differently figure sizes, this is the best
plt.figure(figsize=(14, 8))

# throughput plot
plt.subplot(1, 2, 1)
plt.plot(single_lock['threads'], single_lock['throughput'], label='Single Lock', marker='o')
plt.plot(optimistic_sync['threads'], optimistic_sync['throughput'], label='Optimistic Synchronization', marker='s')
plt.plot(lock_free['threads'], lock_free['throughput'], label='Lock-Free Synchronization', marker='^')
plt.title('Throughput vs. Number of Threads')
plt.xlabel('Number of Threads')
plt.ylabel('Throughput (million ops/sec)')
plt.legend()
plt.grid(True)

# latency plot
plt.subplot(1, 2, 2)
plt.plot(single_lock['threads'], single_lock['latency'], label='Single Lock', marker='o')
plt.plot(optimistic_sync['threads'], optimistic_sync['latency'], label='Optimistic Synchronization', marker='s')
plt.plot(lock_free['threads'], lock_free['latency'], label='Lock-Free Synchronization', marker='^')
plt.title('Latency vs. Number of Threads')
plt.xlabel('Number of Threads')
plt.ylabel('Average Latency (ms/ops)')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()