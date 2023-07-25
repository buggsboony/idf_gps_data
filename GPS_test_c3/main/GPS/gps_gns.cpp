#include "gps_gns.h"
#include <assert.h>
#include <sstream>
#include <stdlib.h>




GPS::GPS()
{
    //Initializations
    UTC                 = 0;
    latitude            = 0;
    longitude           = 0;
    altitude            = 0;
    numberSatellites    = 0;
    speed               = 0;
    heading             = 0;
    date                = 0;
    //2023-06-25 10:57:59 - Explicite valid or not
    GGA_valid = false;
    RMC_valid = false;
    //2023-07-21 08:11:49 - local offset
    localOffsetHours = 0;
}

// Constructor using GGA and RMC sentences only. Assigns all data.
GPS::GPS(string GGASentence,string RMCSentence){
    if (isValidGGA(GGASentence)) setValuesGGA(GGASentence);else printf("GGA is not valid, oups (%s)\n",GGASentence.c_str());
    if (isValidRMC(RMCSentence)) setValuesRMC(RMCSentence);else printf("RMC is not valid, oups (%s)\n",RMCSentence.c_str() );

}

// Set values from GGA sentence

// Checks if GGA sentence is valid.
//2023-06-25 10:53:37 - Modified to detect GNGGA (Glonass)
bool GPS::isValidGGA(const string GGASentence){
        //puts("isValidGGA*******************");
        string err = "";
    bool returnBool = true;
    vector<std::string> elementVector = splitStringByComma(GGASentence);
        
        if((int)elementVector.size() <=0 )
        {
           lastError="No RMC elements found"; 
            printf("GGA not valid [%s]",GGASentence.c_str() );
            return false;
        }

        if (elementVector.size() != 15) {
            lastError="GGA element count is not 15"; 
         return false;}
        if ( (elementVector[0] != "$GPGGA") && (elementVector[0] != "$GNGGA") ) {
           lastError="Not GPGGA or GNGGA";
              return false;}
        if (atoi(elementVector[6].c_str()) == 0)        { 
            //printf("atoi failed 6!!!!!!!!!!\n"); 
        return false;}
        if (elementVector[4].length() < MINUTE_LENGTH)  { lastError="GGA element[4] length < MINUTE_LENGTH";  //printf("lement vector 4 failed !!!!!!!!!!\n");
         return false; }
        if (elementVector[2].length() < MINUTE_LENGTH)  { lastError="GGA element[2] length < MINUTE_LENGTH";  ///printf("element vector 2!!!!!!!!!!\n");
         return false; }
        if (atoi(elementVector[7].c_str()) == 0)        { lastError="GGA element[7] is empty";  ///printf("element vector 7!!!!!!!!!!\n");
          return false; }
 
    if( err != "")
    {
        cout<<"error:"<<err<<endl;
    }

    // if(returnBool)
    // {
    //     printlnErr("Ok GGA valid !");
    // }
    // if(!returnBool)
    // {
    //     printlnErr("Oups GGA not valid !");
    // }
    return returnBool;
}

// Input: GGA NMEA sentence
// Output: set values in class.
//2023-06-25 10:54:52 - Modified, to prevent exit on fail
void GPS::setValuesGGA(string GGA){
//puts("setValuesGGA*******************");
 this->GGA_valid = false;
    vector<std::string> elementVector = splitStringByComma(GGA);
    // Assert we have a GGA sentence
    //assert(elementVector[0] == "$GPGGA");
    if(  (elementVector[0] != "$GPGGA") && (elementVector[0] != "$GNGGA") )
    {
        //non valid GGA, GPS only nor Glonass)
       
        return;
    }

    this->UTC                 = atoi(elementVector[1].c_str()); //His.ms
    this->time  = rawToTime(elementVector[1]); //Converted version
    //this->latitude            = getCoordinates(elementVector[2]);
    this->latitude            = rawCoordToDec(elementVector[2]);
    if (elementVector[3] == "S") this->latitude  = -this->latitude;    
    this->longitude = rawCoordToDec(elementVector[4]);
    if (elementVector[5] == "W") this->longitude  = -this->longitude;
    this->altitude            = stringToDouble(elementVector[9]);
    this->quality            = stringToDouble(elementVector[6]); //2023-06-25 13:20:45
    this->numberSatellites    = atoi(elementVector[7].c_str());
    
    // cout<<"----------Number of Sats:"<<this->numberSatellites<<endl;
    // cout<<"----------Fix quality:"<<this->quality<<endl;    
    this->GGA_valid = true;
}


