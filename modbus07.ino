
//REMEMBER! uncomment #define USE_HARDWARESERIAL 
//in SDM_Config_User.h file if you want to use hardware uart
#include <ErriezTTP229.h>

//128
//250



#include <SDM.h>                                                                //import SDM library



#include <SoftwareSerial.h>    
#include <GxEPD.h>
#include <GxGDE0213B72B/GxGDE0213B72B.h>      // 2.13" b/w

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
//custom font https://rop.nl/truetype2gfx/
#include <Fonts/FreeMonoBold7pt7b.h>
#include <Fonts/Cascadia6pt7b.h>
#include <Fonts/RubikMonoOne_Regular6pt7b.h> //hanya huruf kapital, tapi tebal
#include <Fonts/TheSansMonoCondensed_Black6pt7b.h>
#include <Fonts/TheSansMonoCondensed_Black9pt7b.h>

#include <Fonts/F2FOCRBczyk_LT_Std_Regular_Bold_166079pt7b.h>
#include <Fonts/F2FOCRBczyk_LT_Std_Regular_Bold_166076pt7b.h>
#include <Fonts/OCRB6pt7b.h>
#include <Fonts/OCRB7pt7b.h>
#include <Fonts/OCRB9pt7b.h>
#include <Fonts/TheSansMonoCondensed_Extra_Bold7pt7b.h>
#include <Fonts/TheSansMonoCondensed_Extra_Bold9pt7b.h>


#include GxEPD_BitmapExamples
#include "qrcode.h"

#define TTP229_SDO_PIN     32  // Keep GPIO0 low during programming
#define TTP229_SCL_PIN     33

#define PULSEON_PIN     13  // Keep GPIO0 low during programming
#define PULSEOFF_PIN     14


#define maxdigit  6


#define widthx  128
#define heightx  250

#define infoy  120
#define infoh  20
#define digity  145
#define digith  25

#define statusy  175
#define statush  75


//import SoftwareSerial library
SoftwareSerial swSerSDM;                                                        //config SoftwareSerial
//SDM sdm(swSerSDM, 9600, NOT_A_PIN, SWSERIAL_8N1, SDM_RX_PIN, SDM_TX_PIN);       //config SDM
SDM sdm(swSerSDM, 9600, NOT_A_PIN, SWSERIAL_8N1, 34, 12);       //config SDM

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4


ErriezTTP229 ttp229;
uint8_t buttonMap[17] = {0,0x31,0x32,0x33,0x41,
                           0x34,0x35,0x36,0x42,
                           0x37,0x38,0x39,0x43,
                           0x58,0x30,0x59,0x44};

uint8_t bufdigit[maxdigit];                           
uint8_t flowstatus =0;
uint8_t digitke =0;
char pressedkey = 0;
//------------------------------------------------------------------------------------------------------------------------------

unsigned long currentMillis ;
//selalu berpasangan
unsigned int  interval01=1000; // the time we need to wait
unsigned long prevmill01=0; // millis() returns an unsigned long.
unsigned int  interval02=1000; // the time we need to wait
unsigned long prevmill02=0; // millis() returns an unsigned long.
unsigned int  interval03=1000; // the time we need to wait
unsigned long prevmill03=0; // millis() returns an unsigned long.

unsigned int  interval04=10000; // the time we need to wait
unsigned long prevmill04=0; // millis() returns an unsigned long.

unsigned int  interval05=10000; // the time we need to wait
unsigned long prevmill05=0; // millis() returns an unsigned long.


unsigned int maxrandom = 500;
//------------------------------------------------------------------------------------------------------------------------------ 
unsigned int locrandom = 240;
float expkwh =0;
float impkwh =0;
float standawalkwh =0;
int temporderkwh = 0;
int orderkwh = 0;
unsigned long randomQR=0;


#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
ICACHE_RAM_ATTR
#endif
void keyChange()
{
    // A key press changed
    ttp229.keyChange = true;
}



char* string2char(String ipString){ // make it to return pointer not a single char
  char* opChar = new char[ipString.length() + 1]; // local array should not be returned as it will be destroyed outside of the scope of this function. So create it with new operator.
  memset(opChar, 0, ipString.length() + 1);

  for (int i = 0; i < ipString.length(); i++)
    opChar[i] = ipString.charAt(i);
  return opChar; //Add this return statement.
}

