#include "Shelling.h"
#include "AbilityException.h"
#include "AbilityManager.h"
#include <algorithm>

Shelling::Shelling() : Ability("Shelling") { 
    description_ = "Наносит 1 урон случайному живому сегменту.";
}

std::vector<std::pair<int,int>> Shelling::CollectAliveTargets(AbilityManager& manager) const {
    std::vector<std::pair<int,int>> targets;
    targets.reserve(32);

    const int count_ = manager.ship_count();
    for (int i = 0; i < count_; ++i) {
        const Ship& ship = manager.ship(i);
        if (ship.IsDestroyed()) continue;

        const int sz = ship.ship_size();
        for (int seg = 0; seg < sz; ++seg) {
            const SegmentState st = ship.segment_state(seg);
            if (st == SegmentState::DESTROYED) continue;
            const Position p = ship.segment_position(seg);
            targets.emplace_back(p.x, p.y);
        }
    }
    return targets;
}

int Shelling::RandomIndex(int n) {
    static thread_local std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<> dist(0, n - 1);
    return dist(gen);
}

void Shelling::Use(AbilityManager& manager, int /*x*/, int /*y*/) {
    try {
        auto targets = CollectAliveTargets(manager);
        if (targets.empty()) {
            throw AbilityApplicationException(name(), "Нет доступных целей (вражеские корабли потоплены).");
        }

        const int maxTries = std::min<int>(5, static_cast<int>(targets.size()));

        for (int attempt = 0; attempt < maxTries; ++attempt) {
            const int idx = RandomIndex(static_cast<int>(targets.size()));
            auto [x, y] = targets[idx];

            try {
                manager.DamageEnemyField(x, y /*, 1 — по умолчанию*/);
                coord_.first  = x;
                coord_.second = y;
                return;
            } catch (const std::exception&) {
                // Убираем неудачную цель и пробуем ещё.
                targets.erase(targets.begin() + idx);
                if (targets.empty()) break;
            }
        }

        throw AbilityApplicationException(name(), "Не удалось нанести урон (цели стали недоступны).");

    } catch (const AbilityApplicationException&) {
        throw;
    } catch (const std::exception& e) {
        throw AbilityApplicationException(name(), e.what());
    }
}
