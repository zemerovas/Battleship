#ifndef BATTLESHIP_ABILITIES_ABILITYEXCEPTION_H_
#define BATTLESHIP_ABILITIES_ABILITYEXCEPTION_H_

#include <stdexcept>
#include <string>

class AbilityException : public std::runtime_error {
public:
    explicit AbilityException(const std::string& message);
    virtual ~AbilityException() = default;
};

class EmptyQueueException : public AbilityException {
public:
    EmptyQueueException(const std::string& msg = "Нельзя применить способность - у Вас нет способностей");
};

class AbilityApplicationException : public AbilityException {
private:
    std::string ability_name_;

public:
    AbilityApplicationException(const std::string& ability_name, const std::string& reason);
};

#endif