#ifndef BATTLESHIP_ADDITIONAL_SHIPCOORDINATEEXCEPTIONS_H_
#define BATTLESHIP_ADDITIONAL_SHIPCOORDINATEEXCEPTIONS_H_

#include <stdexcept>
#include <string>

class ShipPlacementException : public std::runtime_error {
public:
    ShipPlacementException(const std::string& msg) : std::runtime_error(std::string("Ошибка кораблей: ") + msg) {}
};

class ShipOutOfBoundsException : public ShipPlacementException {
public:
    ShipOutOfBoundsException(int x, int y, int x_size, int y_size) :
        ShipPlacementException("Координаты вне поля: (" + std::to_string(x) + "," + std::to_string(y) +
                          "), допустимый диапазон: [0-" + std::to_string(x_size-1) + ",0-" +
                          std::to_string(y_size-1) + "]") {}
};

class ShipsOverlapException : public ShipPlacementException {
public:
    ShipsOverlapException(int x, int y) :
        ShipPlacementException("Корабль пересекается с другим кораблем в точке (" + 
                          std::to_string(x) + "," + std::to_string(y) + ")") {}
};

class ShipsTooCloseException : public ShipPlacementException {
public:
    ShipsTooCloseException(int x1, int y1, int x2, int y2) :
        ShipPlacementException("Корабли находятся слишком близко: клетка (" + 
                          std::to_string(x1) + "," + std::to_string(y1) + ") соседствует с (" + 
                          std::to_string(x2) + "," + std::to_string(y2) + ")") {}
};

class ImpossibleFleetException : public ShipPlacementException {
public:
    ImpossibleFleetException() :
        ShipPlacementException("Невозможно разместить данный набор кораблей на текущем поле. Расстановка начинается заново.") {}
};

#endif