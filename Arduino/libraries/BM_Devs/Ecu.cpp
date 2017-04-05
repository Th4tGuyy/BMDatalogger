//################################################################################
//################################################################################
//####                                                                        ####
//####                 	  ARDUINO ECTUNE DATALOGGER V1.4                 	  ####
//####                                                                        ####
//####   THIS IS A HONDA OBD1 DATALOGGER TO BE USED ON THE CN2 DATALOG PORT   ####
//####  OF THE ECU. THE DATALOGGING PROTOCOL IS ONLY FOR ECTUNE, SO YOU MUST  ####
//####      HAVE A CHIPPED ECU AND TUNED UNDER ECTUNE TUNING MANAGMENT.       ####
//####                                                                        ####
//####                   MADE BY BOULETMARC AND AGE-TUNING                    ####
//####                                                                        ####
//################################################################################
//################################################################################

#include "Arduino.h"
#include "Ecu.h"
#include "math.h"

//SET modable variables
int Injectors_Size 			= 240;		//Set the injectors size, used for consumptions
int mBarSeaLevel          	= 1013;
bool UseCelcius				= true;
bool UseKMH           		= true;
String Unit_Map           	= "mBar";	// mBar, Bar, inHgG, inHg, psi, kPa (type it with specifics caps)
String WBinput            	= "o2Input"; //o2Input, eldInput, egrInput, b6Input
long Timeout_Time 			= 500;		//Wait 500ms until the read array can be received otherwise call TimeOut


//################################################################################
//####                DONT EDIT ANYTHING BELLOW THIS LINE                     ####
//################################################################################
//SET unmodable variables
float ReadTime				= 0;
int CurrentTranny			= 0;
bool J12_Cut				= true;
bool Error					= false;
const int Array_Size 		= 52;
byte Datalog_Bytes[Array_Size];
byte Tranny_Bytes[10] = {0x46, 0x00, 0x67, 0x00, 0x8E, 0x00, 0xB8, 0x00, 0x52, 0xDE};
//Tranny_Bytes 				= {0x46, 0x00, 0x67, 0x00, 0x8E, 0x00, 0xB8, 0x00, 0x52, 0xDE};

//JUST SOMES TRANNY BYES INFOS
//0 = 'Y21/Y80/S80'				{0x46, 0x00, 0x67, 0x00, 0x8E, 0x00, 0xB8, 0x00, 0x52, 0xDE};
//1 = 'ITR S80 JDM 98-01'		{0x42, 0x00, 0x5D, 0x00, 0x87, 0x00, 0xB6, 0x00, 0x52, 0xDE};
//2 = 'ITR S80 USDM 97-01 '		{0x46, 0x00, 0x64, 0x00, 0x91, 0x00, 0xB8, 0x00, 0x52, 0xDE};
//3 = 'Z6/Y8'					{0x48, 0x00, 0x71, 0x00, 0xAA, 0x00, 0xE3, 0x00, 0x52, 0xDE};
//4 = 'GSR'						{0x46, 0x00, 0x6E, 0x00, 0x9A, 0x00, 0xC4, 0x00, 0x52, 0xDE};
//5 = 'LS/RS/GS/SE'				{0x48, 0x00, 0x71, 0x00, 0xB2, 0x00, 0xD6, 0x00, 0x52, 0xDE};
//6 = 'H22 JDM'					{0x47, 0x00, 0x6E, 0x00, 0x9E, 0x00, 0xC3, 0x00, 0x52, 0xDE};
//7 = 'H22 USDM/H23 JDM'		{0x47, 0x00, 0x6E, 0x00, 0x9E, 0x00, 0xC9, 0x00, 0x52, 0xDE};
//8 = 'H23 USDM'				{0x4A, 0x00, 0x7B, 0x00, 0xAF, 0x00, 0xE0, 0x00, 0x52, 0xDE};
//9 = 'D16Y7'					{0x4B, 0x00, 0x7D, 0x00, 0xBC, 0x00, 0xED, 0x00, 0x52, 0xDE};

//################################################################################
//################################################################################
//SET MAIN FUNCTIONS

Ecu::Ecu(){
}

