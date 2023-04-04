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
    num = (num && 16383); //done to get only the first 14 bits. 16383 is 2^14 - 1
    return num;
}

struct cntState //simple state for counters
{
    int cnt = 0;
    cntState(int starter)
    {
        cnt = starter;
    }

    int getState()
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

    int initialValueOfCounter = 0;
    vector<cntState> counters(16384, cntState(initialValueOfCounter)); //makes an array of 2^14 counters with the given starting state



    ifstream branchtrace; branchtrace.open("branchtrace.txt");
    ofstream outfile("cnt00.txt");
    int correctPredictions = 0;
    int total = 0;

    outfile << "VARIANT where initially all counters are in state 00-" << endl;
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
            
            int prediction = counters[index].getState();
            outfile << "Prediction for " << a << " is =>" << prediction << " at state " << counters[index].cnt << " which is ";
            branchtrace >> b;
            if((b[0] - '0') == prediction)
            {
                correctPredictions++;
                outfile << "CORRECT" << endl;
            }
            else
            {
                outfile << "WRONG" << endl;
            }
            counters[index].Update(b[0] - '0');
        }
    }

    outfile << endl;
    outfile << "correct = " << correctPredictions << " out of " << total << endl;
    outfile << " Total Accuracy => " << (double)(correctPredictions)/total << endl;
    outfile << endl;

    outfile.close();
}