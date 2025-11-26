#include "PlayingField.h"
#include "additional/ShipCoordinateExceptions.h"
#include "additional/Other.h"
#include <iostream>
#include <iomanip>



PlayingField::PlayingField(int x, int y) : x_size_(x), y_size_(y), count_(0) {
    real_grid_.resize(y_size_, std::vector<Cell>(x_size_, Cell(CellState::EMPTY)));
    visible_grid_.resize(y_size_, std::vector<Cell>(x_size_, Cell(CellState::UNKNOWN)));
    scanned_overlay_.resize(y_size_, std::vector<bool>(x_size_, false));
    is_in_replacement_mode_ = false;
}


PlayingField::PlayingField(const PlayingField& other)
        : x_size_(other.x_size_)
        , y_size_(other.y_size_)
        , count_(other.count_)
        , is_in_replacement_mode_(other.is_in_replacement_mode_)
{
    real_grid_ = other.real_grid_;
    visible_grid_ = other.visible_grid_;
    scanned_overlay_ = other.scanned_overlay_;
    ships_ = other.ships_;          
    removed_ships_ = other.removed_ships_; 
}

PlayingField& PlayingField::operator=(const PlayingField& other) {
    if (this == &other) return *this;

    x_size_ = other.x_size_;
    y_size_ = other.y_size_;
    count_ = other.count_;
    is_in_replacement_mode_ = other.is_in_replacement_mode_;

    real_grid_ = other.real_grid_;
    visible_grid_ = other.visible_grid_;
    scanned_overlay_ = other.scanned_overlay_;
    ships_ = other.ships_;
    removed_ships_ = other.removed_ships_;

    return *this;
}

PlayingField::PlayingField(PlayingField&& other) noexcept
        : real_grid_(std::move(other.real_grid_))
        , visible_grid_(std::move(other.visible_grid_))
        , scanned_overlay_(std::move(other.scanned_overlay_))
        , ships_(std::move(other.ships_))
        , removed_ships_(std::move(other.removed_ships_))
        , x_size_(other.x_size_)
        , y_size_(other.y_size_)
        , count_(other.count_)
        , is_in_replacement_mode_(other.is_in_replacement_mode_)
{
    other.x_size_ = 0;
    other.y_size_ = 0;
    other.count_ = 0;
    other.is_in_replacement_mode_ = false;
}

PlayingField& PlayingField::operator=(PlayingField&& other) noexcept {
    if (this == &other) return *this;

    real_grid_ = std::move(other.real_grid_);
    visible_grid_ = std::move(other.visible_grid_);
    scanned_overlay_ = std::move(other.scanned_overlay_);
    ships_ = std::move(other.ships_);
    removed_ships_ = std::move(other.removed_ships_);

    x_size_ = other.x_size_;
    y_size_ = other.y_size_;
    count_ = other.count_;
    is_in_replacement_mode_ = other.is_in_replacement_mode_;

    other.x_size_ = 0;
    other.y_size_ = 0;
    other.count_ = 0;
    other.is_in_replacement_mode_ = false;
    
    return *this;
}

std::ostream& operator<<(std::ostream& os, const PlayingField& field) {
    field.save(os);
    return os;
}


std::istream& operator>>(std::istream& is, PlayingField& field) {
    field.load(is);
    return is;
}


void PlayingField::MoveShip(int x, int y, int size, Orientation orientation){
    std::vector<std::pair<int, int>> ship_cells;
    for (int i = 0; i < size; ++i) {
        int curr_x = (orientation == Orientation::HORIZONTAL) ? x + i : x;
        int curr_y = (orientation == Orientation::VERTICAL) ? y + i : y;

        if (!IsValid(curr_x, curr_y, x_size_, y_size_)) {
            throw ShipOutOfBoundsException(curr_x, curr_y, x_size_, y_size_);
        }

        if (real_grid_[curr_y][curr_x].IsShip()) {
            throw ShipsOverlapException(curr_x, curr_y);
        }

        ship_cells.emplace_back(curr_x, curr_y);
    }

    for (auto [curr_x, curr_y] : ship_cells) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int adj_x = curr_x + dx;
                int adj_y = curr_y + dy;

                if (IsValid(adj_x, adj_y, x_size_, y_size_) && real_grid_[adj_y][adj_x].IsShip()) {
                    throw ShipsTooCloseException(curr_x, curr_y, adj_x, adj_y);
                }
            }
        }
    }
}

