#ifndef BATTLESHIP_CONTROLGAME__FILEHANDLER_H_
#define BATTLESHIP_CONTROLGAME_FILEHANDLER_H_
#include <fstream>
#include <stdexcept>
#include <string>

class FileHandler {
    std::fstream fs;
public:
    FileHandler(const std::string& path, std::ios::openmode mode)
        : fs(path, mode) {
        if (!fs.is_open()) {
            throw std::runtime_error("Не удалось открыть файл: " + path);
        }
    }

    ~FileHandler() noexcept {
        if (fs.is_open()) fs.close();
    }

    std::fstream& get() { return fs; }

    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;

    FileHandler(FileHandler&& other) noexcept : fs(std::move(other.fs)) {}
    FileHandler& operator=(FileHandler&& other) noexcept {
        if (this != &other) {
            if (fs.is_open()) fs.close();
            fs = std::move(other.fs);
        }
        return *this;
    }
};

#endif