#ifndef BATTLESHIP_CORE_CELL_H_
#define BATTLESHIP_CORE_CELL_H_

enum class CellState {
    UNKNOWN,
    EMPTY,
    SHIP
};

class Cell {
public:
    Cell(CellState state);
    Cell(const Cell& other);
    Cell& operator=(const Cell& other);
    
    bool IsUnknown() const;
    bool IsEmpty() const;
    bool IsShip() const;

    bool CanBeShot() const;
    void reset();

    void set_unknown();
    void set_empty();
    void set_ship(int ind, int number);    

    int ship_index() const;
    void set_ship_index(int index);

    int segment_index() const;
    void set_segment_index(int index);

    CellState segment_state() const;
    void set_state(CellState new_state);

private:
    CellState state_;
    int ship_index_;
    int segment_index_;
};

#endif