String uint64ToString(uint64_t input) {
  String result = "";
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

String convertFloatToString(float n, int ndec)
{ // begin function

  char temp[20];
  String tempAsString;
    
    // perform conversion
    dtostrf(n,1,ndec,temp);
    
    // create string object
  tempAsString = String(temp);
  
  return tempAsString;
  
} 


void qrCard(){
  uint32_t dt = millis();
  byte box_x = 1;
  byte box_y = 1;
  byte box_s = 2;
  byte init_x = box_x;
  randomQR = random(100000, 999999);
  
  //display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

  Serial.print("QR Code Gen ! ");  
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(10)];
    //qrcode_initText(&qrcode, qrcodeData, 10, 0, string2char("OleOle"));
    qrcode_initText(&qrcode, qrcodeData, 10, 0, string2char(String(randomQR)));

  
  // Delta time
  dt = millis() - dt;
  Serial.print("QR Code Generation Time: ");
  Serial.print(dt);
  Serial.print("\n");

  // Top quiet zone
  Serial.print("\n\n\n\n");
  
  for (uint8_t y = 0; y < qrcode.size; y++) {
    // Left quiet zone
    Serial.print("        ");

    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++) {

      if(qrcode_getModule(&qrcode, x, y)){
        Serial.print("\u2588\u2588");
        display.fillRect(box_x, box_y, box_s, box_s, GxEPD_BLACK);
      } else {
        Serial.print("  ");
        display.fillRect(box_x, box_y, box_s, box_s, GxEPD_WHITE);
      }
      box_x = box_x + box_s;
    }
    Serial.print("\n");
    //display.updateWindow(0, box_y, GxEPD_WIDTH, box_s, false);
    box_y = box_y + box_s;
    box_x = init_x;
    
  }

 display.updateWindow(0, 0, GxEPD_WIDTH, box_y, false);


  delay(200);

  
  display.setTextColor(GxEPD_BLACK);

  display.setFont(NULL);
  display.setCursor(120,85);
  display.print("QR OK !");

  Serial.print("\n\n\n\n");  
}

void hapuslayar(){
  display.fillRect(0, 0,  GxEPD_HEIGHT,GxEPD_WIDTH, GxEPD_WHITE);
  display.update();
}

void clearinfo(){
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_BLACK);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
   
}

void displayinfo1(){
  //display.fillRect(0, 0, infoy , widthx, GxEPD_WHITE);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_BLACK);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
        
          
          display.setTextColor(GxEPD_BLACK);
          display.setFont(&TheSansMonoCondensed_Extra_Bold7pt7b);
          display.setCursor(0,infoy+10);
          display.print("ORDER"); 
          display.setCursor(0,infoy+24);
          display.print("BERAPA kWh ?"); 
  
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
          
  //display.update();
}

void displayinfo2(){
  //display.fillRect(0, 0, infoy , widthx, GxEPD_WHITE);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_BLACK);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
  display.fillRect(0, infoy,  widthx,infoh + digith, GxEPD_WHITE);
        
          
          display.setTextColor(GxEPD_BLACK);
          display.setFont(&TheSansMonoCondensed_Extra_Bold7pt7b);
          display.setCursor(0,infoy+10);
          display.print("ORDER:"+String(temporderkwh)+"kwh"); 
          display.setCursor(0,infoy+24);
          display.print("SCAN,INPUT KODE:"); 
  
  display.updateWindow(0, infoy,  widthx,infoh + digith, false);
          
  //display.update();
}


void gambardigit(){
  //display.fillRect(0, 0, infoy , widthx, GxEPD_WHITE);
  /*
  display.fillRect(0, digity,  widthx,digith, GxEPD_BLACK);
  display.updateWindow(0, digity,  widthx,digith, false);
  display.fillRect(0, digity,  widthx,digith, GxEPD_WHITE);
  display.updateWindow(0, digity,  widthx,digith, false);
  display.fillRect(0, digity,  widthx,digith, GxEPD_WHITE);
  display.updateWindow(0, digity,  widthx,digith, false);
  display.fillRect(0, digity,  widthx,digith, GxEPD_WHITE);
  */      
          
          display.setTextColor(GxEPD_BLACK);
          display.setFont(&TheSansMonoCondensed_Extra_Bold9pt7b);
          display.setCursor(0+(digitke*10),digity+15);
          display.print((char)pressedkey); 
          
  display.updateWindow(0, digity,  widthx, digith, false);
          
  //display.update();
}

