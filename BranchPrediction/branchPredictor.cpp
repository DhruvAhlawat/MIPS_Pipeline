#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<iterator>
#include<map>
using namespace std;

int getLeast14(string hex)
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

struct cntState //simple state for counters
{
    int cnt = 0;
    cntState(int starter)
    {
        cnt = starter;
    }

    bool getState()
    {
        return (cnt > 1); //returns 0 for 0 and 1, and 1 for 2 and 3.
    }

    void Update(int val)
    {
        if(val == 0)
        {
            if(cnt == 0) return;
            else cnt--;
        }
        else
        {
            if(cnt == 3) return;
            else cnt++; 
        }
    }

};

int main()
{
    string cntOut[4] = {"cnt00.txt", "cnt01.txt", "cnt10.txt","cnt11.txt"};
    for (int i = 0; i < 4; i++)
    {
        int initialValueOfCounter = i;
        vector<cntState> counters(16384, cntState(initialValueOfCounter)); //makes an array of 2^14 counters with the given starting state
        
        ifstream branchtrace; branchtrace.open("branchtrace.txt");
        ofstream outfile(cntOut[i]);
        int correctPredictions = 0;
        int total = 0;
        outfile << "array counter VARIANT where initially all counters are in state " << i << endl;
        outfile << endl;
        if(!branchtrace.is_open())
        {
            cout << "file cannot be opened" << endl; return -1;
        }    
        while(branchtrace.good())
        {
            string a,b;
            branchtrace >> a;
            if(branchtrace.good()) //because sometimes the >> operator duplicates the last one
            {
                total++;
                int index = getLeast14(a);
                
                int prediction = counters[index].getState(); //getState returns the prediction of the counter object
                outfile << "Prediction for " << a << " is =>" << prediction << " at state " << counters[index].cnt << " which is ";
                branchtrace >> b;
                if((b[0] - '0') == prediction)
                {
                    correctPredictions++;
                    outfile << "CORRECT" << endl; //prints correct if the prediction matches
                }
                else
                {
                    outfile << "WRONG" << endl; //else it prints wrong if the prediction was incorrect
                }
                counters[index].Update(b[0] - '0');
            }
        }

        outfile << endl;
        outfile << "correct = " << correctPredictions << " out of " << total << endl;
        outfile << " Total Accuracy => " << (double)(correctPredictions)/total << endl; //prints the accuracy as a decimal between 0 to 1
        outfile << endl;

        outfile.close();
        branchtrace.close(); 
    }
    //ending of branch prediction via array of counters


    //using branch history register that stores the outcome of the previous 2 branches
    //and predicts based on those values. uses the same 2bit counter cntState.
    
    //since we are using a 2 bit counter, the prediction for the current branch depends on the previous 2 branches if they are 
    //branched in the same way, but if their effects cancel out then the prediction depends on the previous 2 branches before them, and so on.
    //hence prediction depends on all the branches before and the initial state of the counter

    string BhrOut[4] = {"BHR00", "BHR01","BHR10", "BHR11"};
    for (int i = 0; i < 4; i++)
    {
        cntState BranchHistoryRegister(i); //i is the initial value of the branch history register counter
        ifstream branchtrace("branchtrace.txt");
        ofstream outfile(BhrOut[i] + ".txt");
        int correctPredictions = 0, total = 0;

        outfile << "Branch history register VARIANT where initially the counter is in state " << i << "\n\n";
        
        //now we will update the values of the counter that we have when we encounter a branch.
        while(branchtrace.good())
        {
            string hexPC; branchtrace >> hexPC;
            if(!branchtrace.good())
                break; //as then we have reached the end or a possible duplicate value
            total++;
            int taken; branchtrace >> taken;
            
            int prediction = BranchHistoryRegister.getState(); //getState returns the prediction of the counter object
            outfile << "Prediction for " << hexPC << " is =>" << prediction << " at state " << BranchHistoryRegister.cnt << " which is ";
            if(taken == prediction)
            {
                correctPredictions++;
                outfile << "CORRECT" << endl; //prints correct if the prediction matches
            }
            else
            {
                outfile << "WRONG" << endl; //else it prints wrong if the prediction was incorrect
            }
            BranchHistoryRegister.Update(taken);
        }

        outfile << endl;
        outfile << "correct = " << correctPredictions << " out of " << total << endl;
        outfile << " Total Accuracy => " << (double)(correctPredictions)/total << endl; //prints the accuracy as a decimal between 0 to 1
        outfile << endl;

        outfile.close();
        branchtrace.close(); 

    }
    
 



}