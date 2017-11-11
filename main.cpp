///\file main.cpp
///\brief FarMobile programming test main module

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <boost/tokenizer.hpp>
#include <time.h>
#include <unordered_map>
using namespace std;
using namespace boost;

//Trim string. Simplest solution found on Google.
//Wheel not re-invented
string trim(string instring)
{
    size_t startpos = instring.find_first_not_of(" \t");
    size_t endpos = instring.find_last_not_of(" \t");
    string str = instring.substr( startpos, endpos-startpos+1 );
    return str;
}




class Message
{
    public:
    enum MessageType {GPS,CAN} mMessageType;
    

};


class GPSMessage: public Message
{

};

class CANMessage: public Message
{


};


class CANStats
{
    unordered_map<time_t, unsigned int> mCANTimeMap; //[time, messagecount]
    unordered_map<unsigned int,unsigned int>mCANCount;   //[message_id, message_count]

    public:

    bool existsMsgID(unsigned int msgid)
    {
        if(mCANCount.find(msgid) == mCANCount.end())
        {
            return false;
        }
        return true;
    }

    bool existsTime(time_t tstamp)
    {
        if(mCANTimeMap.find(tstamp) == mCANTimeMap.end())
        {
            return false;
        }
        return true;
    }


   //If the msgid does not exist, create an entry with a count of 1
   //otherwise, increment the count for that msgid
   void addMsgID(unsigned int msgid,time_t tstamp)
   {
       if (existsMsgID(msgid))
       {
           mCANCount[msgid]++;
       }

       if(existsTime(tstamp))
       {
           mCANTimeMap[tstamp]++;
       }
    }


    unsigned int getMessageCount(time_t tstamp)
    {

        if (existsTime(tstamp))
        {
            return mCANTimeMap[tstamp];
        }
        return 0;
    }
};

enum FIELD_INDEX
{
    MESSAGE_ID,
    DLC,
    PAYLOAD,
    PUC_ID,
    TIMESTAMP,
    GPS_ID,
    LATITUDE,
    LONGITUDE,
    GROUNDSPEED,
    TRUECOURSE
};    



class StatData: public CANStats
{
    const int MAX_FIELD_COUNT=10;   //Fields greater than this are ignored 
    const int MAX_FIELD_SIZE=30;    //Assumption based on sample data
    time_t mStartTime;
    time_t mEndTime;

    unsigned int mCANCount;         //Number of parsed CAN messages
    unsigned int mUniqueCANCount;   //Unique CAN messages
    unsigned int mGPSCount;         //Number of parsed GPS messages

    public:


    StatData(void)
    {
        mStartTime=0;
        mCANCount=0;
        mUniqueCANCount=0;
        mGPSCount=0;
    }


    void parseFields(string line, string fields[])
    {

        typedef boost::tokenizer<boost::escaped_list_separator<char>> tokenizer;
        tokenizer t{line};

        //Fill array with pointers to fields for convience
        int i=0;
        for (tokenizer::iterator iter=t.begin();iter != t.end(); ++iter)
        {
            if (MAX_FIELD_COUNT == i)
            {
                printf("Entry greater than %d fields\n",MAX_FIELD_COUNT);
                break;
            }
            const char *str=iter->c_str();
            if (*str) //String not empty
            {
                printf("%s\n",iter->c_str());
                fields[i]=trim(iter->c_str());
            }

            i++;
        }
    }

    void addEntry(string line)
    {
        string fields[MAX_FIELD_COUNT];
        time_t t_epoch;
        parseFields(line,fields);

        t_epoch=0;
        if (fields[TIMESTAMP].length())
        {
            struct tm timestamp;
            strptime(fields[TIMESTAMP].c_str(),"%Y-%m-%d %T",&timestamp);
            t_epoch=mktime(&timestamp);
            printf("%lld:\n",t_epoch);
            if (!mStartTime)
            {
                mStartTime=t_epoch;
            }
            mEndTime=t_epoch;         //This is the last time if we don't get another entry
        }
        else
        {
            printf("No timestamp: %s\n",line.c_str());
            return; //Ignore
        }

        //Assume data is either GPS or CAN and no invalid entriesl
        if (fields[MESSAGE_ID].length()) //Discriminant.  First field is empty=GPS, otherwise CAN
        {
            unsigned int msgid=strtoul(fields[MESSAGE_ID].c_str(),NULL,16);
            printf("CAN:%s\n",line.c_str());
            mCANCount++;

            if (!existsMsgID(msgid))
            {
                mUniqueCANCount++;
            }

            addMsgID(msgid,t_epoch);
        }
        else
        {
            printf("GPS:%s\n",line.c_str());
            mGPSCount++;
        }
    }

    unsigned int getCANCount(void)
    {
        return mCANCount;
    }
    unsigned int getGPSount(void)
    {
        return mGPSCount;
    }


    time_t getStartTime(void)
    {
        return mStartTime;
    }


    time_t getEndTime(void)
    {
        return mEndTime;
    }


};



int main(int argc, char** argv)
{
    string filename = "gps_can_data.csv";
    string line;
    ifstream infile;

    StatData sd;

    infile.open(filename);
    if (infile.fail())
    {
        cout <<"Unable to open" << filename << endl;
        exit(0);
    }

    //Assume there is at least one line, don't check for EOF yet
    //And throw away field labels
    getline(infile,line);
    while (!infile.eof())
    {
        getline(infile,line);
        sd.addEntry(line);
    }

    printf("\n");
    

}
