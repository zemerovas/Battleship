#include <iostream>
#include "Scanner.h"
#include "AbilityException.h"
#include "AbilityManager.h"

Scanner::Scanner() : Ability("Scanner") {
    description_ = "Показывает содержимое участка поля размером 2x2 клетки.";
    scan_range_ = 1;
}

void Scanner::Use(AbilityManager& manager, int x, int y) {
    try {
        coord_.first = x;
        coord_.second = y;
        auto results = ScanArea(manager, Position(x, y));
        for (const auto& result : results) {
            manager.set_enemy_cell_visible(result.first.x, result.first.y);
        }
    } catch (const std::exception& e) {
        throw AbilityApplicationException(name(), e.what());
    }
}

std::vector<std::pair<Position, bool>> Scanner::ScanArea(AbilityManager& manager, Position center) const {
    int x_size_ = manager.enemy_field_size_x();
    int y_size_ = manager.enemy_field_size_y();
    std::vector<std::pair<Position, bool>> result;
    for (int dy = -scan_range_; dy <= scan_range_; dy++) {
        for (int dx = -scan_range_; dx <= scan_range_; dx++) {
            int scanX = center.x + dx;
            int scanY = center.y + dy;
            if (IsValid(scanX, scanY, x_size_, y_size_)) {
                result.emplace_back(Position(scanX, scanY), manager.enemy_cell_state(scanX, scanY));
            }
        }
    }
    return result;
}

