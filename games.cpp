#include "games.hpp"

Abstract_GameOfLife::Abstract_GameOfLife(int width, int height):buffers{Screen(width+2,height+2),Screen(width+2,height+2)}{
    this->width = width;
    this->height = height;
    this->workBuffer = 0;

    

    this->buffers[1].set(width/2    +1,height/2     +1,Alive);
    this->buffers[1].set(width/2    +1,height/2+1   +1,Alive);
    this->buffers[1].set(width/2    +1,height/2-1   +1,Alive);
    this->buffers[1].set(width/2+1  +1,height/2     +1,Alive);
    this->buffers[1].set(width/2-1  +1,height/2     +1,Alive);

}

int Abstract_GameOfLife::otherBuffer(int bufferNum){
    return (bufferNum+1)%2;
}

void Abstract_GameOfLife::spawnPlus(int x, int y){  
    this->buffers[this->workBuffer].set(x  ,y  ,Alive);
    this->buffers[this->workBuffer].set(x  ,y+1,Alive);
    this->buffers[this->workBuffer].set(x  ,y-1,Alive);
    this->buffers[this->workBuffer].set(x+1,y  ,Alive);
    this->buffers[this->workBuffer].set(x-1,y  ,Alive);
}

void Abstract_GameOfLife::randomizeBuffer(PixelType state, float percent){
    int cellCount = this->width * this->height * percent;
    for(int i = 0; i<cellCount; i++){
        int randomX = (rand() % this->width) + 1;
        int randomY = (rand() % this->height) + 1;
        this->buffers[this->workBuffer].set(randomX,randomY,state);
    }
}

void Abstract_GameOfLife::flipBuffer(){
    this->workBuffer = this->otherBuffer(this->workBuffer);
}

void Abstract_GameOfLife::updateBufferAt(int xStart, int xStep, int yStart, int yStep){
    int seenBuffer = this->otherBuffer(this->workBuffer);
    for(int y = yStart; y<=this->height; y+=yStep){
        for (int x = xStart; x<=this->width; x+=xStep){
            PixelType state = this->buffers[seenBuffer].nextState(x,y);
            this->buffers[this->workBuffer].set(x,y,state);
        }
    }
}

void Abstract_GameOfLife::show(){
    this->buffers[otherBuffer(workBuffer)].print();
}

SingleThreaded_GameOfLife::SingleThreaded_GameOfLife(int width, int height):Abstract_GameOfLife(width,height){

}

void SingleThreaded_GameOfLife::beginFrame(){

}

void SingleThreaded_GameOfLife::renderFrame(){
    this->updateBufferAt();
}

void SingleThreaded_GameOfLife::endFrame(){
    this->flipBuffer();
}

void SingleThreaded_GameOfLife::start(){

}

void SingleThreaded_GameOfLife::stop(){

}

Abstract_MultiThreaded_GameOfLife::Abstract_MultiThreaded_GameOfLife(int width, int height, int workerThreads)
:Abstract_GameOfLife(width,height){
    this->startBarrier = nullptr;
    this->workerThreads = workerThreads;
    this->allThreads = this->workerThreads+1;
    this->workEnded = false;
    this->workers = std::vector<std::thread>();
}

Abstract_MultiThreaded_GameOfLife::~Abstract_MultiThreaded_GameOfLife(){
    delete this->startBarrier;
    delete this->endBarrier;
}

void Abstract_MultiThreaded_GameOfLife::workerFunc(int threadID){
    while(true){
        this->startBarrier->await(threadID);
        if(this->workEnded){
            break;
        }

        this->updateBufferAt(1,1,threadID,this->workerThreads);
        this->endBarrier->await(threadID);
    }
    return;
}

void Abstract_MultiThreaded_GameOfLife::renderFrame(){
    this->startBarrier->await(0);
    this->endBarrier->await(0);
}

void Abstract_MultiThreaded_GameOfLife::endFrame(){
    this->flipBuffer();
}

void Abstract_MultiThreaded_GameOfLife::stop(){
    this->workEnded = true;
    this->startBarrier->await(0);
    
    int i = 1;
    for(std::thread& worker : this->workers){
        worker.join();
        myPrint("Work ended for: " + std::to_string(i));
        i++;
    }
    this->workers.clear();
}

void Abstract_MultiThreaded_GameOfLife::start(){
    for(int i = 1; i<=this->workerThreads; i++){
        this->workers.emplace_back(
            std::thread(&Abstract_MultiThreaded_GameOfLife::workerFunc,this,i));
    }
}

void Abstract_MultiThreaded_GameOfLife::beginFrame(){
    
}

Sense_MultiThreaded_GameOfLife::Sense_MultiThreaded_GameOfLife(int width, int height, int workerThreads)
:Abstract_MultiThreaded_GameOfLife(width,height,workerThreads){
    this->startBarrier = new SenseReversingBarrier(this->allThreads);
    this->endBarrier = new SenseReversingBarrier(this->allThreads);
}

Combining_MultiThreaded_GameOfLife::Combining_MultiThreaded_GameOfLife(int width, int height, int workerThreads)
:Abstract_MultiThreaded_GameOfLife(width,height,workerThreads){
    this->startBarrier = new CombiningTreeBarrier(this->allThreads);
    this->endBarrier = new CombiningTreeBarrier(this->allThreads);
}

Tournament_MultiThreaded_GameOfLife::Tournament_MultiThreaded_GameOfLife(int width, int height, int workerThreads)
:Abstract_MultiThreaded_GameOfLife(width,height,workerThreads){
    this->startBarrier = new TournamentTreeBarrier(this->allThreads);
    this->endBarrier = new TournamentTreeBarrier(this->allThreads);
}