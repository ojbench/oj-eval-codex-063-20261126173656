#include "MemoryRiver.hpp"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <map>
#include <random>
#include <sys/stat.h>

struct Record {
    int key;
    int value;
    char payload[32];
};

int main() {
    const char *name = "memory_river_test.bin";
    std::map<int, Record> expected;
    std::mt19937 random(1337);
    std::size_t peak = 0;
    {
        MemoryRiver<Record, 5> river;
        river.initialise(name);
        for (int i = 1; i <= 5; ++i) {
            int value = -1;
            river.get_info(value, i);
            assert(value == 0);
            river.write_info(i * 17, i);
        }
    }
    for (int batch = 0; batch < 100; ++batch) {
        MemoryRiver<Record, 5> river(name);
        for (int step = 0; step < 1000; ++step) {
            unsigned op = random() % 4;
            if (expected.empty() || op == 0) {
                Record value{};
                value.key = static_cast<int>(random());
                value.value = static_cast<int>(random());
                for (char &c : value.payload) c = static_cast<char>(random());
                int index = river.write(value);
                assert(expected.emplace(index, value).second);
                if (expected.size() > peak) peak = expected.size();
            } else {
                auto it = expected.begin();
                std::advance(it, random() % expected.size());
                if (op == 1) {
                    river.Delete(it->first);
                    expected.erase(it);
                } else if (op == 2) {
                    it->second.value = static_cast<int>(random());
                    river.update(it->second, it->first);
                } else {
                    Record actual{};
                    river.read(actual, it->first);
                    assert(actual.key == it->second.key);
                    assert(actual.value == it->second.value);
                    for (int i = 0; i < 32; ++i)
                        assert(actual.payload[i] == it->second.payload[i]);
                }
            }
        }
        for (int i = 1; i <= 5; ++i) {
            int value = 0;
            river.get_info(value, i);
            assert(value == i * 17);
        }
        struct stat status{};
        assert(stat(name, &status) == 0);
        assert(static_cast<std::size_t>(status.st_size) ==
               6 * sizeof(int) + peak * sizeof(Record));
    }
    {
        MemoryRiver<Record, 5> river(name);
        river.initialise();
        Record value{};
        int index = river.write(value);
        river.Delete(index);
        MemoryRiver<Record, 5> other(name);
        assert(other.write(value) == index);
        value.key = 42;
        other.update(value, index);
        river.read(value, index);
        assert(value.key == 42);
    }
    std::remove(name);
    std::cout << "All MemoryRiver tests passed\n";
}