bool PlayingField::SetRandomShips(const ShipManager& manager, size_t max_attempts) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < manager.ship_count(); i++) {
        int ship_size = manager.ship_size(i);
        bool placed = false;
        size_t attempts = 0;

        while (!placed && attempts < max_attempts) {
            attempts++;
            std::uniform_int_distribution<> distX(0, x_size_ - 1);
            std::uniform_int_distribution<> distY(0, y_size_ - 1);
            std::uniform_int_distribution<> distOrientation(0, 1);

            int x = distX(gen);
            int y = distY(gen);
            Orientation orient = (distOrientation(gen) == 0) ? Orientation::HORIZONTAL : Orientation::VERTICAL;

            try {
                PlaceShip(x, y, ship_size, orient);
                placed = true;
            } catch (const std::exception&) { }
        }

        if (!placed) {
            return false;
        }
    }

    return true;
}


void PlayingField::RotateOrientation() { 
    current_orientation_ = (current_orientation_ == Orientation::HORIZONTAL) 
                        ? Orientation::VERTICAL : Orientation::HORIZONTAL;
}


Orientation PlayingField::current_orientation() const { 
    return current_orientation_; 
}


bool PlayingField::PlaceRemovedShip(int x, int y, int size, Orientation orientation) {
    if (removed_ships_.empty()) return false;
        
    Ship ship_to_place = removed_ships_.back();
        
    MoveShip(x,y,size,orientation);
    ship_to_place.set_start_position(Position(x, y));
    ship_to_place.set_orientation(orientation);
    ship_to_place.set_ship_number(count_);
    PlaceShipOnGrid(ship_to_place);
    removed_ships_.pop_back();
        
    if (removed_ships_.empty()) {
        is_in_replacement_mode_ = false;
    }
        
    return true;
}


bool PlayingField::PlaceNewShip(int x, int y, int size, Orientation orientation){
    MoveShip(x,y,size,orientation);
    Ship ship(Position(x, y), orientation, size, count_);
    PlaceShipOnGrid(ship);

    return true;
}


void PlayingField::PlaceShipOnGrid(const Ship& ship_to_place){
    int x = ship_to_place.start_position().x; 
    int y = ship_to_place.start_position().y;
    int ship_number = ship_to_place.ship_number();

    for (int i = 0; i < ship_to_place.ship_size(); ++i) {
        int curr_x = (ship_to_place.orientation() == Orientation::HORIZONTAL) ? x + i : x;
        int curr_y = (ship_to_place.orientation() == Orientation::VERTICAL) ? y + i : y;
        real_grid_[curr_y][curr_x].set_ship(i, ship_number);
    }
    ships_.push_back(ship_to_place);
    ++count_;
}


bool PlayingField::PlaceShip(int x, int y, int size, Orientation orientation) {
    if (is_in_replacement_mode_ && !removed_ships_.empty()) {
        return PlaceRemovedShip(x, y, size, orientation);
    } else {
        return PlaceNewShip(x, y, size, orientation);
    }
}


int PlayingField::Damage(int x, int y, int damage) {
    if (!IsValid(x, y, x_size_, y_size_)) {
        throw ShipOutOfBoundsException(x, y, x_size_, y_size_);
    }

    if (!real_grid_[y][x].IsShip()) {
        if (!visible_grid_[y][x].IsUnknown()) {
            return -1;
        }
        visible_grid_[y][x].set_empty();
        return 0;
    }

    const int ship_index = real_grid_[y][x].ship_index();
    const int index = real_grid_[y][x].segment_index();

    if (ship_index < 0 || index < 0 || 
        static_cast<size_t>(ship_index) >= ships_.size()) {
        return -1;
    }

    SegmentState before = ships_[ship_index].segment_state(index);
    if (before == SegmentState::DESTROYED) {
        return -1; 
    }

    ships_[ship_index].DamageShip(Position(x, y), damage);

    visible_grid_[y][x].set_ship(index, ship_index);

    if (ships_[ship_index].IsDestroyed()) {
        int sz = ships_[ship_index].ship_size();
        for (int i = 0; i < sz; ++i) {
            Position p = ships_[ship_index].segment_position(i);
            if (IsValid(p.x, p.y, x_size_, y_size_)) {
                visible_grid_[p.y][p.x].set_ship(i, ship_index);
            }
        }
        auto mark_water = [&](int px, int py){
            if (IsValid(px, py, x_size_, y_size_) && visible_grid_[py][px].IsUnknown()) {
                visible_grid_[py][px].set_empty();
            }
        };
        for (int i = 0; i < sz; ++i) {
            Position p = ships_[ship_index].segment_position(i);
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    mark_water(p.x + dx, p.y + dy);
        }
        ships_[ship_index].MarkFullyDestroyed();
        return 2; 
    }

    return 1;
}