void Ecu::Connect() {
	Serial3.begin(38400);
	Error = false;
}

void Ecu::Disconnect() {
	Serial3.end();
	Error = false;
}

bool Ecu::GetJ12Cut() {
	J12_Cut = true;
	
	Serial3.write((byte) 171);
	if ((int) Serial3.read() != 205) {
		//Serial3.end();
		J12_Cut = false;
		Error = true;
	}
	return J12_Cut;
}

void Ecu::GetData(){
	while(Serial3.available()) {
	  for (int i = 0; i < Array_Size; i++){
		Datalog_Bytes[i] = Serial3.read();
		delay(1);
	  }
	}
	Serial3.flush();
	Serial3.write(" ");
}

void Ecu::SendRead(){
	unsigned long PreviousTimeRead = millis();
	
	Serial3.flush();
	Serial3.write(" ");
	
	unsigned long PreviousTime = millis();
	unsigned long Time = millis() - PreviousTime;
	while(Serial3.available() != Array_Size && (Time < Timeout_Time)) Time = millis() - PreviousTime;
  
	if (Time >= Timeout_Time) {
		for (int i = 0; i < Array_Size; i++)
			Datalog_Bytes[i] = 0;
		
		ReadTime = (int) (millis() - PreviousTimeRead);
		Error = true;
	}
	
	for (int i = 0; i < Array_Size; i++){
		Datalog_Bytes[i] = Serial3.read();
		delay(1);
	}
	
	ReadTime = (int) (millis() - PreviousTimeRead);
	
	Error = false;
}

/*void Ecu::Send(){
	Serial3.flush();
	Serial3.write(" ");
}

void Ecu::Read(){
	Error = false;
	unsigned long PreviousTimeRead = millis();
	
	if (Serial3.available() >= Array_Size) {
		for (int i = 0; i < Array_Size; i++){
			Datalog_Bytes[i] = Serial3.read();
			//delay(1);
		}
	} else {
		Error = true;
	}
	
	ReadTime = (int) (millis() - PreviousTimeRead);
}

void Ecu::ReadAvailable(){
	Error = false;
	unsigned long PreviousTimeRead = millis();
	
	if (Serial3.available() > 0) {
		for (int i = 0; i < Serial3.available(); i++){
			Datalog_Bytes[i] = Serial3.read();
			//delay(1);
		}
	} else {
		Error = true;
	}
	
	ReadTime = (int) (millis() - PreviousTimeRead);
}

int Ecu::GetAvailable() {
	return (int) Serial3.available();
}

int Ecu::GetReadTime() {
	return ReadTime;
}*/

//################################################################################
//################################################################################
//SET VARIABLES FUNCTIONS

//int variables
void Ecu::SetInjSize(int Size) {
	Injectors_Size = Size;
}

int Ecu::GetInjSize() {
	return Injectors_Size;
}

void Ecu::SetSeaLevel(int Size) {
	mBarSeaLevel = Size;
}

int Ecu::GetSeaLevel() {
	return mBarSeaLevel;
}

void Ecu::SetTimeoutTime(int Size) {
	Timeout_Time = (long) Size;
}

int Ecu::GetTimeoutTime() {
	return (int) Timeout_Time;
}

//bool variables
void Ecu::SetUseCelcius(bool Use) {
	UseCelcius = Use;
}

bool Ecu::GetUseCelcius() {
	return UseCelcius;
}

void Ecu::SetUseKMH(bool Use) {
	UseKMH = Use;
}

bool Ecu::GetUseKMH() {
	return UseKMH;
}

bool Ecu::GetError() {
	return Error;
}

//string variables
void Ecu::SetUnitMap(String This) {
	Unit_Map = This;
}

String Ecu::GetUnitMap() {
	return Unit_Map;
}

void Ecu::SetWBinput(String This) {
	WBinput = This;
}

String Ecu::GetWBinput() {
	return WBinput;
}

