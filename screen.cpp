#include "screen.hpp"

std::string color(TextColor value){
    return "\033[" + std::to_string(static_cast<int>(value)) + "m";
}

Screen::Screen(int width, int height){
    this->width = width;
    this->height = height;
    this->pixels = new PixelType[width*height];
    for(int i =0; i<width*height;i++){
        this->pixels[i] = Dead;
    }    
}

Screen::~Screen(){
    delete[] this->pixels;
}

void clearTerminal(){
    std::cout << "\033[2J\033[1;1H"<<std::endl;
}

void Screen::print(){
    for (int y = 0; y < this->height; y++){
        for (int x = 0; x < this->width; x++){
            PixelType pixel = this->get(x,y);
            
            if (pixel == Alive){
                std::cout << color(B_Yellow) << color(BrightYellow) << "1 ";
            }else if(pixel == Dead){
                std::cout << color(B_Black)  << color(BrightBlack) << "0 ";
            } 
        }
        std::cout << color(Reset) << "\n";
    }
    std::cout << color(Reset) << std::endl;
}

void Screen::boundCheck(int x, int y){
    if (x < 0 || x>=this->width){
        std::cerr << "X was out of range [0," << this->width << ") ->" << x << std::endl;
        exit(1);
    }
    if (y < 0 || y>=this->height){
        std::cerr << "Y was out of range [0," << this->height << ") ->" << y << std::endl;
        exit(1);
    }
}

void Screen::set(int x, int y, PixelType value){
    this->boundCheck(x,y);
    this->pixels[x + y*this->width] = value;
}

PixelType Screen::get(int x, int y){
    this->boundCheck(x,y);
    return this->pixels[x + y*this->width];
}

PixelType Screen::nextState(int centerX, int centerY){
    
    int aliveNeighbourCount = 0;
    PixelType centerState = Alive;

    for (int offsetY = 1; offsetY > -2 ; offsetY--){
        for (int offsetX = -1; offsetX < 2; offsetX++){
            if (offsetX == 0 && offsetY == 0){
                // * don't calculate the center state
                centerState = this->get(centerX,centerY);
                continue;
            }
            
            int checkX = centerX + offsetX;
            int checkY = centerY + offsetY;
            
            if(this->get(checkX,checkY) == Alive){
                aliveNeighbourCount++;
            }
        }
    }

    if (centerState == Alive){
        // * Center is alive
        if(aliveNeighbourCount <= 1){
            return Dead;
        }

        if(aliveNeighbourCount == 2 || aliveNeighbourCount == 3){
            return Alive;
        }

        if(aliveNeighbourCount >= 4){
            return Dead;
        }

    }else{
        // * Center is dead
        if (aliveNeighbourCount == 3){
            return Alive;
        }
    }
    return centerState;
}