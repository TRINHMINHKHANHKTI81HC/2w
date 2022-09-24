#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include <iterator>
#include <algorithm>

using namespace std;
int DecToHexDec(int dec, int);
char hexaDecNum[50];
string messageData[5];
string arrangeHex(int i);

template <size_t N>

//Split string and add to array
void splitString(string (&arr)[N], string str)
{
    int n = 0;
    istringstream iss(str);
    for (auto it = istream_iterator<string>(iss); it != istream_iterator<string>() && n < N; ++it, ++n)
        arr[n] = *it;
}

//Use to check start 
bool startswith(const string& str, const string& cmp)
{
  return str.compare(0, cmp.length(), cmp) == 0;
}

//Extract messages and signals from dbc file into dbcConfig.txt file 
void extractMessagesignal(string dbcFile){

    // Extract messages and signals from dbc file
    string line;

    // Change input file whenever change dbc file
    ifstream myfile (dbcFile);
    if (myfile.is_open())
    {   
        //Clear content in CAN_config.txt
        string outputFile = "CAN_config.txt";
        ofstream ofs;
        ofs.open(outputFile, std::ofstream::out | std::ofstream::trunc);
        ofs.close();

        //Read line in dbcConfig
        while (getline (myfile, line))
        {   
            string signalData[8];
            if (startswith(line,"BO_")){
                string message = "" + line;
                splitString(messageData,message);
                ofstream dbcfile(outputFile, ios::app);
            }else if (startswith(line," SG_")) {
                string signal =  "" +line;
                splitString(signalData,signal);
                ofstream dbcfile(outputFile, ios::app);
                //Convert string data to integer for Hexadecimal conversion 
                int i;
                int decimalNum = stoi(messageData[1]); 
                i = DecToHexDec(decimalNum,0);
                dbcfile << arrangeHex(i) << " " << messageData[3] << " "
                        << signalData[1] << " " << signalData[3] << " "
                        << signalData[4] << signalData[5] << " \n";
            }
        }
        myfile.close();
    }
    else{
        cout << "Unable to open file " + dbcFile;
    }   
}

void deleteSpecialcharacters(string txtFile){
    string line;

    // Read input file CAN_config.txt
    ifstream myfile (txtFile);
    if (myfile.is_open())
    {   
        ofstream newfile("CAN_configfinal.txt");
 
      //Delete unnecessary characters for parsing data
        while(getline(myfile, line)){
       //while succesful read
       replace(line.begin(), line.end(), '@', ' ');
       replace(line.begin(), line.end(), '|', ' ');
       replace(line.begin(), line.end(), '(', ' ');
       replace(line.begin(), line.end(), ')', ' ');
       replace(line.begin(), line.end(), '[', ' ');
       replace(line.begin(), line.end(), ']', ' ');
       replace(line.begin(), line.end(), '-', ' ');
       replace(line.begin(), line.end(), '+', ' ');
       replace(line.begin(), line.end(), ',', ' ');
       newfile << line << endl;
   }
   myfile.close();
   }
   else{
        cout << "Unable to open file ";
   }
    
    //Remove unnecessary files 
    int status = remove("CAN_config.txt");
    if(status==0)
        cout<<"\nFile Deleted Successfully!";
    else
        cout<<"\nError Occurred!";
    cout<<endl;
    rename("CAN_configfinal.txt", "CAN_config.txt");
}

int main(){

    //Change dbcFile name when have new dbc file
    string dbcFile = "CAN_newdb.dbc";
    string txtFile = "CAN_config.txt";
    extractMessagesignal(dbcFile);
    deleteSpecialcharacters(txtFile);
    int i;
    return 0;
}

int DecToHexDec(int dec, int i){

    //Change CanID information from Decimal to Hexadecimal
    int rem;
    while(dec!=0)
    {
        rem = dec%16;
        if(rem<10)
            rem = rem+48;
        else
            rem = rem+55;
        hexaDecNum[i] = rem;
        i++;
        dec = dec/16;
    }
    return i;
}

string arrangeHex(int i){
    //Arrange HexaDeciaml in reverse order
    string temp;
    for(i=i-1; i>=0; i--){
        temp += hexaDecNum[i];}
    return temp;
}