void Ecu::SetTranny(int ThisTranny) {
	CurrentTranny = ThisTranny;
	if (ThisTranny == 0) {
		Tranny_Bytes[0] = 0x46;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x67;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0x8E;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xB8;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x46, 0x00, 0x67, 0x00, 0x8E, 0x00, 0xB8, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 1) {
		Tranny_Bytes[0] = 0x42;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x5D;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0x87;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xB6;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x42, 0x00, 0x5D, 0x00, 0x87, 0x00, 0xB6, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 2) {
		Tranny_Bytes[0] = 0x46;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x64;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0x91;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xB8;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x46, 0x00, 0x64, 0x00, 0x91, 0x00, 0xB8, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 3) {
		Tranny_Bytes[0] = 0x48;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x71;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0xAA;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xE3;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x48, 0x00, 0x71, 0x00, 0xAA, 0x00, 0xE3, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 4) {
		Tranny_Bytes[0] = 0x46;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x6E;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0x9A;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xC4;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x46, 0x00, 0x6E, 0x00, 0x9A, 0x00, 0xC4, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 5) {
		Tranny_Bytes[0] = 0x48;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x71;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0xB2;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xD6;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x48, 0x00, 0x71, 0x00, 0xB2, 0x00, 0xD6, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 6) {
		Tranny_Bytes[0] = 0x47;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x6E;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0x9E;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xC3;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x47, 0x00, 0x6E, 0x00, 0x9E, 0x00, 0xC3, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 7) {
		Tranny_Bytes[0] = 0x47;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x6E;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0x9E;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xC9;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x47, 0x00, 0x6E, 0x00, 0x9E, 0x00, 0xC9, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 8) {
		Tranny_Bytes[0] = 0x4A;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x7B;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0xAF;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xE0;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x4A, 0x00, 0x7B, 0x00, 0xAF, 0x00, 0xE0, 0x00, 0x52, 0xDE};
	} else if (ThisTranny == 9) {
		Tranny_Bytes[0] = 0x4B;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x7D;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0xBC;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xED;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x4B, 0x00, 0x7D, 0x00, 0xBC, 0x00, 0xED, 0x00, 0x52, 0xDE};
	} else if (ThisTranny > 9) {
		Tranny_Bytes[0] = 0x46;
		Tranny_Bytes[1] = 0x00;
		Tranny_Bytes[2] = 0x67;
		Tranny_Bytes[3] = 0x00;
		Tranny_Bytes[4] = 0x8E;
		Tranny_Bytes[5] = 0x00;
		Tranny_Bytes[6] = 0xB8;
		Tranny_Bytes[7] = 0x00;
		Tranny_Bytes[8] = 0x52;
		Tranny_Bytes[9] = 0xDE;
		//Tranny_Bytes = {0x46, 0x00, 0x67, 0x00, 0x8E, 0x00, 0xB8, 0x00, 0x52, 0xDE};
		CurrentTranny = 0;
	}
}

String Ecu::GetTranny() {
	if (CurrentTranny == 0)
		return "Y21/Y80/S80";
	else if (CurrentTranny == 1)
		return "ITR S80 JDM 98-01";
	else if (CurrentTranny == 2)
		return "ITR S80 USDM 97-01";
	else if (CurrentTranny == 3)
		return "Z6/Y8";
	else if (CurrentTranny == 4)
		return "GSR";
	else if (CurrentTranny == 5)
		return "LS/RS/GS/SE";
	else if (CurrentTranny == 6)
		return "H22 JDM";
	else if (CurrentTranny == 7)
		return "H22 USDM/H23 JDM";
	else if (CurrentTranny == 8)
		return "H23 USDM";
	else if (CurrentTranny == 9)
		return "D16Y7";
}
//################################################################################
//################################################################################
//SET MULTI USED FUNCTIONS

long Ecu::Long2Bytes(byte ThisByte1, byte ThisByte2) {
	return (long) ((long) ThisByte2 * 256 + (long) ThisByte1);
}

float Ecu::GetTemperature(byte ThisByte) {
	float ThisTemp = (float) ThisByte / 51;
	ThisTemp = (0.1423*pow(ThisTemp,6)) - (2.4938*pow(ThisTemp,5))  + (17.837*pow(ThisTemp,4)) - (68.698*pow(ThisTemp,3)) + (154.69*pow(ThisTemp,2)) - (232.75*ThisTemp) + 284.24;
	ThisTemp = ((ThisTemp - 32)*5)/9;

	if (!UseCelcius)
		ThisTemp = ThisTemp * 1.8 + 32.0;
	
	return ThisTemp;
	/*if (UseCelcius)
		return round(double_Temperature[(int) ThisByte]);
	return round(GetTemperatureF(double_Temperature[(int) ThisByte]));*/
}

