#ifndef __BRANCH_PREDICTOR_HPP__
#define __BRANCH_PREDICTOR_HPP__

#include <vector>
#include <bitset>
#include <cassert>
#include <iostream>
#include<iostream>
#include<string>
#include<fstream>
#include<iterator>
#include<map>
using namespace std;


struct BranchPredictor {
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) =0;
};

struct SaturatingBranchPredictor : public BranchPredictor {
    vector<bitset<2>> table;
    SaturatingBranchPredictor(int value) : table(1 << 14, value) {}

    bool predict(uint32_t pc) {
       int index = (pc & 16383); //the 14 lsbs of the pc
         if(table[index][1] == 1)
              return true;
         else
              return false;
    }

    void update(uint32_t pc, bool taken) {
        int index = (pc & 16383);
        if(taken)
        {
            if(table[index].count() == 2) return; //if both are already 1
            if(table[index][0] == 1) //01 goes to 10
            {
                table[index][0].flip(); table[index][1].flip();
            }
            else //00 goes to 01, 10 goes to 11
            {
                table[index][0].flip(); //updating this bit to 1
            }
        }
        else
        {
            if(table[index].count() == 0) return; //if both are already 0
            if(table[index][0] == 1) //01 or 11 goes to 00 or 10
            {
                table[index][0].flip(); //updating this bit to 0
            }
            else //10 goes to 01 (00 case is not there as its already taken care of)
            {
                table[index][0].flip(); table[index][1].flip();
            }
        }
        
    }
};

struct BHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    BHRBranchPredictor(int value) : bhrTable(1 << 2, value), bhr(value) {}

    bool predict(uint32_t pc) {
        // your code here
        return false;
    }

    void update(uint32_t pc, bool taken) {
        // your code here
    }
};

struct SaturatingBHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    std::vector<std::bitset<2>> table;
    std::vector<std::bitset<2>> combination;
    SaturatingBHRBranchPredictor(int value, int size) : bhrTable(1 << 2, value), bhr(value), table(1 << 14, value), combination(size, value) {
        assert(size <= (1 << 16));
    }

    bool predict(uint32_t pc) {
        // your code here
        return false;
    }

    void update(uint32_t pc, bool taken) {
        // your code here
    }
};



#endif