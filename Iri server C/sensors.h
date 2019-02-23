#include <stdio.h>
#include <stdbool.h>
//This class contains the blueprints of both Analog and Digital sensors so they can be generated in an object oriented way.
struct binarySensor
{
    char name[40];
    bool value;
	double epochTime;
	char alarm[40];
};

struct analogSensor
{
    char name[40];
    int value;
    char unit[15];
    double epochTime;
    int lowerBound;
    int higherBound;
	bool lowerBoundAlarm;
    bool higherBoundAlarm;
};


 static int changedSensorList[3000];
 static int changedSensorLastIndex = 0;

void generateSensors(char *sensorJson[], char *changedValueString[]);
char** str_split(char* a_str, const char a_delim);