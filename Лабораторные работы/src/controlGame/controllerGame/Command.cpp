#include "Command.h"

Command::Command(CommandType type, const std::string& description) : type_(type), description_(description) {}

CommandType Command::type() const { 
    return type_; 
}
    
std::string Command::description() const { 
    return description_; 
}