void buftoint(){
 uint8_t pangkat10 =0; 
 temporderkwh =0;
          for (uint8_t i = maxdigit; i > 0; i--) {
            if (bufdigit[i-1] > 0) {
              temporderkwh = temporderkwh + (bufdigit[i-1]-48)*pow(10, pangkat10);
              pangkat10 = pangkat10+1;
            }
             //Serial.println(bufdigit[i-1]);
             //Serial.println(orderkwh);
          }
}

unsigned long buftoint02(){
 uint8_t pangkat10 =0; 
 unsigned long tempint =0;
          for (uint8_t i = maxdigit; i > 0; i--) {
            if (bufdigit[i-1] > 0) {
              tempint = tempint + (bufdigit[i-1]-48)*pow(10, pangkat10);
              pangkat10 = pangkat10+1;
            }
             //Serial.println(bufdigit[i-1]);
             //Serial.println(orderkwh);
          }
 return tempint;         
}

float getsisakwh(){
  float tempfloatku = ((float)orderkwh/1000)-( impkwh - standawalkwh);
                    Serial.println("sisakwh");                    
                    Serial.println(convertFloatToString(((float)orderkwh/1000), 3)); 
                    Serial.println(convertFloatToString(impkwh, 3)); 
                    Serial.println(convertFloatToString(standawalkwh, 3)); 
                    Serial.println(convertFloatToString(tempfloatku, 3)); 
  
  return tempfloatku;
}

void displaystatus1(){
  int offset =0;
  //display.fillRect(0, 0, infoy , widthx, GxEPD_WHITE);
  display.fillRect(0, statusy+offset,  widthx, statush, GxEPD_BLACK);
  display.updateWindow(0, statusy+offset,  widthx, statush, false);
  display.fillRect(0, statusy+offset,  widthx, statush, GxEPD_WHITE);
  display.updateWindow(0, statusy+offset,  widthx, statush, false);
  display.fillRect(0, statusy+offset,  widthx, statush, GxEPD_WHITE);
  display.updateWindow(0, statusy+offset,  widthx, statush, false);
  display.fillRect(0,statusy+offset,  widthx, statush, GxEPD_WHITE);
        
          
          display.setTextColor(GxEPD_BLACK);
          display.setFont(&TheSansMonoCondensed_Extra_Bold7pt7b);
          display.setCursor(0,statusy+offset+10);
          display.print("order(sblumnya)"); 
          display.setCursor(0,statusy+offset+22);
          //buftoint();
          display.print(orderkwh); 
          display.setCursor(0,statusy+offset+34);
          display.print("sisa kwh"); 
          display.setCursor(0,statusy+offset+46);
          display.print(convertFloatToString(getsisakwh(), 3)); 
          display.setCursor(0,statusy+offset+58);
          display.print("angka kwh meter "); 
          display.setCursor(0,statusy+offset+70);
          display.print(convertFloatToString(impkwh, 3)); 

  display.updateWindow(0, statusy+offset,  widthx, statush, false);
          
  //display.update();
}

void displayrandom(){
            //display.fillRect(0, 120,  GxEPD_HEIGHT,240-120, GxEPD_BLACK);
          //display.updateWindow(0, 120, GxEPD_WIDTH, 240-120, false);
          display.fillRect(0, 120,  GxEPD_HEIGHT,240-120, GxEPD_WHITE);
          display.updateWindow(0, 120, GxEPD_WIDTH, 240-120, false);
        
          
          locrandom = random(120, 240);    
          display.setTextColor(GxEPD_BLACK);
        
          /*
          if ((locrandom % 3)==0) {
            display.setFont(&FreeMonoBold7pt7b);  
          }  
          if ((locrandom % 3)==1) {
            display.setFont(&Cascadia6pt7b);  
          }
          if ((locrandom % 3)==2) {
            display.setFont(&TheSansMonoCondensed_Black6pt7b);  
          }
        
          */
        
          display.setFont(&TheSansMonoCondensed_Extra_Bold7pt7b);
        
        
          display.setCursor(0,locrandom);
          display.print(locrandom); 
          display.print(" - ");
          display.print(String(random(100, 999)));
          display.print("(ABC)");
          display.updateWindow(0, 120, GxEPD_WIDTH, 240-120, false);
}



