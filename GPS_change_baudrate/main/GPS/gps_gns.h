#ifndef GPS_H
#define GPS_H

#include <iostream>
#include <vector>
#include <string>

#define MINUTE_LENGTH 9 // Length of minutes data in NMEA
#define KNOTS_TO_MPS 0.514444444
#define KNOTS_TO_KMH 1.852
//#define SPEED_UNIT KNOTS_TO_MPS
#define SPEED_UNIT KNOTS_TO_KMH
//#define SPEED_UNIT 1 //#KNOTS (default RMS GPS metric)

#define private public

using namespace std;


struct GPS_time {
    short hours;
    short minutes;
    short seconds;
    short milliseconds;
    short localOffsetHours; //2023-07-21 07:56:14 - Set offset hours for local hour. exemple: localOffsetHours -4 for GMT -4    
    //2023-07-20 18:07:16 - Constructeur : 
    GPS_time():hours(0),minutes(0),seconds(0),milliseconds(0),localOffsetHours(0) {}
     //Convert to readable string His.ms
    string toString(bool include_ms=true)
    {
        string str = "";
        if(hours<=9) str.append("0");
        str.append(std::to_string(hours) );
        str.append(":");

         if(minutes<=9) str.append("0");
        str.append(std::to_string(minutes) );
        str.append(":");

         if(seconds<=9) str.append("0");
        str.append(std::to_string(seconds) );        

        if(include_ms)
        {
            str.append(".");
            str.append(std::to_string(milliseconds));
        }         
        return str;
    }
   
   //2023-07-21 08:04:06 - convertToTimezone 
    static GPS_time convertToTimezone(GPS_time gps_utc, int offsetHours) {        
        // Ajouter l'offset horaire spécifié pour obtenir l'heure locale
        int hoursLocal = gps_utc.hours + offsetHours;
        int minutesLocal = (gps_utc.minutes + offsetHours * 60 + 60) % 60;
        int secondsLocal = (gps_utc.seconds + offsetHours * 3600 + 3600) % 60;

        // Ajuster les heures, les minutes et les secondes pour être dans des plages valides
        if (hoursLocal < 0) {
            hoursLocal += 24;
        }
        hoursLocal %= 24;
        minutesLocal = (minutesLocal + 60) % 60;
        secondsLocal = (secondsLocal + 60) % 60;

        GPS_time result;
        result.hours = hoursLocal;
        result.minutes = minutesLocal;
        result.seconds = secondsLocal;
        result.milliseconds = gps_utc.milliseconds; //2023-07-21 08:03:19 - keep ms
        return result;
    }
};

class GPS
{
public:
    GPS();
    GPS(const string GGASentence,const string RMCSentence);    
    short localOffsetHours;//2023-07-20 18:23:34 - spécify local utc offset
    int UTC;
    GPS_time time;
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
    GPS_time rawToTime(string His_dot_MS);
    string lastError;

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
