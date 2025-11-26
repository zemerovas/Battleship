#ifndef BATTLESHIP_CORE_SHIP_H_
#define BATTLESHIP_CORE_SHIP_H_
#include <vector>
#include <iostream>
#include <tuple>
#include <string>
#include "abilities/AbilityManager.h"


enum class SegmentState {
    INTACT,
    DAMAGED, 
    DESTROYED,
    NONE  
};


enum class Orientation {
    VERTICAL,
    HORIZONTAL
};


struct Position {
    int x;
    int y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
};


class Ship {
public:
    Ship(Position start, Orientation orient, int size, int number);
    Ship(const Ship& other);
    Ship(Ship&& other) noexcept;
    Ship& operator=(const Ship& other);
    Ship& operator=(Ship&& other) noexcept;
    ~Ship();

    int DamageShip(Position position_for_damage, int damage = 1);
    void Hit(int segment_index, int damage);
    bool IsDestroyed() const;
    void MarkFullyDestroyed();

    int ship_size() const;
    int segment_index(Position current) const;
    Position segment_position(int index) const;
    const std::vector<SegmentState>& segments() const;

    Position start_position() const;
    void set_start_position(Position pos);

    Orientation orientation() const;
    void set_orientation(Orientation orientation);

    int ship_number() const;
    void set_ship_number(int number);

    SegmentState segment_state(int index) const;
    void set_segment_state(int index, SegmentState state);

    int destroyed_segments() const;
    void set_destroyed_segments(int count);
    int hit_count() const;
    void set_hit_count(int count);

    std::tuple<int, SegmentState, Orientation> segment_state(int x, int y) const; 

private:
    Position start_position_;
    int ship_size_;
    Orientation ship_orientation_;
    std::vector<SegmentState> segments_;
    int ship_number_;

    int destroyed_segments_; 
    int hit_count_;
};

#endif
