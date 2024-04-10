#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <queue>
#include <chrono>

using namespace std;

// Constants
const int MAX_CACHE_ENTRIES = 1024 * 1024; // Assuming each Trace object can fit into 1KB, for a 1GB cache
const int DRAM_WRITE_LATENCY = 0;
const int SSD_WRITE_LATENCY = 200;
const int SSD_READ_LATENCY = 300;

// Trace entry structure
struct Trace {
    chrono::microseconds timestamp;
    string type;
    long long offset;
    long long size;
    int response_time;
};

// Parsing the trace file
vector<Trace> parseTraceFile(const string& filename) {
    vector<Trace> traces;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open the file: " << filename << endl;
        return traces;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string item;
        vector<string> items;

        while (getline(ss, item, ',')) {
            items.push_back(item);
        }

        if (items.size() == 8) {
            try {
                Trace trace;
                trace.timestamp = chrono::microseconds(stoll(items[0]));
                trace.type = items[3];
                trace.offset = stoll(items[4]);
                trace.size = stoll(items[5]);
                trace.response_time = stoi(items[6]);
                traces.push_back(trace);
            } catch (const std::exception& e) {
                cerr << "Error parsing line: " << line << ". Exception: " << e.what() << endl;
            }
        }
    }

    return traces;
}

// Simulate LRU Cache
double simulateLRUCache(const vector<Trace>& traces) {
    unordered_map<long long, list<Trace>::iterator> cache; // Maps offset to an iterator in the LRU list
    list<Trace> lru_list; // List to maintain LRU order
    int total_reads = 0;
    int cache_hits = 0;

    for (const auto& trace : traces) {
        if (trace.type == "Read") {
            total_reads++;

            if (cache.find(trace.offset) != cache.end()) {
                // Cache hit
                cache_hits++;
                lru_list.erase(cache[trace.offset]);
                lru_list.push_front(trace);
                cache[trace.offset] = lru_list.begin();
            } else {
                // Cache miss
                if (cache.size() >= MAX_CACHE_ENTRIES) {
                    // Cache is full, evict the least recently used item
                    cache.erase(lru_list.back().offset);
                    lru_list.pop_back();
                }
                lru_list.push_front(trace);
                cache[trace.offset] = lru_list.begin();
            }
        }
    }

    double hit_ratio = static_cast<double>(cache_hits) / total_reads;
    double avg_latency = hit_ratio * DRAM_WRITE_LATENCY + (1 - hit_ratio) * SSD_READ_LATENCY;
    return avg_latency;
}

// Simulate Frequent Read Cache
// Simulate Frequent Read Cache with Improved Handling
double simulateFrequentReadCache(const vector<Trace>& traces) {
    unordered_map<long long, int> read_counts; // Maps offset to its read count
    unordered_map<long long, bool> is_cached; // Tracks whether an offset is cached
    unordered_map<long long, int> cached_reads; // Tracks read counts for cached items
    priority_queue<pair<int, long long>, vector<pair<int, long long>>, greater<>> frequent_reads; // Min-heap for read counts and offsets

    int total_reads = 0;
    int cache_hits = 0;

    for (const auto& trace : traces) {
        if (trace.type == "Read") {
            total_reads++;
            read_counts[trace.offset]++;

            if (is_cached[trace.offset]) {
                // Cache hit
                cache_hits++;
                // Update the cached read count for this offset
                cached_reads[trace.offset] = read_counts[trace.offset];
            } else {
                // Cache miss
                if (frequent_reads.size() < MAX_CACHE_ENTRIES) {
                    // Cache is not full, add new item
                    frequent_reads.push({read_counts[trace.offset], trace.offset});
                    is_cached[trace.offset] = true;
                    cached_reads[trace.offset] = read_counts[trace.offset];
                } else {
                    // Check if the new item should replace the least frequently read cached item
                    auto least_frequent = frequent_reads.top();
                    if (read_counts[trace.offset] > least_frequent.first) {
                        // Replace the least frequently read item
                        is_cached[least_frequent.second] = false; // Remove old item from cache
                        cached_reads.erase(least_frequent.second);
                        frequent_reads.pop(); // Remove old item from the queue

                        frequent_reads.push({read_counts[trace.offset], trace.offset}); // Add new item
                        is_cached[trace.offset] = true; // Mark new item as cached
                        cached_reads[trace.offset] = read_counts[trace.offset];
                    }
                }
            }
        }

        // Periodically clean up the priority queue to remove stale entries
        if (total_reads % 1000 == 0) {
            auto temp_queue = frequent_reads;
            frequent_reads = decltype(frequent_reads)(); // Clear the queue

            while (!temp_queue.empty()) {
                auto item = temp_queue.top();
                temp_queue.pop();
                if (cached_reads.find(item.second) != cached_reads.end() && cached_reads[item.second] == item.first) {
                    frequent_reads.push(item); // Re-add valid items
                }
            }
        }
    }

    double hit_ratio = static_cast<double>(cache_hits) / total_reads;
    double avg_latency = hit_ratio * DRAM_WRITE_LATENCY + (1 - hit_ratio) * SSD_READ_LATENCY;
    return avg_latency;
}


int main() {
    string trace_file = "mds_1";
    vector<Trace> traces = parseTraceFile(trace_file);

    double lru_avg_latency = simulateLRUCache(traces);
    double frequent_read_avg_latency = simulateFrequentReadCache(traces);

    cout << "Average Read Latency (LRU Cache): " << lru_avg_latency << " us" << endl;
    cout << "Average Read Latency (Frequent Read Cache): " << frequent_read_avg_latency << " us" << endl;

    return 0;
}

