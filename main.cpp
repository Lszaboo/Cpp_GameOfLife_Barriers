#include <iostream>
#include <ranges>
#include <chrono>
#include "screen.hpp"
#include "games.hpp"

#define ARG_COUNT 5

int to_int(std::string str){
    int num = 0;
    try {
        num = std::stoi(str);
    } catch (const std::invalid_argument& e) {
        std::cerr << "\"" << str << "\" is not an integer." << std::endl;
        exit(1);
    } catch (const std::out_of_range& e) {
        std::cerr << "\"" << str << "\" is out of range for an integer." << std::endl;
        exit(1);
    }
    return num;
}

int main(int argc, char** argv) {
    
    if(argc-1<ARG_COUNT){
        std::cerr << "Need at least " << std::to_string(ARG_COUNT) << " arguments (only " << std::to_string(argc-1) << "supplied)" << std::endl;
        exit(1);
    }

    std::srand(999);
    
    
    
    std::string barrierType = std::string(argv[1]);
    bool showFrames     = (to_int(argv[2])==1) ? true : false;
    int frameCount      = to_int(argv[3]);   
    int screenWidth     = to_int(argv[4]);
    int screenHeight    = to_int(argv[5]);
    
    Abstract_GameOfLife *Game = nullptr;

    if (barrierType=="Single"){
        Game = new SingleThreaded_GameOfLife(screenWidth,screenHeight);
    }else{
        size_t split = barrierType.find("-");
        if(split==std::string::npos){
            std::cerr << "Unrecognized barrier type." << std::endl;
            exit(1);
        }

        std::string barrierTypeName = barrierType.substr(0,split);
        int threads = to_int(barrierType.substr(split + 1,barrierType.length() - split + 1));

        if(threads<=1){
            std::cerr << "More than 1 thread is needed for multithreaded type." << std::endl;
            exit(1);
        }

        if(barrierTypeName=="Sense"){
            Game = new Sense_MultiThreaded_GameOfLife(screenWidth,screenHeight,threads);
        }else if(barrierTypeName=="Combine"){
            Game = new Combining_MultiThreaded_GameOfLife(screenWidth,screenHeight,threads);
        }else if(barrierTypeName=="Tour"){
            Game = new Tournament_MultiThreaded_GameOfLife(screenWidth,screenHeight,threads);
        }else{
            std::cerr << "Unrecognized multithreaded barrier type." << std::endl;
            exit(1);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    Game->start();

    for(int frame = 1; frame<=frameCount; frame++){
        Game->beginFrame();
        
        if(showFrames){
            Game->show();
        }
        
        Game->renderFrame();

        if(frame % 10 == 0){
            int randomX = (rand() % (screenWidth - 2)) + 2;
            int randomY = (rand() % (screenHeight- 2)) + 2;
            Game->spawnPlus(randomX,randomY);
        }

        Game->endFrame();
        myPrint("Frame " + std::to_string(frame) + " is finished!");
    }
    Game->show();
    Game->stop();
    delete Game;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    myPrint("Game completed in (type,width,height,frames,seconds):");
    myPrint("");
    myPrint(barrierType             + "\t" + 
    std::to_string(screenWidth)     + "\t" +
    std::to_string(screenHeight)    + "\t" +
    std::to_string(frameCount)      + "\t" + 
    std::to_string(duration.count()));

    return 0;
}