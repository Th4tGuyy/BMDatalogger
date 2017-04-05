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

#ifndef Ecu_h
	#define Ecu_h

	#include "Arduino.h"
	#include "math.h"

	class Ecu {
		
	  public:
		Ecu();
		
		//Main functions
		void Connect();
		void Disconnect();
		bool GetJ12Cut();
		void GetData();
		void SendRead();
		//void Send();
		//void Read();
		//void ReadAvailable();
		//int GetAvailable();
		//int GetReadTime();
		
		//Variables functions
		void SetInjSize(int Size);
		int GetInjSize();
		void SetSeaLevel(int Size);
		int GetSeaLevel();
		void SetTimeoutTime(int Size);
		int GetTimeoutTime();
		void SetUseCelcius(bool Use);
		bool GetUseCelcius();
		void SetUseKMH(bool Use);
		bool GetUseKMH();
		bool GetError();
		void SetUnitMap(String This);
		String GetUnitMap();
		void SetWBinput(String This);
		String GetWBinput();
		void SetTranny(int ThisTranny);
		String GetTranny();
		 
		//Sensors functions
		unsigned int GetEct();
		unsigned int GetIat();
		float GetO2();
		unsigned int GetBaro();
		float GetMap();
		unsigned int GetTps();
		unsigned int GetRpm();
		
		//bool GetPostFuel();
		//bool GetSCCChecker();
		bool GetIgnCut();
		bool GetVTSM();
		bool GetFuelCut1();
		bool GetFuelCut2();
		//bool GetATShift1();
		//bool GetATShift2();
		
		unsigned int GetVss();
		double GetInjFV();
		double GetInjDuration();
		float GetInjectorDuty();
		float GetInj();
		unsigned int GetIgn();
		//int GetIgnTable();
		
		/*bool GetParkN();
		bool GetBKSW();
		bool GetACC();*/
		bool GetVTP();
		/*bool GetStart();
		bool GetSCC();
		bool GetVTSFeedBack();
		bool GetPSP();
		
		bool GetFuelPump();
		bool GetIAB();*/
		bool GetFanCtrl();
		bool GetAtlCtrl();
		//bool GetPurge();
		//bool GetAC();
		
		bool GetMIL();
		//bool GetO2Heater();
		bool GetVTS();
		
		//double GetELDVolt();
		float GetBattery();
		/*double GetECTFC();
		long GetO2Short();
		long GetO2Long();
		long GetIATFC();
		double GetVEFC();
		float GetIATIC();
		float GetECTIC();
		float GetGEARIC();*/
		
		bool GetInputFTL();
		bool GetInputFTS();
		bool GetInputEBC();
		/*bool GetInputEBCHi();
		bool GetInputGPO1();
		bool GetInputGPO2();
		bool GetInputGPO3();*/
		bool GetInputBST();
		
		bool GetOutputFTL();
		bool GetOutputAntilag();
		bool GetOutputFTS();
		bool GetOutputBoostCut();
		bool GetOutputEBC();
		bool GetOutput2ndMap();
		bool GetOutputFanCtrl();
		bool GetOutputBST();

		//double GetEBCBaseDuty();
		//double GetEBCDuty();
		
		/*bool GetOutputGPO1();
		bool GetOutputGPO2();
		bool GetOutputGPO3();
		bool GetOutputBSTStage2();
		bool GetOutputBSTStage3();
		bool GetOutputBSTStage4();*/
		bool GetLeanProtect();
		
		double GetIACVDuty();
		
		//Added functions
		double GetMapVolt();
		double GetTPSVolt();
		unsigned int GetGear();
		float GetInstantConsumption();
		
		//OLD UNUSED FUNCTION
		//int Getbit();
		//int Getbit2();
			
	  private:
		//multi shared functions
		long Long2Bytes(byte ThisByte1, byte ThisByte2);
		float GetTemperature(byte ThisByte);
		double GetVolt(byte ThisByte);
		//float GetIC(byte ThisByte);
		//double GetFC(long ThisByte, long ThisLong);
		//double GetEBC(byte ThisByte);
		byte GetActivated(byte ThisByte, int ThisPos, bool Reversed);
		float GetDuty();
		float GetValueHG(int ThisInt);
		float GetDuration(int ThisInt);

	};

#endif
