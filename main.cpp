///\file main.cpp
///\brief FarMobile programming test main module

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <boost/tokenizer.hpp>
#include <time.h>
#include <limits.h>
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



void time_tToStruct_tm(time_t timet,struct tm strcttm)
{
    gmtime_r(&timet, &strcttm);
}

string timeString(time_t timet)
{
    struct tm strcttm;
    char str[50];
    time_tToStruct_tm(timet,strcttm);

    strftime(str,50,"Y-%m-%d %T",&strcttm);
    return string(str);
}

class CANStats
{
    unordered_map<time_t, unsigned int> mCANTimeMap; //[time, messagecount]
    unordered_map<unsigned int,unsigned int>mCANCount;   //[message_id, message_count]

    public:

    bool existsMsgID(unsigned int msgid) const
    {
        if(mCANCount.find(msgid) == mCANCount.end())
        {
            return false;
        }
        return true;
    }

    bool existsTime(time_t tstamp) const
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

    unsigned int getCANMessageCount(time_t tstamp) 
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
                //printf("%s\n",iter->c_str());
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
            //printf("%lld:\n",t_epoch);
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
            //printf("CAN:%s\n",line.c_str());
            mCANCount++;

            if (!existsMsgID(msgid))
            {
                mUniqueCANCount++;
            }

            addMsgID(msgid,t_epoch);
        }
        else
        {
            //printf("GPS:%s\n",line.c_str());
            mGPSCount++;
        }
    }

    unsigned int getCANCount(void) const
    {
        return mCANCount;
    }
    
    unsigned int getUniqueCANCount(void) const
    {
        return mUniqueCANCount;
    }


    unsigned int getGPSCount(void) const
    {
        return mGPSCount;
    }


    time_t getStartTime(void) const
    {
        return mStartTime;
    }


    time_t getEndTime(void) const
    {
        return mEndTime;
    }
    
    string getEndTimeString(void) const
    {
        struct tm etime;
        char tmp[50];
        gmtime_r(&mEndTime,&etime);
        strftime(tmp,50,"%Y-%m-%d %T",&etime);

        return string(tmp);
    }

    string getStartTimeString(void) const
    {
        struct tm etime;
        char tmp[50];
        gmtime_r(&mStartTime,&etime);
        strftime(tmp,50,"%Y-%m-%d %T",&etime);
        return string(tmp);
    }

    unsigned int getRunTimeInt(void)
    {
        return mEndTime-mStartTime;
    }
    string getRunTime(void) const
    {
        unsigned int delta_time=mEndTime-mStartTime;
        char str[50];

        //Quick conversion to delta time HH:MM:SS (No support for days)
        snprintf(str,50,"%02d:%02d:%02d",delta_time/3600, (delta_time%3600)/60,(delta_time%3600)%60);

        return string(str);

    }

    void getMinMaxCounts(time_t& mintime, time_t maxtime)
    {
        mintime=getEndTime();
        maxtime=getEndTime();
        unsigned int minval;
        unsigned int maxval=UINT_MAX;
        time_t tstamp=getStartTime();

        while (tstamp <= getEndTime())
        {
           if (existsTime(tstamp))
           {
               if (getCANMessageCount(tstamp)<maxval)
               {
                   maxtime=tstamp;
                   maxval=getCANMessageCount(tstamp);
               }

               if (getCANMessageCount(tstamp)>minval)
               {
                   mintime=tstamp;
                   minval=getCANMessageCount(tstamp);
               }
            }

        }
    }
};



int main(int argc, char** argv)
{

    if (argc < 2)
    {
        printf("Usage: %s <inputfile>\n",argv[0]);
        exit(1);
    }
    string filename = argv[1];

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

    printf("Run time: %s\n",sd.getRunTime().c_str());
    printf("CAN Message count: %d\n",sd.getCANCount()),
    printf("Unique CAN Message count: %d\n",sd.getUniqueCANCount());
    printf("GPS Message count: %d\n",sd.getGPSCount());

    if (sd.getRunTimeInt())
    {
        printf("CAN Messages/sec: %3.2f\n",static_cast<float>(sd.getCANCount())/sd.getRunTimeInt());
    }
    else
    {
        printf("Run time = 0\n");
    }
   
    if (sd.getRunTimeInt())
    {
        printf("CAN Messages/GPS: %3.2f\n",static_cast<float>(sd.getCANCount())/sd.getGPSCount());
    }
    else
    {
        printf("GPS message count  = 0\n");
    }
    



}