double Ecu::GetVolt(byte ThisByte) {
	return round(((double) ThisByte * 0.0196078438311815) * 100) / 100;
}

float Ecu::GetDuration(int ThisInt) {
  return (float) ThisInt * 3.20000004768372 / 1000.0;
}

/*float Ecu::GetIC(byte ThisByte) {
	if ((int) ThisByte == 128)
		return 0.0f;
	if ((int) ThisByte < 128)
		return (float) (128 - (int) ThisByte) * -0.25f;
	return (float) ((int) ThisByte - 128) * 0.25f;
}

double Ecu::GetFC(long ThisByte, long ThisLong) {
  double num = (double) ThisByte / (double) ThisLong;
  //if (CorrectionUnits == "multi")
    return round((num) * 100) / 100;
  //return round(num * 100.0 - 100.0);
}

double Ecu::GetEBC(byte ThisByte) {
  double num = (double) ThisByte / 2.0;
  if (num > 100.0)
    num = 100.0;
  return round((num * 10)) / 10;
}*/

byte Ecu::GetActivated(byte ThisByte, int ThisPos, bool Reversed) {
	int bitArray[8];
	for (int i=0; i < 8; ++i ) {
		bitArray[i] = ThisByte & 1;
		ThisByte = ThisByte >> 1;
	}

	if (Reversed)
		return bitArray[ThisPos] ? (byte) 0 : (byte) 1;
	return bitArray[ThisPos] ? (byte) 1 : (byte) 0;
}

float Ecu::GetInstantConsumption(){
	float hundredkm = ((60 / GetVss()) * 100) / 60;     //minutes needed to travel 100km
	float fuelc = (hundredkm * ((Injectors_Size / 100) * GetDuty())) / 1000;     
	return constrain(fuelc, 0.0001, 50.0);
}

float Ecu::GetDuty(){
	return ((float) GetRpm() * (float) GetInj()) / 1200;
}

float Ecu::GetValueHG(int ThisInt) {
  return (float) round(((double) ThisInt * 0.02952999) * 10) / 10;
}

//################################################################################
//################################################################################
//SET 'GET' FUNCTIONS

unsigned int Ecu::GetEct(){
	return GetTemperature(Datalog_Bytes[0]);
}

unsigned int Ecu::GetIat(){
	return GetTemperature(Datalog_Bytes[1]);                  
}

float Ecu::GetO2(){
	//unsigned byte O2Byte = 0;
	if (WBinput == "o2Input") return round((float) GetVolt(Datalog_Bytes[2]) * 100) / 100;//O2Byte = Datalog_Bytes[2];
    if (WBinput == "eldInput") return round((float) GetVolt(Datalog_Bytes[24]) * 100) / 100;//O2Byte = Datalog_Bytes[24];
    if (WBinput == "egrInput") return round((float) GetVolt(Datalog_Bytes[44]) * 100) / 100;//O2Byte = Datalog_Bytes[44];   //byte_25
    if (WBinput == "b6Input") return round((float) GetVolt(Datalog_Bytes[45]) * 100) / 100;//O2Byte = Datalog_Bytes[45]; 
	//return round((float) GetVolt(O2Byte) * 100) / 100;
	//return ((2 * (float) O2Byte[2]) + 10) / 10f;
}

unsigned int Ecu::GetBaro(){
	return (int) round((double) ((int) Datalog_Bytes[3] / 2 + 24) * 7.221 - 59.0);
}