void setup() {
  Serial.begin(115200);    
  pinMode(PULSEON_PIN, OUTPUT);
  pinMode(PULSEOFF_PIN, OUTPUT);  

  delay(50);
  digitalWrite(PULSEON_PIN, LOW);
  digitalWrite(PULSEOFF_PIN, HIGH);                  
  delay(50);
  digitalWrite(PULSEOFF_PIN, LOW);
  
  //initialize serial
  memset(bufdigit, 0, maxdigit);
  sdm.begin();                                                                  //initialize SDM communication



  Serial.println("Keypad setup...");
    ttp229.begin(TTP229_SCL_PIN, TTP229_SDO_PIN);

    // Initialize interrupt pin on SD0
    attachInterrupt(digitalPinToInterrupt(TTP229_SDO_PIN), keyChange, FALLING);

  Serial.println("Display setup...");



  display.init(115200); // enable diagnostic output on Serial
  display.setRotation(0);
  display.update();
  Serial.println("Done");
  Serial.println(GxEPD_WIDTH);
  Serial.println(GxEPD_HEIGHT);
  hapuslayar();
  /*
  Serial.println("Random Gen ! "); 
  display.setTextColor(GxEPD_BLACK);
  display.setFont(NULL);
  //display.setCursor(120,95);
  //display.setCursor(0,120);
  display.setCursor(0,240);
  display.print(millis()); 
  display.print(" - ");
  display.print(String(random(100000, 999999)));
  display.print("(max=271)");
  display.update();
  */
  
  qrCard();

  //potongstring ( 120,105,10,21,String(ssid));
 

//setup millistimer    ;
   currentMillis = millis(); // grab current time
  

  
}


