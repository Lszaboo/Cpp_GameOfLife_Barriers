#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <iostream>

void myPrint(std::string str);

class Abstract_Barrier{
    private:
        /* data */
    protected:
        int maxThreads;
    public:
        Abstract_Barrier(int maxThreads);
        virtual ~Abstract_Barrier() = default;
        virtual void await(int threadID)=0;
};

class SenseReversingBarrier : public Abstract_Barrier{
    private:
        /* data */
    protected:
        std::atomic<int> remainingThreads;
        volatile bool globalSense;
        bool *localSenses;
    public:
        void await(int threadID) override;

        SenseReversingBarrier(int maxThreads);
        ~SenseReversingBarrier();
};

class CombiningTreeBarrierNode : public Abstract_Barrier{
    private:

    protected:

        std::atomic<int> remainingThreads;
        volatile bool globalSense;
        bool *localSenses;
        
    public:
        int childNum;
        CombiningTreeBarrierNode *parent;
        int idToParent;

        void await(int threadID) override;
        CombiningTreeBarrierNode(int maxThreads);
        ~CombiningTreeBarrierNode();
};

class CombiningTreeBarrier : public Abstract_Barrier{
    private:

    protected:
        
        int bottomNodeNum;
        int nodeNum;
        std::vector<CombiningTreeBarrierNode*> nodes;
        CombiningTreeBarrierNode **bottomNodes;

    
    public:
        void await(int threadID) override;
        CombiningTreeBarrier(int maxThreads);
        ~CombiningTreeBarrier();
};

class TournamentTreeNode{
    private:

    protected:
        int maxThreads;
    public:
        
        int childNum;
        volatile bool flag;
        TournamentTreeNode* partner;
        TournamentTreeNode* parent;
        bool top;


        void await(bool localSense);
        TournamentTreeNode();
        ~TournamentTreeNode() = default;
};

class TournamentTreeBarrier : public Abstract_Barrier{
    private:

    protected:
        
        int nodeNum;
        std::vector<TournamentTreeNode*> nodes;
        TournamentTreeNode **bottomNodes;
        TournamentTreeNode *top;
    
    public:
        void await(int threadID) override;
        TournamentTreeBarrier(int maxThreads);
        ~TournamentTreeBarrier();
};