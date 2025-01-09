#include "barriers.hpp"

void myPrint(std::string str){
    std::cout << (str + "\n");
}

Abstract_Barrier::Abstract_Barrier(int maxThreads){
    this->maxThreads = maxThreads;
    //myPrint("Barrier created " + std::to_string(maxThreads));
}

SenseReversingBarrier::SenseReversingBarrier(int maxThreads) : Abstract_Barrier(maxThreads){
    this->remainingThreads = this->maxThreads;
    this->globalSense = true;
    this->localSenses = new bool[this->maxThreads];
    for(int i = 0; i<this->maxThreads; i++){
        this->localSenses[i] = true;
    }
}

SenseReversingBarrier::~SenseReversingBarrier(){
    delete[] this->localSenses;
}

void SenseReversingBarrier::await(int threadID){
    bool mySense = this->localSenses[threadID];
    if(this->remainingThreads.fetch_sub(1,std::memory_order_release) == 1){
        //myPrint(std::to_string(threadID) + ": Was Last");
        this->remainingThreads.store(this->maxThreads,std::memory_order_release);
        this->globalSense = !mySense;
    }else{
        //myPrint(std::to_string(threadID) + ": Entered Waiting");
        while(mySense == this->globalSense){}
    }
    this->localSenses[threadID] = !mySense;
}

CombiningTreeBarrierNode::CombiningTreeBarrierNode(int maxThreads)
:Abstract_Barrier(maxThreads){
    
    this->idToParent    = 0;
    this->parent        = nullptr;
    this->childNum      = 1;

    this->remainingThreads = this->maxThreads;
    this->globalSense = true;
    this->localSenses = new bool[this->maxThreads];
    for(int i = 0; i<this->maxThreads; i++){
        this->localSenses[i] = true;
    }
}

CombiningTreeBarrierNode::~CombiningTreeBarrierNode(){
    delete[] this->localSenses;
}

void CombiningTreeBarrierNode::await(int threadID){
    bool mySense = this->localSenses[threadID];
    if(this->remainingThreads.fetch_sub(1,std::memory_order_release) == 1){
        
        if(this->parent!=nullptr){
            parent->await(this->idToParent);
        }
    
        this->remainingThreads.store(this->maxThreads,std::memory_order_release);
        this->globalSense = !mySense;
    }else{
        while(mySense == this->globalSense){}
    }
    this->localSenses[threadID] = !mySense;
}

CombiningTreeBarrier::CombiningTreeBarrier(int maxThreads):Abstract_Barrier(maxThreads){
    
    this->nodes = std::vector<CombiningTreeBarrierNode*>();
    this->bottomNodeNum = this->maxThreads / 2;
    this->bottomNodes = new CombiningTreeBarrierNode*[this->bottomNodeNum];

    // * Fst node conrains trailing node
    CombiningTreeBarrierNode* node = nullptr;
    if((this->maxThreads%2)==0){
        node = new CombiningTreeBarrierNode(2);
    }else{
        node = new CombiningTreeBarrierNode(3);
    }

    this->nodes.push_back(node);
    this->bottomNodes[0] = node;

    for(int i = 1; i<this->bottomNodeNum; i++){
        CombiningTreeBarrierNode* node = new CombiningTreeBarrierNode(2);
        this->nodes.push_back(node);
        this->bottomNodes[i] = node;
    }

    std::vector<int> indexes;
    
    for(int i = 0; i<static_cast<int>(this->nodes.size());i++){
        indexes.push_back(i);
    }

    while(true){
        int fstMinIndex = -1;
        
        int minVal = 99999999;
        for(int i = 0; i<static_cast<int>(indexes.size());i++){
            if(this->nodes[indexes[i]]->childNum <= minVal){
                fstMinIndex = i;
                minVal = this->nodes[indexes[i]]->childNum;
            }
        }
        minVal = 99999999;
        int sndMinIndex = -1;
        for(int i = 0; i<static_cast<int>(indexes.size());i++){
            if(i!=fstMinIndex && this->nodes[indexes[i]]->childNum <= minVal){
                sndMinIndex = i;
                minVal = this->nodes[indexes[i]]->childNum;
            }
        }

       

        if(fstMinIndex < 0 || sndMinIndex < 0 ){
            // * no more nodes left to join together.
            break;
        }

        CombiningTreeBarrierNode* fstNode = nodes[indexes[fstMinIndex]];
        CombiningTreeBarrierNode* sndNode = nodes[indexes[sndMinIndex]];

        CombiningTreeBarrierNode* newParent = new CombiningTreeBarrierNode(2);
        this->nodes.push_back(newParent);
        indexes.push_back(static_cast<int>(this->nodes.size())-1);

        fstNode->parent = newParent;
        fstNode->idToParent = 0;
        sndNode->parent = newParent;
        sndNode->idToParent = 1;

        newParent->childNum = fstNode->childNum + sndNode->childNum;

        if(fstMinIndex<sndMinIndex){
            indexes.erase(indexes.begin() + sndMinIndex);
            indexes.erase(indexes.begin() + fstMinIndex);
        }else{
            indexes.erase(indexes.begin() + fstMinIndex);
            indexes.erase(indexes.begin() + sndMinIndex);
        }
    }

    int nullCount = 0;
    int nCount = 0;
    for(CombiningTreeBarrierNode* n : this->nodes){
        if(n->parent==nullptr){
            nullCount++;
        }
        nCount++;
    }

    myPrint("Nullcount was:" + std::to_string(nullCount) + "/" + std::to_string(nCount));
}

