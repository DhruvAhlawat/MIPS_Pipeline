#include<BranchPredictor.hpp>

int main()
{
    string cntOut[4] = {"cnt00.txt", "cnt01.txt", "cnt10.txt","cnt11.txt"};
    for (int i = 0; i < 4; i++)
    {
        int initialValueOfCounter = i; //makes an array of 2^14 counters with the given starting state
        SaturatingBranchPredictor counters = (SaturatingBranchPredictor(initialValueOfCounter));
        ifstream branchtrace; 
        branchtrace.open("branchtrace.txt");
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
            string a; string b;
            branchtrace >> a;
            if(branchtrace.good()) //because sometimes the >> operator duplicates the last one
            {
                total++;
                uint32_t index  = getLeast14(a);
                outfile<<index<<" ";
                bool prediction = counters.predict(index);
                outfile << "Prediction for " << a << " is =>" << prediction << " at state " << b << " which is ";
                branchtrace >> b;
                if( b == to_string(prediction))
                {
                    outfile<<"b"<<" "<<prediction;
                    correctPredictions++;
                    outfile << "CORRECT" << endl; //prints correct if the prediction matches
                }
                else
                {
                    outfile<<"b"<<" "<<prediction;
                    outfile << "WRONG" << endl; //else it prints wrong if the prediction was incorrect
                }
                counters.update(index,stoi(b));
            }
        }
           
        outfile << endl;
        outfile << "correct = " << correctPredictions << " out of " << total << endl;
        outfile << " Total Accuracy => " << (double)(correctPredictions)/total << endl; //prints the accuracy as a decimal between 0 to 1
        outfile << endl;

        outfile.close();
        branchtrace.close(); 
    }
}
