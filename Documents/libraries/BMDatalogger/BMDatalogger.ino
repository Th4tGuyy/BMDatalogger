//#####################################################
//#####################################################
//#####################################################
/*
 *         THIS IS THE BM DEVS DATALOGGER
 * 
 * THIS WORKS BY DATALOGGING ECTUNE ISR V3 PROTOCOL ON
 * THE ARDUINO SERIAL PORT. THE SERIAL PORT IS SET TO 3
 * SINCE THIS WAS DEVELOPPED ON ARDUINO MEGA. THIS WORKS
 * WITH 2X BUTTONS NAVIGATION ONLY. IT CAN SUPPORT 16X02
 * AND 20X04 LCD SCREEN TYPE.
 * 
 * ####################################################
 *    WIRING DIAGRAM :
 * LCD VSS pin to Gnd
 * LVD VDD pin to 5v
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 10
 * LCD D5 pin to digital pin 9
 * LCD D6 pin to digital pin 8
 * LCD D7 pin to digital pin 7
 * BUTTON1 pin to digital pin 6
 * BUTTON2 pin o digital pin 5
 * LCD R/W pin to Gnd
 * LCD A pin to 5v
 * LCD K pin to Gnd
 * 
 *  ####################################################
 *    ALL CREDITS GOES TO :
 * -Bouletmarc (BM Devs)
 * -Majidi
*/

// TRANNY TYPES REFERENCES :
//0 = 'Y21/Y80/S80'
//1 = 'ITR S80 JDM 98-01'
//2 = 'ITR S80 USDM 97-01 '
//3 = 'Z6/Y8'
//4 = 'GSR'
//5 = 'LS/RS/GS/SE'
//6 = 'H22 JDM'
//7 = 'H22 USDM/H23 JDM'
//8 = 'H23 USDM'
//9 = 'D16Y7'
//#####################################################
//#####################################################
//#####################################################

String VersionStr = "V1.0.4";
bool BypassJ12Error = false;
bool Is20x04Screen = false;     //Set to True is you run on a 20x04 Screen otherwise its a 16x02
int InjectorsSize = 450;
int TrannyType = 5;             

//Load Modules
#include "Wire.h"
#include "LiquidCrystal.h"
#include "Ecu.h"
#include <LcdBarGraph.h>
#include <stdio.h>
#include <avr/pgmspace.h>
Ecu ecu;

//Set Inputs
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
int TopButton = 6;
int BottomButton = 5;

//Buttons Vars
#define ON 1
#define OFF 0
volatile int buttonTop = 0;
volatile int buttonBottom = 0;
unsigned long last_interrupt_time=0;
int debouncing = 50;

//Screen Vars
int ScreenCurrentIndex = 0;
int ScreenIndex[8];       //Display Index for 8x values (or 2x values for 16x2)
int ScreenMaxIndex = 35;  //Maximum datalogs values
int ScreenMaxPx = 16;
int ScreenMaxPy = 2;
int ScreenMaxLines = 2;

bool EcuConnected = false; //is Ecu Connected ?

//#####################################################

void setup() {
  //Serial3.begin(38400);
  ecu.Connect();
  
  if (!EcuConnected & BypassJ12Error) EcuConnected = true;  //Don't check for J12 Error

  ecu.SetInjSize(InjectorsSize);
  ecu.SetTranny(TrannyType);
  
  //Reset Screen Pixel/Lines Size for 20x04
  if (Is20x04Screen) {
    ScreenMaxPx = 20;
    ScreenMaxPy = 4;
    ScreenMaxLines = 8; //Show 8 Values (4x Lines of 2x Values)
  }

  //Initialize Screen Indexes
  for (int i=0; i<ScreenMaxLines; i++) ScreenIndex[i] = i;

  //Set Buttons Pinout
  pinMode(TopButton,INPUT_PULLUP);  
  pinMode(BottomButton,INPUT_PULLUP);

  //Start LCD Display
  lcd.begin(ScreenMaxPx, ScreenMaxPy);
  show_flash();
  lcd.clear();
}