bool PlayingField::IsShipCell(int x, int y) const {
    return real_grid_[y][x].IsShip();
}


bool PlayingField::IsScanned(int x, int y) const {
    if (!IsValid(x, y, x_size_, y_size_)) return false;
    return scanned_overlay_[y][x];
}


bool PlayingField::IsAllShipsDestroyed() const {
    for (const auto& ship : ships_) {
        if (!ship.IsDestroyed()) {
            return false;
        }
    }
    return true;
}


bool PlayingField::HasShipsToReplace() const {
    return !removed_ships_.empty();
}


void PlayingField::ClearField() {
    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            real_grid_[y][x].set_empty();
        }
    }

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            visible_grid_[y][x].set_unknown(); 
        }
    }

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            scanned_overlay_[y][x] = false;
        }
    }
    
    ships_.clear();
    count_ = 0; 
}


void PlayingField::ClearShip(int x, int y) {
    int ship_index = real_grid_[y][x].ship_index();
    if (ship_index < 0) return;

    Ship ship_to_remove = ships_[ship_index]; 

    Orientation orientation = ship_to_remove.orientation();
    Position pos = ship_to_remove.start_position();
    
    int dx = (orientation == Orientation::HORIZONTAL) ? 1 : 0;
    int dy = (orientation == Orientation::VERTICAL) ? 1 : 0;

    int current_x = pos.x;
    int current_y = pos.y;
    int length = ship_to_remove.ship_size();
    
    for (int j = 0; j < length; j++) {
        real_grid_[current_y][current_x].reset();
        current_x += dx;
        current_y += dy;
    }

    removed_ships_.push_back(ship_to_remove);
    ships_.erase(ships_.begin() + ship_index);
    UpdateShipNumbersAfterRemoval(ship_index);

    is_in_replacement_mode_ = true;
    --count_;
}


void PlayingField::ReturnStartState(){
    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            visible_grid_[y][x].set_unknown(); 
        }
    }

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            scanned_overlay_[y][x] = false;
        }
    }
    
    for (size_t i = 0; i < ships_.size(); i++){
        for (int j = 0; j < ships_[i].ship_size(); j++){
            ships_[i].set_segment_state(j, SegmentState::INTACT);
        }
    } 
}


void PlayingField::UpdateShipNumbersAfterRemoval(int removed_index) {
    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            if (real_grid_[y][x].IsShip()) {
                int ship_index = real_grid_[y][x].ship_index();
                if (ship_index > removed_index) {
                    real_grid_[y][x].set_ship_index(ship_index - 1);
                }
            }
        }
    }
    
    for (size_t i = removed_index; i < ships_.size(); ++i) {
        ships_[i].set_ship_number(ships_[i].ship_number() - 1);
    }
}


void PlayingField::save(std::ostream& out) const {
    out << x_size_ << ' ' << y_size_ << '\n';
    out << count_ << ' ' << is_in_replacement_mode_ << '\n';

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            out << static_cast<int>(real_grid_[y][x].segment_state()) << ' ';
            out << real_grid_[y][x].ship_index() << ' ';
            out << real_grid_[y][x].segment_index() << ' ';
        }
        out << '\n';
    }

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            out << static_cast<int>(visible_grid_[y][x].segment_state()) << ' ';
            out << visible_grid_[y][x].ship_index() << ' ';
            out << visible_grid_[y][x].segment_index() << ' ';
        }
        out << '\n';
    }

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            out << (scanned_overlay_[y][x] ? '1' : '0') << ' ';
        }
        out << '\n';
    }

    out << ships_.size() << '\n';
    for (const auto& s : ships_) {
        out << s.start_position().x << ' ' << s.start_position().y << ' '
            << s.ship_size() << ' ' << static_cast<int>(s.orientation()) << ' '
            << s.ship_number() << ' ' << s.destroyed_segments() << ' ' << s.hit_count() << '\n';

        for (int j = 0; j < s.ship_size(); ++j) {
            out << static_cast<int>(s.segment_state(j)) << ' '; 
        }
        out << '\n';
    }

    out << removed_ships_.size() << '\n';
    for (const auto& s : removed_ships_) {
        out << s.start_position().x << ' ' << s.start_position().y << ' '
            << s.ship_size() << ' ' << static_cast<int>(s.orientation()) << ' '
            << s.ship_number() << '\n';
        for (int j = 0; j < s.ship_size(); ++j) {
            out << static_cast<int>(s.segment_state(j)) << ' ';  
        }
        out << '\n';
    }
}


