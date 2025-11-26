#ifndef BATTLESHIP_CORE_PLAYINGFIELD_H_
#define BATTLESHIP_CORE_PLAYINGFIELD_H_

#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include "Ship.h"
#include "Cell.h"
#include "ShipManager.h"
#include <functional> 



class PlayingField {
public:
    PlayingField(int x = 10, int y = 10);
    PlayingField(const PlayingField& other);       
    PlayingField& operator=(const PlayingField& other); 
    PlayingField(PlayingField&& other) noexcept;
    PlayingField& operator=(PlayingField&& other) noexcept;

    ~PlayingField() = default;

    friend std::ostream& operator<<(std::ostream& os, const PlayingField& field);
    friend std::istream& operator>>(std::istream& is, PlayingField& field);

    bool SetRandomShips(const ShipManager& manager, size_t max_attempts = 10000);
    void MoveShip(int x, int y, int size, Orientation orientation);
    bool PlaceRemovedShip(int x, int y, int size, Orientation orientation);
    bool PlaceNewShip(int x, int y, int size, Orientation orientation);
    void PlaceShipOnGrid(const Ship& ship_to_place);
    bool PlaceShip(int x, int y, int size, Orientation orientation);

    void RotateOrientation();
    Orientation current_orientation() const;

    int Damage(int x, int y, int Damage = 1);

    bool IsShipCell(int x, int y) const;
    bool IsScanned(int x, int y) const;
    bool IsAllShipsDestroyed() const;
    bool HasShipsToReplace() const;

    void ClearField();
    void ClearShip(int x, int y);
    void ReturnStartState();
    void UpdateShipNumbersAfterRemoval(int removed_index);

    void save(std::ostream& out) const;
    void load(std::istream& in);

    int count() const;
    int x_size() const;
    int y_size() const;
    int removed_ship_size() const;
    
    const Cell& visible_cell(int x, int y) const;
    const Ship& ship(int index) const;
    
    void set_cell_visible(int x, int y);

    bool ship_info_at(int x, int y, int& ship_index, int& segment_index, int& ship_size,
                        SegmentState& segment_state, Orientation& orientation) const;

private:
    std::vector<std::vector<Cell>> real_grid_;
    std::vector<std::vector<Cell>> visible_grid_;
    std::vector<std::vector<bool>> scanned_overlay_;
    std::vector<Ship> ships_;
    std::vector<Ship> removed_ships_;

    int x_size_;
    int y_size_;
    int count_;
    bool is_in_replacement_mode_;
    Orientation current_orientation_ = Orientation::HORIZONTAL; 
};



#endif
