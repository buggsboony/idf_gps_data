#ifndef GPS_H
#define GPS_H

#include <iostream>
#include <vector>
#include <string>

#define MINUTE_LENGTH 9 // Length of minutes data in NMEA
#define KNOTS_TO_MPS 0.514444444
//#define SPEED_UNIT KNOTS_TO_MPS
#define SPEED_UNIT 1

#define private public

using namespace std;

class GPS
{
public:
    GPS();
    GPS(const string GGASentence,const string RMCSentence);
    int     UTC;
    double  latitude;
    double  longitude;
    double  altitude;
    double  quality;
    double  speed;
    double  heading;
    int     numberSatellites;
    int     date;

    //2023-06-25 10:57:59 - Explicite valid or not
    bool GGA_valid;
    bool RMC_valid;
    // 2023-06-25 12:27:09 - Check and save
    bool checkGGA(const string GGASentence, bool save=true);
    bool checkRMC(const string RMCSentence, bool save=true);    
    double rawCoordToDec(string array); //2023-06-25 20:45:06

private:
    // Set values of each sentence type

    // GGA sentences
    bool isValidGGA(const string GGASentence);
    void setValuesGGA(const string GGASentence);    

    // RMC sentence
    bool isValidRMC(const string RMCSentence);
    void setValuesRMC(const string RMCSentence);

    // Auxualiary functions
    //détect si une chaine débute avec une autre.
    bool stringStartsWith(string str, string pattern); //2023-06-25 12:32:01

    vector<string> splitStringByComma(const string, char comma=',');
    double stringToDouble(const string);
    double getCoordinates(string);

};

double degreesToDecimal(const int Degrees,const double Minutes,const int seconds = 0);

#endif // GPS_H
