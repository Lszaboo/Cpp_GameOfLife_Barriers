#pragma once
#include <vector>
#include <map>
#include <string>
#include <iostream>

enum TextColor {
    Reset           = 0,
    Black           = 30,
    Red             = 31,
    Green           = 32,
    Yellow          = 33,
    Blue            = 34,
    Magenta         = 35,
    Cyan            = 36,
    White           = 37,
    BrightBlack     = 90,
    BrightRed       = 91,
    BrightGreen     = 92,
    BrightYellow    = 93,
    BrightBlue      = 94,
    BrightMagenta   = 95,
    BrightCyan      = 96,
    BrightWhite     = 97,
    B_Black = 40,
    B_Red = 41,
    B_Green = 42,
    B_Yellow = 43,
    B_Blue = 44,
    B_Magenta = 45,
    B_Cyan = 46,
    B_White = 47,
    B_BrightBlack = 100,
    B_BrightRed = 101,
    B_BrightGreen = 102,
    B_BrightYellow = 103,
    B_BrightBlue = 104,
    B_BrightMagenta = 105,
    B_BrightCyan = 106,
    B_BrightWhite = 107
};

std::string color(TextColor value);

void clearTerminal();

enum PixelType{
    Dead,
    Alive
};

class Screen
{
private:
    /* data */
protected:    
    int width;
    int height;
    PixelType* pixels;
    void boundCheck(int x, int y);
public:
    Screen(int width, int height);
    ~Screen();
    void print();
    void set(int x, int y, PixelType value);
    PixelType get(int x, int y);
    PixelType nextState(int centerX, int centerY);
};