void PlayingField::load(std::istream& in) {
    in >> x_size_ >> y_size_;
    in >> count_ >> is_in_replacement_mode_;

    real_grid_.assign(y_size_, std::vector<Cell>(x_size_, Cell(CellState::EMPTY)));
    visible_grid_.assign(y_size_, std::vector<Cell>(x_size_, Cell(CellState::UNKNOWN)));
    scanned_overlay_.assign(y_size_, std::vector<bool>(x_size_, false));

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            int cell_state_value, ship_index, segment_index;
            in >> cell_state_value >> ship_index >> segment_index;
            real_grid_[y][x].set_state(static_cast<CellState>(cell_state_value));
            real_grid_[y][x].set_ship_index(ship_index);
            real_grid_[y][x].set_segment_index(segment_index);
        }
    }

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            int cell_state_value, ship_index, segment_index;
            in >> cell_state_value >> ship_index >> segment_index;
            visible_grid_[y][x].set_state(static_cast<CellState>(cell_state_value));
            visible_grid_[y][x].set_ship_index(ship_index);
            visible_grid_[y][x].set_segment_index(segment_index);
        }
    }

    for (int y = 0; y < y_size_; ++y) {
        for (int x = 0; x < x_size_; ++x) {
            int scanned_value;
            in >> scanned_value;
            scanned_overlay_[y][x] = (scanned_value != 0);
        }
    }

    size_t ship_count;
    in >> ship_count;
    ships_.clear();
    ships_.reserve(ship_count);
    for (size_t i = 0; i < ship_count; ++i) {
        int x, y, size, orientation_value, number, destroyed_segments, hit_count;
        in >> x >> y >> size >> orientation_value >> number >> destroyed_segments >> hit_count;

        Ship s(Position(x, y), static_cast<Orientation>(orientation_value), size, number);

        
        for (int j = 0; j < size; ++j) {
            int segment_value;
            in >> segment_value;
            s.set_segment_state(j, static_cast<SegmentState>(segment_value));
        }

        s.set_destroyed_segments(destroyed_segments);
        s.set_hit_count(hit_count);

        ships_.push_back(std::move(s));
    }
    
    size_t removed_count;
    in >> removed_count;
    removed_ships_.clear();
    removed_ships_.reserve(removed_count);
    for (size_t i = 0; i < removed_count; ++i) {
        int x, y, size, orientation_value, number;
        in >> x >> y >> size >> orientation_value >> number;

        Ship s(Position(x, y), static_cast<Orientation>(orientation_value), size, number);
        for (int j = 0; j < size; ++j) {
            int segment_value;
            in >> segment_value;
            s.set_segment_state(j, static_cast<SegmentState>(segment_value));
        }
        removed_ships_.push_back(std::move(s));
    }
}


int PlayingField::count() const { 
    return count_; 
}


int PlayingField::x_size() const { 
    return x_size_; 
}


int PlayingField::y_size() const { 
    return y_size_; 
}


int PlayingField::removed_ship_size() const {
    return removed_ships_.back().ship_size();
}


const Cell& PlayingField::visible_cell(int x, int y) const {
    if (!IsValid(x, y, x_size_, y_size_)) {
        throw ShipOutOfBoundsException(x, y, x_size_, y_size_);
    }
    return visible_grid_[y][x];
}


const Ship& PlayingField::ship(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= ships_.size()) {
        throw std::out_of_range("Недопустимый индекс корабля");
    }
    return ships_[index];
}


void PlayingField::set_cell_visible(int x, int y) { 
    if (IsValid(x, y, x_size_, y_size_)) {
        scanned_overlay_[y][x] = true;
    }
}


bool PlayingField::ship_info_at(int x, int y,
                                 int& ship_index, int& segment_index, int& ship_size,
                                 SegmentState& segment_state, Orientation& orientation) const {
    for (size_t i = 0; i < ships_.size(); ++i) {
        int index = ships_[i].segment_index(Position(x, y));
        if (index >= 0) {
            ship_index = static_cast<int>(i);
            segment_index = index;
            ship_size = ships_[i].ship_size();
            segment_state = ships_[i].segment_state(index);
            orientation = ships_[i].orientation();
            return true;
        }
    }
    return false;
}
