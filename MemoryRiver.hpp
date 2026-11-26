#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>
#include <string>

template<class T, int info_len = 2>
class MemoryRiver {
    static_assert(sizeof(T) >= sizeof(int), "A free slot must hold an index");
    static_assert(info_len >= 0, "The header length must be nonnegative");

    std::fstream file;
    std::string file_name;
    // The public header is followed by one private int: the free-list head.
    static constexpr std::streamoff head_offset = info_len * sizeof(int);

    void open() {
        if (!file.is_open())
            file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    }

    void read_bytes(std::streamoff offset, void *value, std::streamsize size) {
        file.seekg(offset);
        file.read(static_cast<char *>(value), size);
    }

    void write_bytes(std::streamoff offset, const void *value,
                     std::streamsize size) {
        file.seekp(offset);
        file.write(static_cast<const char *>(value), size);
    }

public:
    MemoryRiver() = default;
    MemoryRiver(const std::string &name) : file_name(name) {}

    void initialise(std::string FN = "") {
        if (file.is_open()) file.close();
        file.clear();
        if (!FN.empty()) file_name = FN;
        std::ofstream output(file_name, std::ios::binary | std::ios::trunc);
        const int zero = 0;
        for (int i = 0; i <= info_len; ++i)
            output.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
    }

    void get_info(int &tmp, int n) {
        if (n < 1 || n > info_len) return;
        open();
        read_bytes((n - 1) * sizeof(int), &tmp, sizeof(tmp));
    }

    void write_info(int tmp, int n) {
        if (n < 1 || n > info_len) return;
        open();
        write_bytes((n - 1) * sizeof(int), &tmp, sizeof(tmp));
        file.flush();
    }

    int write(T &t) {
        open();
        int index = 0;
        read_bytes(head_offset, &index, sizeof(index));
        if (index != 0) {
            int next = 0;
            read_bytes(index, &next, sizeof(next));
            write_bytes(head_offset, &next, sizeof(next));
        } else {
            file.seekp(0, std::ios::end);
            index = static_cast<int>(file.tellp());
        }
        write_bytes(index, &t, sizeof(T));
        file.flush();
        return index;
    }

    void update(T &t, const int index) {
        open();
        write_bytes(index, &t, sizeof(T));
        file.flush();
    }

    void read(T &t, const int index) {
        open();
        read_bytes(index, &t, sizeof(T));
    }

    void Delete(int index) {
        open();
        int head = 0;
        read_bytes(head_offset, &head, sizeof(head));
        // Only deleted records are overwritten with free-list links.
        write_bytes(index, &head, sizeof(head));
        write_bytes(head_offset, &index, sizeof(index));
        file.flush();
    }
};

#endif