// RMC sentence

// Check if RMC sentence is valid with NMEA standard.
bool GPS::isValidRMC(const string RMCSentence){    
    puts("isValidRMC*******************");
    
    bool returnBool = true;
    vector<std::string> elementVector = splitStringByComma(RMCSentence);
    if((int)elementVector.size() <=0 )
    {
        printf("GGA not valid [%s]",RMCSentence.c_str() );
        lastError="No RMC elements found"; 
         return false;
    }
    if (elementVector.size() != 13)                 {lastError="RMC element count is not 13"; return false;}
    if ( (elementVector[0] != "$GPRMC") && (elementVector[0] != "$GNRMC") ) { lastError="Not GPRMC or GNRMC"; return false;}
    if (elementVector[2] != "A")                    { lastError="Invalid(from device)"; return false;}
    if (elementVector[3].length() < MINUTE_LENGTH)  { lastError="element[3] length < MINUTE_LENGTH"; return false;}
    if (elementVector[5].length() < MINUTE_LENGTH)  { lastError="element[5] length < MINUTE_LENGTH";  return false;}

    return returnBool;

}
//2023-06-25 11:03:42 - Modified to detect Glonass RMC
void GPS::setValuesRMC(const string RMCSentence){
    puts("setValuesRMC*******************");
    this->RMC_valid = false;
    vector<std::string> elementVector = splitStringByComma(RMCSentence);
    // Assert we have an RMC  sentence
    // assert(elementVector[0] == "$GPRMC");    
    if(  (elementVector[0] != "$GPRMC") && (elementVector[0] != "$GNRMC") )
    {
        //non valid GGA, GPS only nor Glonass)
        cout<<"GPRMC exit"<<endl;
        return;
    }

    this->UTC               = atoi(elementVector[1].c_str());
    this->time  = rawToTime(elementVector[1]); //Converted version
    //this->latitude          = getCoordinates(elementVector[3]);
    this->latitude          = rawCoordToDec(elementVector[3]);    
    if (elementVector[3] == "S") this->latitude  = -this->latitude;
    //this->longitude         = getCoordinates(elementVector[5]);
    this->longitude         = rawCoordToDec(elementVector[5]);
    if (elementVector[5] == "W") this->longitude  = -this->longitude;
    this->speed             = stringToDouble(elementVector[7])*SPEED_UNIT;
    this->heading           = stringToDouble(elementVector[8]);
    this->date              = atoi(elementVector[9].c_str());


    this->RMC_valid = true;
}

/*-----Auxiliary functions-----*/

// Input: coma separated string
// Output: Vector with all the elements in input.
vector<string> GPS::splitStringByComma(string input, char comma){

    vector<string>  returnVector;
    stringstream    ss(input);
    string          element;
        //cout<<"input ======["<<input<<"]"<<endl;
    while(std::getline(ss, element, comma)) {
        //cout<<"getline ======["<<element<<"]"<<endl;
        returnVector.push_back(element);
    }


    return returnVector;
}
double degreesToDecimal(int degrees, double minutes, int seconds )
{
    double returnDouble = 0;

    returnDouble = degrees + minutes/60 + seconds/3600.0f;

    return returnDouble;
}


double GPS::stringToDouble(string inputString){

    //If string empty, return 0.
    double returnValue = 0;
    std::istringstream istr(inputString);

    istr >> returnValue;

    return (returnValue);

}


//2023-06-25 20:06:25 - Convert gps lat or long data to decimal value
// Usage : 
    // double lat= rawCoordToDec(rawLat);  
    // double lng= rawCoordToDec(rawLng);  