void loop() {
  delayMicroseconds(100000);
  //delay(100);
  //ecu.GetData();
  
  if (!EcuConnected) {
    if (!Is20x04Screen) {
      lcd.setCursor(0,0);
      lcd.print(" NOT CONNECTED");
      lcd.setCursor(0,1);
      lcd.print("IS J12 INPLACE?");
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print(" NOT CONNECTED");
      lcd.setCursor(0,2);
      lcd.print("IS J12 INPLACE?");
      lcd.setCursor(0,3);
      lcd.print("                ");
    }
    EcuConnected = ecu.GetJ12Cut();

    //Buttons will bypass J12 Check
    GetButtonTopState();
    GetButtonBottomState();
  }
  if (EcuConnected) {
    //ecu.SendRead(); //Send 0x30 and Read the Datalog Array at same time
    ecu.GetData();
    GetButtonTopState();
    GetButtonBottomState();
    GetButtonStates();
    execScreen();
  }
}

//#####################################################

void GetButtonStates() {
  if (buttonTop == ON){
    //If we are not into 20x04 Mode, then we always change the Line0
    if (!Is20x04Screen) ScreenCurrentIndex = 0;

    //Increase Index
    ScreenIndex[ScreenCurrentIndex]++;

    //Check If the Index is not already been in use
    if (!Is20x04Screen) {
      if(ScreenIndex[1] == ScreenIndex[0]) ScreenIndex[ScreenCurrentIndex]++;
    }
    else {
      for (int i=0; i<ScreenMaxPy; i++)
        if (i != ScreenCurrentIndex)
          if(ScreenIndex[ScreenCurrentIndex] == ScreenIndex[i]) ScreenIndex[ScreenCurrentIndex]++;
    }
        
    if(ScreenIndex[ScreenCurrentIndex] > ScreenMaxIndex) ScreenIndex[ScreenCurrentIndex] = 1;
    buttonTop=OFF;

    //else if (actualScreen == 7) init_big_font(&lcd);
    //if (actualScreen == 2) LcdBarGraph progressBar(&lcd, 16);
  }
  if(buttonBottom == ON){
    if (!Is20x04Screen) {
      ScreenCurrentIndex = 1;   //Set Line1 since we are not into 20x04 Mode

      //Increase Index
      ScreenIndex[ScreenCurrentIndex]++;
      if(ScreenIndex[1] == ScreenIndex[0]) ScreenIndex[ScreenCurrentIndex]++;
      if(ScreenIndex[ScreenCurrentIndex] > ScreenMaxIndex) ScreenIndex[ScreenCurrentIndex] = 1;
    }
    else {
      //When been in 20x04 mode, the 2nd button serve to switch to the next lines
      ScreenCurrentIndex++;
      if(ScreenIndex[ScreenCurrentIndex] > (ScreenMaxLines - 1)) ScreenCurrentIndex = 0;
    }
    buttonBottom=OFF;
  }
}

