import csv
from collections import defaultdict
from heapq import heappush, heappop
from typing import List, Tuple

# Constants
MAX_CACHE_ENTRIES = 1024 * 512  # Assuming each Trace object can fit into 1KB, for a 1GB cache
DRAM_WRITE_LATENCY = 0
SSD_WRITE_LATENCY = 200
SSD_READ_LATENCY = 300

# Trace entry structure
class Trace:
    def __init__(self, timestamp: int, trace_type: str, offset: int, size: int, response_time: int):
        self.timestamp = timestamp
        self.type = trace_type
        self.offset = offset
        self.size = size
        self.response_time = response_time

# Parsing the trace file
def parse_trace_file(filename: str) -> List[Trace]:
    traces = []
    with open(filename, 'r') as file:
        csv_reader = csv.reader(file)
        for row in csv_reader:
            if len(row) == 8:
                try:
                    trace = Trace(int(row[0]), row[3], int(row[4]), int(row[5]), int(row[6]))
                    traces.append(trace)
                except ValueError:
                    print(f"Error parsing line: {row}")
    return traces

# Simulate LRU Cache
def simulate_lru_cache(traces: List[Trace]) -> float:
    cache = {}
    lru_list = []
    total_reads = 0
    cache_hits = 0

    for trace in traces:
        if trace.type == "Read":
            total_reads += 1

            if trace.offset in cache:
                # Cache hit
                cache_hits += 1
                lru_list.remove(trace.offset)
                lru_list.append(trace.offset)
            else:
                # Cache miss
                if len(cache) >= MAX_CACHE_ENTRIES:
                    # Cache is full, evict the least recently used item
                    evicted_offset = lru_list.pop(0)
                    del cache[evicted_offset]
                cache[trace.offset] = trace
                lru_list.append(trace.offset)

    hit_ratio = cache_hits / total_reads
    avg_latency = hit_ratio * DRAM_WRITE_LATENCY + (1 - hit_ratio) * SSD_READ_LATENCY
    return avg_latency

# Simulate Frequent Read Cache
def simulate_frequent_read_cache(traces: List[Trace]) -> float:
    read_counts = defaultdict(int)
    is_cached = defaultdict(bool)
    cached_reads = defaultdict(int)
    frequent_reads: List[Tuple[int, int]] = []  # Min-heap for read counts and offsets

    total_reads = 0
    cache_hits = 0

    for trace in traces:
        if trace.type == "Read":
            total_reads += 1
            read_counts[trace.offset] += 1

            if is_cached[trace.offset]:
                # Cache hit
                cache_hits += 1
                cached_reads[trace.offset] = read_counts[trace.offset]
            else:
                # Cache miss
                if len(frequent_reads) < MAX_CACHE_ENTRIES:
                    # Cache is not full, add new item
                    heappush(frequent_reads, (read_counts[trace.offset], trace.offset))
                    is_cached[trace.offset] = True
                    cached_reads[trace.offset] = read_counts[trace.offset]
                else:
                    # Check if the new item should replace the least frequently read cached item
                    least_frequent = frequent_reads[0]
                    if read_counts[trace.offset] > least_frequent[0]:
                        # Replace the least frequently read item
                        is_cached[least_frequent[1]] = False  # Remove old item from cache
                        del cached_reads[least_frequent[1]]
                        heappop(frequent_reads)  # Remove old item from the heap

                        heappush(frequent_reads, (read_counts[trace.offset], trace.offset))  # Add new item
                        is_cached[trace.offset] = True  # Mark new item as cached
                        cached_reads[trace.offset] = read_counts[trace.offset]

            # Periodically clean up the heap to remove stale entries
            if total_reads % 1000 == 0:
                temp_heap = frequent_reads.copy()
                frequent_reads.clear()

                while temp_heap:
                    item = heappop(temp_heap)
                    if cached_reads[item[1]] == item[0]:
                        heappush(frequent_reads, item)  # Re-add valid items

    hit_ratio = cache_hits / total_reads
    avg_latency = hit_ratio * DRAM_WRITE_LATENCY + (1 - hit_ratio) * SSD_READ_LATENCY
    return avg_latency

def main():
    trace_file = "mds_1"
    traces = parse_trace_file(trace_file)

    lru_avg_latency = simulate_lru_cache(traces)
    frequent_read_avg_latency = simulate_frequent_read_cache(traces)

    print(f"Average Read Latency (LRU Cache): {lru_avg_latency} us")
    print(f"Average Read Latency (Frequent Read Cache): {frequent_read_avg_latency} us")

if __name__ == "__main__":
    main()