double GPS::rawCoordToDec(string array)
{
    //cout<<"gpsCoord:"<<array<<endl;
    double decimalDegrees = 0;
   
    //2023-06-25 18:11:35 - Transform DMS format to split,//40 44 54.3 φ N, 73 59 9 λ W (DMS)
    vector<string> parts = splitStringByComma(array,'.');
    string deg_min = parts[0];   
    string smin = deg_min.substr(deg_min.length() - 2);
    string sdeg = deg_min.substr(0, deg_min.length() - smin.length() );
    string ssec= "0000";

    if(parts.size()>=2)//6909
    {
        ssec = parts[1]; //in fact this is not seconds, but the decimal part of minutes
        smin+="."+ssec;
    }

    // Convert input array into two sub arrays containing the degrees and the minutes
      int degrees = atoi(sdeg.c_str());
    double   minutes =  stringToDouble(smin);

    //cout<<"===["<<degrees<<"°"<<minutes<<"'"<<sec<<"\""<<"]==="<<endl;

    // Convert degrees and minutes into decimal
    decimalDegrees = degreesToDecimal(degrees,minutes);
    //cout<<"decimalDegrees:"<<decimalDegrees<<endl;
    return decimalDegrees;
}//rawCoordToDec


//2023-07-20 17:36:28 - Convert 213714.00 to 21:37:14.00
GPS_time GPS::rawToTime(string His_dot_MS)
{       
    string sH = His_dot_MS.substr(0,2);
    string si = His_dot_MS.substr(2,2);
    string ss = His_dot_MS.substr(4,2);
    string ms = His_dot_MS.substr(7);

    GPS_time gps_time_utc;
    gps_time_utc.hours = atoi(sH.c_str());
    gps_time_utc.minutes = atoi(si.c_str());
    gps_time_utc.seconds = atoi(ss.c_str());
    gps_time_utc.milliseconds = atoi(ms.c_str());
    
    if( localOffsetHours != 0 )
    {
        return GPS_time::convertToTimezone(gps_time_utc, localOffsetHours);    
    }else
    {
        return gps_time_utc;
    }
}//rawToMsTime


double GPS::getCoordinates(string array){


cout<<"gpsCoord:"<<array<<endl;
    double decimalDegrees = 0;
    string degreeArray;
    string minuteArray;
    string secondArray;//2023-06-25 13:49:24


    // Convert input array into two sub arrays containing the degrees and the minutes
    // Check for correct array length
    if (array.length() > MINUTE_LENGTH){

        degreeArray.assign(array.begin(), array.end() - MINUTE_LENGTH);
        minuteArray.assign(array.end() - MINUTE_LENGTH, array.end());

        // Convert strings into numbers
        int degrees;
        double minutes;
        degrees = atoi(degreeArray.c_str());
        minutes = stringToDouble(minuteArray);

        // Convert degrees and minutes into decimal
        decimalDegrees = degreesToDecimal(degrees,minutes);

    }
    //cout<<"decimalDegrees:"<<decimalDegrees<<endl;
    return decimalDegrees;

}

//détect si une chaine débute avec une autre.
bool GPS::stringStartsWith(string str, string pattern)
{
	bool result= false;
	if( str.length()>=pattern.length())
	{
		if(! str.substr(0,pattern.length()).compare(pattern) )
		{
			result = true;
		}
	}
	return result;
}

//2023-06-25 12:28:27 - More convinient
bool GPS::checkGGA(const string line, bool save)
{
    printf("Analyse de GGA --%s--\n",line.c_str() );
    bool valid = false;
    if(stringStartsWith(line,"$GPGGA,")||stringStartsWith(line,"$GNGGA,") )
    { 
        valid = this->isValidGGA(line);
        if( (valid)&& save)
        {
            this->setValuesGGA(line);
        }        
    }else
    {
        //printf("-----Not found GGA [%s]\n",line.c_str());
    }
    return valid;
}//checkGGA

bool GPS::checkRMC(const string line, bool save)
{    
    printf("Analyse de RMC --%s--\n",line.c_str() );
    bool valid = false;
    if(stringStartsWith(line,"$GPRMC,")||stringStartsWith(line,"$GNRMC,")  )
    {  
        valid = this->isValidRMC(line);
        if( (valid)&&save)
        {
            this->setValuesRMC(line);
        }
    }else
    {
        //printf("-----Not found RMC [%s]\n",line.c_str());
    }
    return valid;
}//checkRMC


