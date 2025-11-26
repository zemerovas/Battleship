#ifndef BATTLESHIP_CORE_SHIPMANAGER_H_
#define BATTLESHIP_CORE_SHIPMANAGER_H_
#include <vector>
#include <stdexcept>
#include <string>
#include <iostream>

class ShipManager {
public:
    ShipManager();
    ShipManager(int count, const std::vector<int>& sizes);
    ShipManager(const ShipManager& other);
    ShipManager& operator=(const ShipManager& other);    
    
    friend std::ostream& operator<<(std::ostream& os, const ShipManager& manager);
    friend std::istream& operator>>(std::istream& is, ShipManager& manager);

    void clear();
    
    int ship_count() const;
    int ship_size(int index) const;

private:
    int ship_count_;
    std::vector<int> ship_sizes_;
};

#endif