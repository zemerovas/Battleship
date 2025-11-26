#include "Other.h"


bool IsValid(int x, int y, int x_size_, int y_size_) {
    return x >= 0 && x < x_size_ && y >= 0 && y < y_size_;
}

void Trim(std::string& str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(" \t\r\n"));
    auto pos = str.find_last_not_of(" \t\r\n");
    if (pos != std::string::npos) str.erase(pos + 1);
}

bool IsComment(const std::string& line) {
    return !line.empty() && (line[0] == '#' || line[0] == ';');
}

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string ColumnLabel(int index) {
    std::string s;
    int n = index + 1;
    while (n > 0) {
        int r = (n - 1) % 26;
        s.push_back(static_cast<char>('A' + r));
        n = (n - 1) / 26;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

int DigitsCount(int value) {
    if (value <= 0) return 1;
    int c = 0;
    while (value > 0) { value /= 10; ++c; }
    return c;
}


int MaxLabelWidthForX(int size) {
    auto LabelLen = [](int idx)->int{
        int n = 0; idx += 1;
        do { idx = (idx - 1) / 26; ++n; } while (idx > 0);
        return n;
    };
    int maxw = 1;
    for (int x = 0; x < size; ++x)
        maxw = std::max(maxw, LabelLen(x));
    return maxw;
}

