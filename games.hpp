#pragma once
#include <array>
#include <thread>
#include "screen.hpp"
#include "barriers.hpp"

class Abstract_GameOfLife{
    private:
        /* data */
    protected:
        int width;
        int height;
        int workBuffer;
        std::array<Screen,2> buffers;

        void flipBuffer();
        void updateBufferAt(int xStart=1, int xStep=1, int yStart=1, int yStep=1);
        int otherBuffer(int bufferNum);
    public:
        Abstract_GameOfLife(int width, int height);
        virtual ~Abstract_GameOfLife() = default;

        void randomizeBuffer(PixelType state, float percent);
        void show();
        void spawnPlus(int x, int y);


        virtual void start()        = 0;
        virtual void beginFrame()   = 0;
        virtual void renderFrame()  = 0; 
        virtual void endFrame()     = 0;
        virtual void stop()         = 0;
};

class SingleThreaded_GameOfLife : public Abstract_GameOfLife{
    private:

    protected:
        
    public:
        SingleThreaded_GameOfLife(int width, int height);
        
        void start()        override;
        void beginFrame()   override;
        void renderFrame()  override;
        void endFrame()     override;
        void stop()         override;
};

class Abstract_MultiThreaded_GameOfLife : public Abstract_GameOfLife{
    private:

    protected:
        Abstract_Barrier* startBarrier;
        Abstract_Barrier* endBarrier;
        int workerThreads;
        int allThreads;
        volatile bool workEnded;
        std::vector<std::thread> workers;
    public:
        Abstract_MultiThreaded_GameOfLife(int width, int height, int workerThreads);
        ~Abstract_MultiThreaded_GameOfLife();
        
        void workerFunc(int threadID);
        
        void start()        override;
        void beginFrame()   override;
        void renderFrame()  override;
        void endFrame()     override;
        void stop()         override; 
};

class Sense_MultiThreaded_GameOfLife : public Abstract_MultiThreaded_GameOfLife{
    private:

    protected:
    
    public:
        Sense_MultiThreaded_GameOfLife(int width, int height, int workerThreads);
};

class Combining_MultiThreaded_GameOfLife : public Abstract_MultiThreaded_GameOfLife{
    private:

    protected:
    
    public:
        Combining_MultiThreaded_GameOfLife(int width, int height, int workerThreads);
};

class Tournament_MultiThreaded_GameOfLife : public Abstract_MultiThreaded_GameOfLife{
    private:

    protected:
    
    public:
        Tournament_MultiThreaded_GameOfLife(int width, int height, int workerThreads);
};
