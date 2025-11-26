#ifndef BATTLESHIP_ADDITIONAL_OTHER_H_
#define BATTLESHIP_ADDITIONAL_OTHER_H_

#include <string>
#include <iostream>
#include <algorithm> 
#include <cctype>   

bool IsValid(int x, int y, int x_size_, int y_size_);
void Trim(std::string& str);
bool IsComment(const std::string& line);
std::string ToLower(std::string s); 
std::string ColumnLabel(int index);
int DigitsCount(int value);
int MaxLabelWidthForX(int size);
#endif