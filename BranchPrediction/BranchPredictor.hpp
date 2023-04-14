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

int getLeast14(std::string hex)
{
    //will just check the last 4 digits of it, and then remove the 2 msbs
    int num = 0; int mult = 1;
    for (int i = hex.size()-1; i >= 4; i--)
    {
        if(hex[i] >= 'a')
            num += (hex[i]-'a'+10)*mult;
        else
            num += (hex[i] - '0')*mult;
        mult *= 16;
    }
    num = (num & 16383); //done to get only the first 14 bits. 16383 is 2^14 - 1
    return num;
}

struct BranchPredictor {
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) =0;
};

struct SaturatingBranchPredictor : public BranchPredictor {
    vector<bitset<2>> table;
    SaturatingBranchPredictor(int value) : table(1 << 14, value) {}

    bool predict(uint32_t pc) {
        int index = pc;
        //  bitset<2> a = bitset<2>("11");
        // bitset<2> b = bitset<2>("10");
       if(table[index] == bitset<2>("11") ){
        return true;
       }
       else if(table[index] == bitset<2>("10")){
        return true;
       }
       else{
        return false;
       }
    }

    void update(uint32_t pc, bool taken) {
        int index = pc;
        bitset<2> a = bitset<2>("11");
        bitset<2> b = bitset<2>("10");
        bitset<2> c = bitset<2>("01");
        bitset<2> d = bitset<2>("00");
        if(taken == true){

          if(table[index].operator==(a)){
           table[index].operator=(a);
          }
          if(table[index].operator==(b)){
             table[index].operator=(a);
          }
          else if(table[index].operator==(c)){
              table[index].operator=(b);
          }
          else if(table[index].operator==(d)){
            table[index].operator=(b);
          }
        }
        else{
             if(table[index].operator==(d)){
            table[index].operator=(d);
          }
          else if((table[index].operator==(c))){
            table[index].operator=(d);
          }
          else if(table[index].operator==(b)){
              table[index].operator=(c);
          }
          else if(table[index].operator==(a)){
            table[index].operator=(b);
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