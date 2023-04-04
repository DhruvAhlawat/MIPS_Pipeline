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

int main()
{
    cout << getLeast14("b77a8a3a") << endl;
    cout << getLeast14("b77be7ab");
    cout << getLeast14("b77b55a0");
    cout << getLeast14("b77b55e2");
}