float Ecu::GetMap(){
	int ThisInt = (int) Datalog_Bytes[4];
	//if (Unit_Map == "mBar") return ((float) ThisInt * 2041) / 255;	//(1764/255) * (float) ThisInt;
	if (Unit_Map == "mBar") return 6.9176 * (float) ThisInt;
	else if (Unit_Map == "Bar") return (float) (ThisInt / 1000);
	else if (Unit_Map == "inHgG") return (float) round(-((double) GetValueHG(mBarSeaLevel) + (double) GetValueHG(ThisInt)) * 10) / 10;
	else if (Unit_Map == "inHg") return GetValueHG(ThisInt);
	else if (Unit_Map == "psi") return (float) round(((double) (ThisInt - mBarSeaLevel) * 0.0145037695765495) * 100) / 100; //GetValuePSI(ThisInt);
	else if (Unit_Map == "kPa") return (float) round((double) ThisInt * 0.1);	//GetValueKPa(ThisInt);
	else return 0;

	//float mapRaw = Datalog_Bytes[4];
	//return (1764/255)*mapRaw;
}

unsigned int Ecu::GetTps(){
	return (int) round(((double) Datalog_Bytes[5] - 25.0) / 2.04);
	//return constrain((0.4716  * Datalog_Bytes[5]) - 11.3184, 0, 100);    
}

unsigned int Ecu::GetRpm(){
	//return (int) (1875000/Long2Bytes(Datalog_Bytes[6], Datalog_Bytes[7]));
	return (int) (1851562/Long2Bytes(Datalog_Bytes[6], Datalog_Bytes[7]));  
}
//####
/*bool Ecu::GetPostFuel(){
	return (bool) GetActivated(Datalog_Bytes[8], 0, false);
}

bool Ecu::GetSCCChecker(){
	return (bool) GetActivated(Datalog_Bytes[8], 1, false);
}*/

bool Ecu::GetIgnCut(){
	return (bool) GetActivated(Datalog_Bytes[8], 2, false);
}

bool Ecu::GetVTSM(){
	return (bool) GetActivated(Datalog_Bytes[8], 3, false);
}

bool Ecu::GetFuelCut1(){
	return (bool) GetActivated(Datalog_Bytes[8], 4, false);
}

bool Ecu::GetFuelCut2(){
	return (bool) GetActivated(Datalog_Bytes[8], 5, false);
}

/*bool Ecu::GetATShift1(){
	return (bool) GetActivated(Datalog_Bytes[8], 6, false);
}

bool Ecu::GetATShift2(){
	return (bool) GetActivated(Datalog_Bytes[8], 7, false);
}*/
//####
unsigned int Ecu::GetVss(){
	if (UseKMH)
		return (int) Datalog_Bytes[16];
	return (int) round((float) Datalog_Bytes[16] / 1.6f);
}

double Ecu::GetInjFV() {
    return round(((double) Long2Bytes(Datalog_Bytes[17], Datalog_Bytes[18]) / 4.0) * 100) / 100;
}

float Ecu::GetInjectorDuty() {
	//Get rpm scalars for menus, not extracted the method yet
	//if (GetRpm() == 0)
	//  return this.method_159(ReturnRPM);

	return (float) ((double) GetRpm() * (double) GetDuration(Long2Bytes(Datalog_Bytes[17], Datalog_Bytes[18])) / 1200.0);
}

float Ecu::GetInj(){
	return (float) (Long2Bytes(Datalog_Bytes[17], Datalog_Bytes[18]) / 352);  
}

unsigned int Ecu::GetIgn(){
	return (0.25 * Datalog_Bytes[19]) - 6;
}

/*int Ecu::GetIgnTable(){
	return (0.25 * Datalog_Bytes[20]) - 6;
}
//####
bool Ecu::GetParkN(){
	return (bool) GetActivated(Datalog_Bytes[21], 0, false);
}

bool Ecu::GetBKSW(){
	return (bool) GetActivated(Datalog_Bytes[21], 1, false);
}

bool Ecu::GetACC(){
	return (bool) GetActivated(Datalog_Bytes[21], 2, false);
}*/

bool Ecu::GetVTP(){
	return (bool) GetActivated(Datalog_Bytes[21], 3, false);
}