void loop() {

   currentMillis = millis(); // grab current time
   




    if (ttp229.keyChange) {
        pressedkey = (char)buttonMap[ttp229.GetKey16()]; 
        Serial.println(pressedkey);
        switch (pressedkey) {
          
                case 'A': //inisiasi masukkan KWH         
                  flowstatus = 1;  
                  hapuslayar();
                  displayinfo1();   
                  digitke = 0;
                  memset(bufdigit, 0, maxdigit);
                  buftoint();                    
                  break;
                case 'D': //batalkan         
                  flowstatus = 0;     
                  break;

                case 'B': //batalkan  
                    if (getsisakwh()>0){
                      digitalWrite(PULSEON_PIN, HIGH);                  
                      delay(50);
                      digitalWrite(PULSEON_PIN, LOW);                                               
                    }
                  break;

                case 'C': //batalkan         
                    digitalWrite(PULSEOFF_PIN, HIGH);                  
                    delay(50);
                    digitalWrite(PULSEOFF_PIN, LOW);                  
                  break;




                case '0' ... '9': //batalkan         
                  if (( flowstatus==1)||( flowstatus==2)) { //input isi kwh
                     if (digitke <maxdigit)  {
                       bufdigit[digitke] = pressedkey; 
                       digitke = digitke +1;                      
                     }   
                     gambardigit();                     
                  }
                  break;

                case 'X': //batalkan         
                  if ( flowstatus==1){ //bersihkan input isi kwh
                    displayinfo1();   
                    digitke = 0;
                    memset(bufdigit, 0, maxdigit);
                    //buftoint();                    
                  }
                  if ( flowstatus==2){ //bersihkan input isi kodeqr
                    displayinfo2();   
                    digitke = 0;
                    memset(bufdigit, 0, maxdigit);
                    //buftoint();                    
                  }
                                    
                  break;

                case 'Y': //batalkan         
                  if ( flowstatus==2){ //status
                    flowstatus =0;
                    clearinfo();
                    Serial.println("QRcode");                    
                    Serial.println(randomQR); 
                    unsigned long temprandomQR = buftoint02();
                    Serial.println(temprandomQR);                                        
                    if (randomQR==temprandomQR){
                       orderkwh = temporderkwh ;
                       standawalkwh = impkwh;
                       
                       displaystatus1();
                       //buftoint();
                       flowstatus =0;
                    }
                    digitke = 0;
                    memset(bufdigit, 0, maxdigit);                    
                  }
                  if ( flowstatus==1){ //setuju order, lanjut ke isi kodeQR
                    //displaystatus1();
                    buftoint();
                    flowstatus =2;
                    displayinfo2();
                    qrCard();
                    digitke = 0;
                    memset(bufdigit, 0, maxdigit);                    
                  }

                  break;

                //default:

                  /* 
                  // turn all the LEDs off:
                  for (int thisPin = 2; thisPin < 7; thisPin++) {
                    digitalWrite(thisPin, LOW);
                  */
                   
                  } 



        
        ttp229.keyChange = false;
    }


 if ((unsigned long)(currentMillis - prevmill02) >= (unsigned long)interval02) {
      
        Serial.print("Import: ");
        impkwh = sdm.readVal(SDM_IMPORT_ACTIVE_ENERGY);
        Serial.print(impkwh, 3);                                  //display frequency
        Serial.println("kWh");   
      
   prevmill02 = currentMillis + random(0,maxrandom);
 }

 if ((unsigned long)(currentMillis - prevmill03) >= (unsigned long)interval03) {
      
        Serial.print("Export: ");
        expkwh = sdm.readVal(SDM_EXPORT_ACTIVE_ENERGY);                                  //display frequency
        Serial.print(expkwh, 3);  
        Serial.println("kWh");   
   prevmill03 = currentMillis + random(0,maxrandom);
 }



 if (flowstatus==0) {
         // check if "interval" time has passed (1000 milliseconds)
         if ((unsigned long)(currentMillis - prevmill01) >= (unsigned long)interval01) {
                /*
                char bufout[10];
                sprintf(bufout, "%c[1;0H", 27);
                Serial.print(bufout);
                */
                
                Serial.print("Voltage:   ");
                Serial.print(sdm.readVal(SDM_PHASE_1_VOLTAGE), 2);                            //display voltage
                Serial.println("V");
              
                Serial.print("Current:   ");
                Serial.print(sdm.readVal(SDM_PHASE_1_CURRENT), 2);                            //display current  
                Serial.println("A");
              
                Serial.print("Power:     ");
                Serial.print(sdm.readVal(SDM_PHASE_1_POWER), 2);                              //display power
                Serial.println("W");
              
                Serial.print("Frequency: ");
                Serial.print(sdm.readVal(SDM_FREQUENCY), 2);                                  //display frequency
                Serial.println("Hz");   
        
           prevmill01 = currentMillis + random(0,maxrandom);
         }
        
        
        
        
         if ((unsigned long)(currentMillis - prevmill05) >= (unsigned long)interval05) {
              
              //qrCard();
           prevmill05 = currentMillis + random(0,maxrandom);
         }

  }//status = 0;
  

         if ((unsigned long)(currentMillis - prevmill04) >= (unsigned long)interval04) {
              //displayrandom();

                    if (getsisakwh()<=0){
                      digitalWrite(PULSEOFF_PIN, HIGH);                  
                      delay(50);
                      digitalWrite(PULSEOFF_PIN, LOW);                                               
                    }
              
              displaystatus1();
              Serial.print("flowstatus:");
              Serial.println(flowstatus);


              
           prevmill04 = currentMillis + random(0,maxrandom);
         }
  
  
  
  //wait a while before next loop
} //LLOoOOOoOOOoOOOPPPPPP

/*
void helloWorld1()
{
  //display.setFullWindow();
  display.setPartialWindow(0, 120, display.width(), display.height());
  display.setRotation(0);
  //display.setFont(&FreeMonoBold9pt7b);
  display.setFont(NULL);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    //display.setCursor(120,95);
    display.setCursor(0,120);
    display.print(millis()); 
    display.print(" - ");
    display.print(String(random(100000, 999999)));
    display.print("(max=271)");
    
  }
  while (display.nextPage());
}

*/