CombiningTreeBarrier::~CombiningTreeBarrier(){
    for(int i = 0; i<static_cast<int>(this->nodes.size());i++){
        CombiningTreeBarrierNode* node = this->nodes[i];
        delete node;
    }

    delete[] this->bottomNodes;
}

void CombiningTreeBarrier::await(int threadID){
    int nodeIndex = threadID/2;
    int nodeID    = threadID%2;
    
    if(((this->maxThreads%2)==1) && ((this->maxThreads-1)==threadID)){
        // * Trailing node
        nodeIndex = 0;
        nodeID = 2;
    }

    this->bottomNodes[nodeIndex]->await(nodeID);
}

TournamentTreeNode::TournamentTreeNode(){
    this->childNum = 1;
    this->flag = true;
    this->partner = nullptr;
    this->parent = nullptr;
    this->top = false;
}

void TournamentTreeNode::await(bool localSense){
    if(this->top){
        // * for knowing what to start with next time
        this->flag = localSense;
        return;
    }else if (parent != nullptr){
        while(this->flag!=localSense){}
        parent->await(localSense);
        partner->flag = localSense;
    }else{
        partner->flag = localSense;
        while(this->flag!=localSense){}
    }
}

TournamentTreeBarrier::TournamentTreeBarrier(int maxThreads):Abstract_Barrier(maxThreads){
    
    this->nodes = std::vector<TournamentTreeNode*>();
    this->bottomNodes = new TournamentTreeNode*[this->maxThreads];

    for(int i = 0; i<this->maxThreads; i++){
        TournamentTreeNode* node = new TournamentTreeNode();
        this->nodes.push_back(node);
        this->bottomNodes[i] = node;
    }

    std::vector<int> indexes;
    for(int i = 0; i<static_cast<int>(this->nodes.size());i++){
        indexes.push_back(i);
    }

    this->top = nullptr;

    while(true){
        int fstMinIndex = -1;
        
        int minVal = 99999999;
        for(int i = 0; i<static_cast<int>(indexes.size());i++){
            if(this->nodes[indexes[i]]->childNum <= minVal){
                fstMinIndex = i;
                minVal = this->nodes[indexes[i]]->childNum;
            }
        }
        minVal = 99999999;
        int sndMinIndex = -1;
        for(int i = 0; i<static_cast<int>(indexes.size());i++){
            if(i!=fstMinIndex && this->nodes[indexes[i]]->childNum <= minVal){
                sndMinIndex = i;
                minVal = this->nodes[indexes[i]]->childNum;
            }
        }

        if(fstMinIndex < 0 || sndMinIndex < 0 ){
            // * no more nodes left to join together.
            break;
        }

        TournamentTreeNode* fstNode = nodes[indexes[fstMinIndex]];
        TournamentTreeNode* sndNode = nodes[indexes[sndMinIndex]];

        TournamentTreeNode* newParent = new TournamentTreeNode();
        this->nodes.push_back(newParent);
        indexes.push_back(static_cast<int>(this->nodes.size())-1);

        fstNode->parent = newParent;
        fstNode->partner = sndNode;
        sndNode->parent = nullptr;
        sndNode->partner = fstNode;

        newParent->childNum = fstNode->childNum + sndNode->childNum;
        
        // * last new parent is the top
        this->top = newParent;

        if(fstMinIndex<sndMinIndex){
            indexes.erase(indexes.begin() + sndMinIndex);
            indexes.erase(indexes.begin() + fstMinIndex);
        }else{
            indexes.erase(indexes.begin() + fstMinIndex);
            indexes.erase(indexes.begin() + sndMinIndex);
        }
    }

    this->top->top = true;

    int parentCount = 0;
    int nCount = 0;
    int topCount = 0;
    for(TournamentTreeNode* n : this->nodes){
        if(n->top==true){
            topCount++;
        }
        if(n->parent!=nullptr){
            parentCount++;
            myPrint("Cn: " + std::to_string(n->childNum) + "\t up Cn: " + std::to_string(n->parent->childNum) + 
            "\t partner Cn: " + std::to_string(n->partner->childNum));
        }
        nCount++;
    }

    myPrint("ParentCount was:" + std::to_string(parentCount) + "/" + std::to_string(nCount));
    myPrint("TopCount was:" + std::to_string(topCount) + "/" + std::to_string(nCount));
    
}

TournamentTreeBarrier::~TournamentTreeBarrier(){
    for(int i = 0; i<static_cast<int>(this->nodes.size());i++){
        TournamentTreeNode* node = this->nodes[i];
        delete node;
    }

    delete[] this->bottomNodes;
}

void TournamentTreeBarrier::await(int threadID){
    bool sense = !this->top->flag;
    this->bottomNodes[threadID]->await(sense);
}