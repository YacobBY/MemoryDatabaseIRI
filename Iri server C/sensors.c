#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "sensors.h"
#include "jWrite.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
//This class contains the functions to generate all sensors and transform the entire database to a valid JSON string.
#pragma warning(disable:4996)

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

//This value contains the amount of Analog and Binary sensors. When changed more or less sensors will be generated.
#define BINARY_SENSOR_AMOUNT 5
#define ANALOG_SENSOR_AMOUNT 10

double  getTimeInMillis() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	double millisecondsSinceEpoch =
		(double)(tv.tv_sec) * 1000 +
		(double)(tv.tv_usec) / 1000;
	return millisecondsSinceEpoch;
}
//generateSensors generates all the sensor values, and then adjusts certain values like the temperature boundaries based on the changedValueString that was passed as a parameter.
void generateSensors(char *sensorJson[], char *changedValueString[])
{
	double times = getTimeInMillis();
	printf(" TIME IN MILIS %lf", times);
	//These variables are to change the sensor name for each new sensor. The variables will increment so the name changes eg. binarySensor001, binarySensor002 etc.
	int i;
	int firstDigit = 0;
	int secondDigit = 0;
	int thirdDigit = 1;
	//This struct is to get the current time in miliseconds

	//buffer holds the entire JSON string. 
	unsigned int buflen;
	buflen = 500000;
	int err;
	// start root Json object. In this case the Json consists of Two different objects (Analog and Digital Sensors) so we use an object as root not an array.
	jwOpen(sensorJson, buflen, JW_OBJECT, JW_PRETTY);
	jwObj_array("binarySensors");

	//First binarySensor name. firstDigit seconDigit and thirdDigit replace the last three digits of this string based on the current iteration number
	char binarySensorName[40] = "binarySensor000";
	int binaryInitialLength = strlen(binarySensorName);
	//generates the array of binarySensor structs
	struct binarySensor binarySensorArray[BINARY_SENSOR_AMOUNT];
	//I is het sensor nummer
	//generate the sensors
	//GENERATE BINARY SENSORS
	for (i = 0; i < BINARY_SENSOR_AMOUNT; ++i)
	{
		if (thirdDigit > 9)
		{
			++secondDigit;
			thirdDigit = 0;
		}
		if (secondDigit > 9)
		{
			++firstDigit;
			secondDigit = 0;
		}
		binarySensorName[-3 + binaryInitialLength] = firstDigit + '0';
		binarySensorName[-2 + binaryInitialLength] = secondDigit + '0';
		binarySensorName[-1 + binaryInitialLength] = thirdDigit + '0';
		thirdDigit++;

		
		//Fills the binary sensor with the base of its name, a generated false value which can be changed, and the current time.
		struct binarySensor temp = { "digitalSensor000",FALSE,getTimeInMillis(), "no alarm" };
		binarySensorArray[i] = temp;
		strncpy(binarySensorArray[i].name, binarySensorName, sizeof(binarySensorArray[i].name) - 1);

		//Randomly generates a true or false value for the binary sensors.
		int trueOrFalse = (rand() % (binaryInitialLength*2));
		if (trueOrFalse > binaryInitialLength)
		{
			binarySensorArray[i].value = TRUE;
	
			strncpy(binarySensorArray[i].alarm, "ALARM ON", sizeof(binarySensorArray[i].name) - 1);
		}
		//Makes a binarySensor object in the array and fills it with values of the current sensor.
		jwArr_object();
		jwObj_string("name", binarySensorArray[i].name);
		jwObj_bool("sensorValue", binarySensorArray[i].value);
		jwObj_double("timeStamp", binarySensorArray[i].epochTime);
		jwObj_string("alarm", binarySensorArray[i].alarm);
		//Closes this JSON object
		jwEnd();
	}
	//Closes the current object array of Binary sensors.
	jwEnd();

	//END BINARY SENSORS
	//______________________________________________________________________
	//BEGIN ANALOG SENSORS

	//Makes two arrays with upper and lower bounds based on the amount of analog sensors
	//This is done in a in a seperate array so we can change certain indeces later on, and read out the array changed array afterwards
	int analogSensorLowerBound[ANALOG_SENSOR_AMOUNT];
	int analogSensorUpperBound[ANALOG_SENSOR_AMOUNT];

	//Fill the arrays with base lower and upper bounds. 
	int z;
	for (z = 0; z < ANALOG_SENSOR_AMOUNT; ++z)
	{
		analogSensorLowerBound[z] = 40;
		analogSensorUpperBound[z] = 80;
	}
	//First name in array
	char analogSensorName[40] = "AnalogSensor000";
	int analogInitialLength = strlen(analogSensorName);
	struct analogSensor analogSensorArray[ANALOG_SENSOR_AMOUNT];
	//resets the counter digit values
	firstDigit = 0;
	secondDigit = 0;
	thirdDigit = 1;
	i = 0;
	//GENERATES ANALOG SENSORS
	for (i = 0; i < ANALOG_SENSOR_AMOUNT; ++i)
	{
		if (thirdDigit > 9)
		{
			++secondDigit;
			thirdDigit = 0;
		}
		if (secondDigit > 9)
		{
			++firstDigit;
			secondDigit = 0;
		}
		analogSensorName[-3 + analogInitialLength] = firstDigit + '0';
		analogSensorName[-2 + analogInitialLength] = secondDigit + '0';
		analogSensorName[-1 + analogInitialLength] = thirdDigit + '0';
		thirdDigit++;
		//Sensor Alarm values are written here.
		struct analogSensor temps = { "analogSensor000",50,"celcius",getTimeInMillis(), analogSensorLowerBound[i], analogSensorUpperBound[i], FALSE, FALSE };
		analogSensorArray[i] = temps;

		strncpy(analogSensorArray[i].name, analogSensorName, sizeof(analogSensorArray[i].name) - 1);
		analogSensorArray[i].value = (40 + rand() % 70);

		if (analogSensorArray[i].value < analogSensorArray[i].lowerBound)
		{
			analogSensorArray[i].lowerBoundAlarm = TRUE;
		}
		if (analogSensorArray[i].value > analogSensorArray[i].higherBound  )
		{
			analogSensorArray[i].higherBoundAlarm = TRUE;
		}
		
	}
	/*This is a comma seperator. It takes the static changedValueString in the parametres and adds it to the changedSensorList array
	  This array  is Static so it remains in between functions.*/
	if (("Stringcopmarison= %d\n\n", strcmp(changedValueString, "a"))!=0){
	char** tokens; 
	printf("changedValueString=[%s]\n\n", changedValueString);
	tokens = str_split(changedValueString, ',');
	if (tokens)
	{
		int i;
		int p;
		for (p = 0; *(tokens + p); p++)
		{
			changedSensorList[changedSensorLastIndex] = atoi(*(tokens + p));
			free(*(tokens + p));
			printf("changedIndex  %d %d\n", changedSensorLastIndex, changedSensorList[changedSensorLastIndex]);
			changedSensorLastIndex++;
		}
		printf("\n");
		printf("LastIndex %d\n", changedSensorLastIndex);
		free(tokens);
	}
	}
	//This part changes the analogSensor struct array and fills the lower and higher bound of the index with new values
	if (changedSensorLastIndex >0){
	int loopIndex;
	int currentChangedVal;
	for (int replacementCounter = 0; replacementCounter < changedSensorLastIndex; ++replacementCounter)
	{
		//The first number the changedSensorList is the index that needs to be changed. This value is saved in loopIndex
		loopIndex = changedSensorList[replacementCounter];
		//The counter of the changedSensorList is incremented
		++replacementCounter;
		printf(" loopIndex is %d lower value is %d\n", loopIndex, changedSensorList[replacementCounter]);
		//The second number the changedSensorList is the lowerbound that needs to be changed. This value is saved in loopIndex
		analogSensorArray[loopIndex].lowerBound = changedSensorList[replacementCounter];
		++replacementCounter;

		printf(" loopIndex is %d upper value is %d\n", loopIndex, changedSensorList[replacementCounter]);
		//The third number the changedSensorList is the higherBound that needs to be changed. This value is saved in loopIndex
		analogSensorArray[loopIndex].higherBound = changedSensorList[replacementCounter];
	}
	}
	//Creates a new Json array for the analogSensors
	jwObj_array("analogSensors");
	//fills the with analogSensor objects
	for (i = 0; i < ANALOG_SENSOR_AMOUNT; ++i) {
		jwArr_object();
		jwObj_string("name", analogSensorArray[i].name);
		jwObj_int("sensorValue", analogSensorArray[i].value);
		jwObj_string("unit", analogSensorArray[i].unit);
		jwObj_double("timeStamp", analogSensorArray[i].epochTime);
		jwObj_int("lowerBound", analogSensorArray[i].lowerBound);
		jwObj_int("upperBound", analogSensorArray[i].higherBound);
		jwObj_bool("lowerBoundAlarm", analogSensorArray[i].lowerBoundAlarm);
		jwObj_bool("upperBoundAlarm", analogSensorArray[i].higherBoundAlarm);
		//Ends the analogSensor object
		jwEnd();
	}
	//Ends the analogSensor Array
	jwEnd();
	//close the Json writer
	err = jwClose();

	if (err != JWRITE_OK)
		printf("Error: %s at function call %d\n", jwErrorToString(err), jwErrorPos());
	//prints sensorJson in the command prompt. This exact same string is also sent to the network.
	printf("%s", sensorJson);
}


struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag = 0;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		tmpres /= 10;  /*convert into microseconds*/
					   /*converting file time to unix epoch*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}
//comma splitting function for changed sensor method
char** str_split(char* a_str, const char a_delim)
{
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (a_delim == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);

	if (result)
	{
		size_t idx = 0;
		char* token = strtok(a_str, delim);

		while (token)
		{
			assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}