/*bool Ecu::GetStart(){
	return (bool) GetActivated(Datalog_Bytes[21], 4, false);
}

bool Ecu::GetSCC(){
	return (bool) GetActivated(Datalog_Bytes[21], 5, false);
}

bool Ecu::GetVTSFeedBack(){
	return (bool) GetActivated(Datalog_Bytes[21], 6, false);
}

bool Ecu::GetPSP(){
	return (bool) GetActivated(Datalog_Bytes[21], 7, false);
}
//####
bool Ecu::GetFuelPump(){
	return (bool) GetActivated(Datalog_Bytes[22], 0, false);
}

bool Ecu::GetIAB(){
	return (bool) GetActivated(Datalog_Bytes[22], 2, false);
}*/

bool Ecu::GetFanCtrl(){
	return (bool) GetActivated(Datalog_Bytes[22], 4, false);
}

bool Ecu::GetAtlCtrl(){
	return (bool) GetActivated(Datalog_Bytes[22], 5, false);
}

/*bool Ecu::GetPurge(){
	return (bool) GetActivated(Datalog_Bytes[22], 6, false);
}

bool Ecu::GetAC(){
	return (bool) GetActivated(Datalog_Bytes[22], 7, false);
}*/
//####
bool Ecu::GetMIL(){
	return (bool) GetActivated(Datalog_Bytes[23], 5, false);
}

/*bool Ecu::GetO2Heater(){
	return (bool) GetActivated(Datalog_Bytes[23], 6, false);
}

bool Ecu::GetVTS(){
	return (bool) GetActivated(Datalog_Bytes[23], 7, false);
}
//####
double Ecu::GetELDVolt(){
	return GetVolt(Datalog_Bytes[24]);
}*/

float Ecu::GetBattery(){  
	return (26.0 * Datalog_Bytes[25]) / 270.0;
}

/*double Ecu::GetECTFC(){
	return GetFC(Datalog_Bytes[26], 128);
}

long Ecu::GetO2Short(){
	return (long) GetFC(Long2Bytes(Datalog_Bytes[27], Datalog_Bytes[28]), 32768);
}

long Ecu::GetO2Long(){
	return (long) GetFC(Long2Bytes(Datalog_Bytes[29], Datalog_Bytes[30]), 32768);
}

long Ecu::GetIATFC(){
	return (long) GetFC(Long2Bytes(Datalog_Bytes[31], Datalog_Bytes[32]), 32768);
}

double Ecu::GetVEFC(){
	return GetFC(Datalog_Bytes[33], 128);
}

float Ecu::GetIATIC(){
	return GetIC(Datalog_Bytes[34]);
}

float Ecu::GetECTIC(){
	return GetIC(Datalog_Bytes[35]);
}

float Ecu::GetGEARIC(){
	return GetIC(Datalog_Bytes[36]);
}*/
//####
bool Ecu::GetInputFTL(){
	return (bool) GetActivated(Datalog_Bytes[38], 0, false);
}

bool Ecu::GetInputFTS(){
	return (bool) GetActivated(Datalog_Bytes[38], 1, false);
}

bool Ecu::GetInputEBC(){
	return (bool) GetActivated(Datalog_Bytes[38], 2, false);
}

/*bool Ecu::GetInputEBCHi(){
	return (bool) GetActivated(Datalog_Bytes[38], 3, false);
}

bool Ecu::GetInputGPO1(){
	return (bool) GetActivated(Datalog_Bytes[38], 4, false);
}

bool Ecu::GetInputGPO2(){
	return (bool) GetActivated(Datalog_Bytes[38], 5, false);
}

bool Ecu::GetInputGPO3(){
	return (bool) GetActivated(Datalog_Bytes[38], 6, false);
}*/

bool Ecu::GetInputBST(){
	return (bool) GetActivated(Datalog_Bytes[38], 7, false);
}
//####
bool Ecu::GetOutputFTL(){
	return (bool) GetActivated(Datalog_Bytes[39], 0, false);
}

bool Ecu::GetOutputAntilag(){
	return (bool) GetActivated(Datalog_Bytes[39], 1, false);
}

bool Ecu::GetOutputFTS(){
	return (bool) GetActivated(Datalog_Bytes[39], 2, false);
}

bool Ecu::GetOutputBoostCut(){
	return (bool) GetActivated(Datalog_Bytes[39], 3, false);
}

bool Ecu::GetOutputEBC(){
	return (bool) GetActivated(Datalog_Bytes[39], 4, false);
}

