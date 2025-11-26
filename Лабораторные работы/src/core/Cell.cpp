#include "Cell.h"

Cell::Cell(CellState state) : state_(state), ship_index_(-1), segment_index_(-1) {}


Cell::Cell(const Cell& other) : state_(other.state_), ship_index_(other.ship_index_), segment_index_(other.segment_index_) {}


Cell& Cell::operator=(const Cell& other) {
    if (this != &other) {
        state_ = other.state_;
        ship_index_ = other.ship_index_;
        segment_index_ = other.segment_index_;
    }
    return *this;
}


bool Cell::IsUnknown() const { 
    return state_ == CellState::UNKNOWN; 
}


bool Cell::IsEmpty() const { 
    return state_ == CellState::EMPTY; 
}


bool Cell::IsShip() const { 
    return state_ == CellState::SHIP; 
}


bool Cell::CanBeShot() const {
    return state_ == CellState::UNKNOWN || state_ == CellState::SHIP;
}


void Cell::reset() {
    state_ = CellState::EMPTY;
    ship_index_ = -1;
    segment_index_ = -1;
}


void Cell::set_unknown() {
    state_ = CellState::UNKNOWN;
    segment_index_ = -2;
    ship_index_ = -2;
}


void Cell::set_empty() {
    state_ = CellState::EMPTY;
    ship_index_ = -1;
    segment_index_ = -1;
}


void Cell::set_ship(int ind, int number) {
    state_ = CellState::SHIP;
    segment_index_ = ind;
    ship_index_ = number;
}


int Cell::ship_index() const {
    return ship_index_;
}


void Cell::set_ship_index(int index) { 
    ship_index_ = index; 
};


int Cell::segment_index() const {
    return segment_index_;
}


void Cell::set_segment_index(int index){
    segment_index_ = index;
}


CellState Cell::segment_state() const { 
    return state_; 
}


void Cell::set_state(CellState new_state){
    state_ = new_state;
}