void execScreen(){
  //Running 2x Loop (Top & Bottom Screen) .... OR 8x Loop for 20x04 Display
  for (int i=0; i<ScreenMaxLines; i++) {
    //Get Text Index
    int ThisScreenIndex = ScreenIndex[i];
    
    //Set Text
    String Text = "";
    if (ThisScreenIndex == 0) Text += "    RPM:" + String(ecu.GetRpm());
    if (ThisScreenIndex == 1) Text += "    ECT:" + String(ecu.GetEct()) + "C";
    if (ThisScreenIndex == 2) Text += "    IAT:" + String(ecu.GetIat()) + "C";
    if (ThisScreenIndex == 3) Text += "   TPS:" + String(ecu.GetTps()) + "%";
    if (ThisScreenIndex == 4) Text += "    O2:" + String(ecu.GetO2());
    if (ThisScreenIndex == 5) Text += "    IGN:" + String(ecu.GetIgn());
    if (ThisScreenIndex == 6) Text += "  INJ:" + String(ecu.GetInj()) + "ms";
    if (ThisScreenIndex == 7) Text += "  FUEL:" + String(ecu.GetInstantConsumption()) + "L";
    if (ThisScreenIndex == 8) Text += "   DTY:" + String(ecu.GetInjectorDuty());
    if (ThisScreenIndex == 9) Text += "    MAP:" + String(ecu.GetMap());
    if (ThisScreenIndex == 10) Text += " VSS:" + String(ecu.GetVss()) + "kmh";
    if (ThisScreenIndex == 11) Text += "    Gear:" + String(ecu.GetGear());
    if (ThisScreenIndex == 12) Text += "  BATT:" + String(ecu.GetBattery());
    if (ThisScreenIndex == 13) Text += "    VTSM:" + String(ecu.GetVTSM());
    if (ThisScreenIndex == 14) Text += "    ICUT:" + String(ecu.GetIgnCut());
    if (ThisScreenIndex == 15) Text += "    FCUT:" + String(ecu.GetFuelCut1());
    if (ThisScreenIndex == 16) Text += "   FCUT2:" + String(ecu.GetFuelCut2());
    if (ThisScreenIndex == 17) Text += " INJFV:" + String(ecu.GetInjFV());
    if (ThisScreenIndex == 18) Text += "   INJD:" + String(ecu.GetInjDuration());
    if (ThisScreenIndex == 19) Text += "    VTP:" + String(ecu.GetVTP());
    if (ThisScreenIndex == 20) Text += "   FCTRL:" + String(ecu.GetFanCtrl());
    if (ThisScreenIndex == 21) Text += "   ACTRL:" + String(ecu.GetAtlCtrl());
    if (ThisScreenIndex == 22) Text += "    MIL:" + String(ecu.GetMIL());
    if (ThisScreenIndex == 23) Text += "    FTLI:" + String(ecu.GetInputFTL());
    if (ThisScreenIndex == 24) Text += "    FTSI:" + String(ecu.GetInputFTS());
    if (ThisScreenIndex == 25) Text += "    EBCI:" + String(ecu.GetInputEBC());
    if (ThisScreenIndex == 26) Text += "    BSTI:" + String(ecu.GetInputBST());
    if (ThisScreenIndex == 27) Text += "    FTLO:" + String(ecu.GetOutputFTL());
    if (ThisScreenIndex == 28) Text += "    ATLG:" + String(ecu.GetOutputAntilag());
    if (ThisScreenIndex == 29) Text += "    FTSO:" + String(ecu.GetOutputFTS());
    if (ThisScreenIndex == 30) Text += "   BSTCT:" + String(ecu.GetOutputBoostCut());
    if (ThisScreenIndex == 31) Text += "    EBCO:" + String(ecu.GetOutputEBC());
    if (ThisScreenIndex == 32) Text += "    2MAP:" + String(ecu.GetOutput2ndMap());
    if (ThisScreenIndex == 33) Text += "   FCTRLO:" + String(ecu.GetOutputFanCtrl());
    if (ThisScreenIndex == 34) Text += "    BSTO:" + String(ecu.GetOutputBST());
    if (ThisScreenIndex == 35) Text += "  IACV:" + String(ecu.GetIACVDuty());

    //POSSIBLE USELESS DATALOGS
    //if (ThisScreenIndex == 00) Text += "   BARO:" + String(ecu.GetBaro());
    //if (ThisScreenIndex == 00) Text += "    PSTF:" + String(ecu.GetPostFuel());
    //if (ThisScreenIndex == 00) Text += "   SCC.C:" + String(ecu.GetSCCChecker());
    //if (ThisScreenIndex == 00) Text += "   ATSF1:" + String(ecu.GetATShift1());
    //if (ThisScreenIndex == 00) Text += "   ATSF2:" + String(ecu.GetATShift2());
    //if (ThisScreenIndex == 00) Text += "    IGNT:" + String(ecu.GetIgnTable());
    //if (ThisScreenIndex == 00) Text += "    PARK:" + String(ecu.GetParkN());
    //if (ThisScreenIndex == 00) Text += "    BKSW:" + String(ecu.GetBKSW());
    //if (ThisScreenIndex == 00) Text += "    ACC:" + String(ecu.GetACC());
    //if (ThisScreenIndex == 00) Text += "    STRT:" + String(ecu.GetStart());
    //if (ThisScreenIndex == 00) Text += "    SCC:" + String(ecu.GetSCC());
    //if (ThisScreenIndex == 00) Text += "   VTSFB:" + String(ecu.GetVTSFeedBack());
    //if (ThisScreenIndex == 00) Text += "    PSP:" + String(ecu.GetPSP());
    //if (ThisScreenIndex == 00) Text += "    FPMP:" + String(ecu.GetFuelPump());
    //if (ThisScreenIndex == 00) Text += "    IAB:" + String(ecu.GetIAB());
    //if (ThisScreenIndex == 00) Text += "   PURGE:" + String(ecu.GetPurge());
    //if (ThisScreenIndex == 00) Text += "    AC:" + String(ecu.GetAC());
    //if (ThisScreenIndex == 00) Text += "    O2H:" + String(ecu.GetO2Heater());
    //if (ThisScreenIndex == 00) Text += "    VTS:" + String(ecu.GetVTS());
    //if (ThisScreenIndex == 00) Text += "  ELDV:" + String(ecu.GetELDVolt()) + "v";
    //if (ThisScreenIndex == 00) Text += "   ECTFC:" + String(ecu.GetECTFC());
    //if (ThisScreenIndex == 00) Text += "    O2S:" + String(ecu.GetO2Short());
    //if (ThisScreenIndex == 00) Text += "    O2L:" + String(ecu.GetO2Long());
    //if (ThisScreenIndex == 00) Text += "   IATFC:" + String(ecu.GetIATFC());
    //if (ThisScreenIndex == 00) Text += "   VEFC:" + String(ecu.GetVEFC());
    //if (ThisScreenIndex == 00) Text += "  IATIC:" + String(ecu.GetIATIC());
    //if (ThisScreenIndex == 00) Text += "  ECTIC:" + String(ecu.GetECTIC());
    //if (ThisScreenIndex == 00) Text += "  GEARIC:" + String(ecu.GetGEARIC());
    //if (ThisScreenIndex == 00) Text += "   EBCHi:" + String(ecu.GetInputEBCHi());
    //if (ThisScreenIndex == 00) Text += "   GPO1I:" + String(ecu.GetInputGPO1());
    //if (ThisScreenIndex == 00) Text += "   GPO2I:" + String(ecu.GetInputGPO2());
    //if (ThisScreenIndex == 00) Text += "   GPO3I:" + String(ecu.GetInputGPO3());
    //if (ThisScreenIndex == 00) Text += " EBCBDTY:" + String(ecu.GetEBCBaseDuty());
    //if (ThisScreenIndex == 00) Text += "  EBCDTY:" + String(ecu.GetEBCDuty());
    //if (ThisScreenIndex == 00) Text += "   GPO1O:" + String(ecu.GetOutputGPO1());
    //if (ThisScreenIndex == 00) Text += "   GPO2O:" + String(ecu.GetOutputGPO2());
    //if (ThisScreenIndex == 00) Text += "   GPO3O:" + String(ecu.GetOutputGPO3());
    //if (ThisScreenIndex == 00) Text += "   BSTS2:" + String(ecu.GetOutputBSTStage2());
    //if (ThisScreenIndex == 00) Text += "   BSTS3:" + String(ecu.GetOutputBSTStage3());
    //if (ThisScreenIndex == 00) Text += "   BSTS4:" + String(ecu.GetOutputBSTStage4());
    //if (ThisScreenIndex == 00) Text += "    LEAN:" + String(ecu.GetLeanProtect());

    //Remove Unwanted Whitespace on 20x04
    if (Is20x04Screen) Text.trim();
    
    //Reset Invalid Char Over Text Lenght
    int ResetLenght = ScreenMaxPx - Text.length();
    if (Is20x04Screen) ResetLenght = 10 - Text.length(); //Reset Only Half of the screen (10px) since the other 10px server for another value on 20x04
    for (int i2=0; i2<ResetLenght; i2++) Text += " ";

    //Offset the text (used on 20x04 Display, to show 2x value wide rather than only one)
    int Offset = 0;
    int Lines = i;
    if (Is20x04Screen) {
      if (i == 1)  {
        Lines = 0;
        Offset = 10;
      }
      if (i == 2) Lines = 1;
      if (i == 3) {
        Lines = 1;
        Offset = 10;
      }
      if (i == 4) Lines = 2;
      if (i == 5) {
        Lines = 2;
        Offset = 10;
      }
      if (i == 6) Lines = 3;
      if (i == 7) {
        Lines = 3;
        Offset = 10;
      }
    }
    
    //Print Text
    lcd.setCursor(Offset, Lines);
    lcd.print(Text);
  }
}

//#####################################################
//These Calls occurs on anys voltage change Applyed to buttons inputs
void GetButtonTopState() {
  if (digitalRead(TopButton) == LOW) {
    if (!EcuConnected)
      EcuConnected = true;
    else {
      unsigned long interrupt_time = millis();  
      if (buttonTop == OFF && (interrupt_time - last_interrupt_time > debouncing)) {
        buttonTop=ON;
        last_interrupt_time = interrupt_time;
      }
    }
  }
}

void GetButtonBottomState() {
  if (digitalRead(BottomButton) == LOW) {
    if (!EcuConnected)
      EcuConnected = true;
    else {
      unsigned long interrupt_time = millis();  
      if (buttonBottom == OFF && (interrupt_time - last_interrupt_time > debouncing)) {
        buttonBottom=ON;
        last_interrupt_time = interrupt_time;
      }
    }
  }
}