bool Ecu::GetOutput2ndMap(){
	return (bool) GetActivated(Datalog_Bytes[39], 5, false);
}

bool Ecu::GetOutputFanCtrl(){
	return (bool) GetActivated(Datalog_Bytes[39], 6, false);
}

bool Ecu::GetOutputBST(){
	return (bool) GetActivated(Datalog_Bytes[39], 7, false);
}
//####
/*double Ecu::GetEBCBaseDuty(){
	return GetEBC(Datalog_Bytes[40]);
}

double Ecu::GetEBCDuty(){
	return GetEBC(Datalog_Bytes[41]);
}
//####
bool Ecu::GetOutputGPO1(){
	return (bool) GetActivated(Datalog_Bytes[43], 0, false);
}

bool Ecu::GetOutputGPO2(){
	return (bool) GetActivated(Datalog_Bytes[43], 1, false);
}

bool Ecu::GetOutputGPO3(){
	return (bool) GetActivated(Datalog_Bytes[43], 2, false);
}

bool Ecu::GetOutputBSTStage2(){
	return (bool) GetActivated(Datalog_Bytes[43], 3, false);
}

bool Ecu::GetOutputBSTStage3(){
	return (bool) GetActivated(Datalog_Bytes[43], 4, false);
}

bool Ecu::GetOutputBSTStage4(){
	return (bool) GetActivated(Datalog_Bytes[43], 5, false);
}*/

bool Ecu::GetLeanProtect(){
	return (bool) GetActivated(Datalog_Bytes[43], 7, false);
}
//####
double Ecu::GetIACVDuty(){
	return ((double) ((float) Long2Bytes(Datalog_Bytes[49], Datalog_Bytes[50]) / 32768) * 100.0 - 100.0);
}

//################################################################################
//################################################################################
//ADDED FUNCTIONS
double Ecu::GetMapVolt(){
	return GetVolt(Datalog_Bytes[4]);
}

double Ecu::GetTPSVolt(){
	return GetVolt(Datalog_Bytes[5]);
}

double Ecu::GetInjDuration(){
	return round(((double) GetDuration((int) Long2Bytes(Datalog_Bytes[17], Datalog_Bytes[18]))) * 100) / 100;
}

unsigned int Ecu::GetGear(){ 
	if (GetVss() == 0)
		return 0;

	long ThisRatio = (long) (GetVss() * 256) * (long) GetRpm() / (long) 256;
	byte Gear = 0;

	for (int i = 0; i < 4; i++) {
		int ThisIndex = i * 2;
				
		if (ThisRatio >= Tranny_Bytes[ThisIndex]) {
			Gear = i + 1;
			//return ThisRPM;
		} else {
			//Gear = ThisRPM;
			break;
		}	
	}

	return Gear;
}

//################################################################################
//################################################################################
// OLD UNUSED FUNCTIONS

/*float Ecu::GetValuePSI(int ThisInt) {
  return (float) round(((double) (ThisInt - mBarSeaLevel) * 0.0145037695765495) * 100) / 100;
}

float Ecu::GetValueKPa(int ThisInt) {
  return (float) round((double) ThisInt * 0.1);
}*/

/*float Ecu::GetTemperatureF(float ThisTemp) {
	return ThisTemp * 1.8f + 32.0f;
}*/

/*float Ecu::calcTempInCelsius(float Datalog_Bytes){
  Datalog_Bytes = Datalog_Bytes / 51;
  Datalog_Bytes = (0.1423*pow(Datalog_Bytes,6)) - (2.4938*pow(Datalog_Bytes,5))  + (17.837*pow(Datalog_Bytes,4)) - (68.698*pow(Datalog_Bytes,3)) + (154.69*pow(Datalog_Bytes,2)) - (232.75*Datalog_Bytes) + 284.24;
  return ((Datalog_Bytes - 32)*5)/9;
}*/

/*int Ecu::Getbit(){
	int firsteightbit = Datalog_Bytes[8];
	return firsteightbit;
}

int Ecu::Getbit2(){
	int firsteightbit2 = Datalog_Bytes[23];
	return firsteightbit2;
}*/


