// v konstrukci> pro vstupni piny, pokud se pouzijou 3, pouzit 2,3,X, protoze na 23 jsou
//preruseni. A4, A5 jsou na twi viz nize. 
String vers="2019042601";


//TWI DEFINE - TWI PRIPOJENI DISPLAY
// twiPinTX=A4;
// twiPinCLK=A5;

//for setMillis() function
extern volatile unsigned long timer0_millis;

//defreezing in milliseconds
unsigned long defreezingPeriod=172800000; //178800000=48h
unsigned long   defreezingTime=9000000; //9000000=2.5h

//EEProm Write position
byte eepos1=10;  //vzdy sude
byte eepos2=(eepos1+2);


//LED diag pin
byte LED=13;

//relay switch
byte relaySW=9;

//USER INPUT PINS tlacitka
byte SW1=2;
byte SW2=3;

//FEEDBACK CALIBRATION INPUT
// fedbackPin=A0

//DallasTemperature
byte DallasPin=5;

volatile unsigned long lasttime=0, presstime=0, switchtime=0;  //zasobniky pro millis()
volatile bool runon=0;

unsigned int time_hysterezis=5;   //time hystereze v minutach (moznost preklopeni)

unsigned long timer01;

#include <Wire.h>
#include <EEPROM.h>
#include "U8glib.h"
#include "OneWire.h"
#include "DallasTemperature.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send AC

OneWire DallasOW(DallasPin);  //create instance Onewire
DallasTemperature DallasDS(&DallasOW); //create instance Dallas (use OneWire)

void setup() {
 


  pinMode(A0, INPUT);
  //pinMode(LED, OUTPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(relaySW, OUTPUT);

/*
attachInterrupt(digitalPinToInterrupt(LLp),LLup,RISING);
*/ 

//Wire.begin();        // join i2c bus (address optional for master)
DallasDS.begin(); //start comm with Dallas sensor

timer01=millis();

}  //end of setup part

void overflowCheck(){  //osetreni preteceni
   if(millis()>3456000000){
    setMillis(0);
    timer01=millis();  
   }

}

void defreezing(){
  timer01=millis();
  while((millis()-timer01)<defreezingTime){
    u8g.begin();
    cooling(0);
    //runon=0;
    wdl2PlusOn(String(nactiTeplotu()),String("DEFREEZ") + String(" ") + String((millis()-switchtime)/1000), runon);
    delay(1000);
  }
  timer01=millis();
}

//DISPLAY
//kod z https://www.arduinotech.cz/inpage/wifi-teplomer-385/
void displ(String st) 
{
  u8g.setFont(u8g_font_7x14);
  u8g.setPrintPos(0, 20); 
  // call procedure from base class, http://arduino.cc/en/Serial/Print
  u8g.print(st);
  //u8g.setFont(u8g_font_fub20);
  //u8g.setPrintPos(10,50); 
  //u8g.print("Ahoj!!!");
  //u8g.drawLine(0,60,128,60);

}

void dispL(String st) //Large font
{
  u8g.setFont(u8g_font_osr21);
  u8g.setPrintPos(0, 30); 
  u8g.print(st);
}

//pracovni funkce 2 stringy lednice
void dispL2(String st1, String st2){
  u8g.setFont(u8g_font_osr21);
  u8g.setPrintPos(0, 30); 
  u8g.print(st1);
  u8g.setFont(u8g_font_7x14);
  u8g.setPrintPos(0, 60); 
  u8g.print(st2);
}

//pracovni funkce 3 stringy
void displ3(String st1, String st2, String st3){
  u8g.setFont(u8g_font_7x14);
  u8g.setPrintPos(0, 20); 
  u8g.print(st1);
  u8g.setPrintPos(0, 40); 
  u8g.print(st2);
  u8g.setPrintPos(0, 60); 
  u8g.print(st3);
}

//uzivatelska funkce 1 string
void wd(String str){
  u8g.firstPage();  
  do {
    displ(str);
  } while( u8g.nextPage() );
}

//uzivatelska funkce 1 string Large
void wdl(String str){
  u8g.firstPage();  
  do {
    dispL(str);
  } while( u8g.nextPage() );
}

//uzivatelska funkce 2 string Large
void wdl2(String str1, String str2){
  u8g.firstPage();  
  do {
    dispL2(str1, str2);
  } while( u8g.nextPage() );
}

//uzivatelska funkce 3 stringy
void wd3(String str1,String str2, String str3 ){
  u8g.firstPage();  
  do {
    displ3(str1, str2, str3);
  } while( u8g.nextPage() );
}

//uzivatelska funkce 2 string Large plus ON
void wdl2PlusOn(String str1, String str2, bool onsw){
  u8g.firstPage();  
  do {
    dispL2On(str1, str2, onsw);
  } while( u8g.nextPage() );
}

//pracovni funkce 2 stringy lednice plus ON
void dispL2On(String st1, String st2, bool ons){
  u8g.setFont(u8g_font_osr21);
  u8g.setPrintPos(0, 30); 
  u8g.print(st1);
  u8g.setFont(u8g_font_7x14);
  u8g.setPrintPos(0, 60); 
  u8g.print(st2);
  if(ons){
                  u8g.drawDisc(120,10,5);
         }
}

//KONEC DISPLAY SEKCE







void BlinkInternalLed(int pocet){
  for(int i=0; i<pocet; i++){
  digitalWrite(LED, HIGH);
  delay(300);
  digitalWrite(LED, LOW);
  delay(300);
  }
}

void panic(){ //panic
  wd("PANIC");
  for(;;){
  digitalWrite(LED, HIGH);
  delay(30);
  digitalWrite(LED, LOW);
  delay(30);
  }
}

void ledon(){
  digitalWrite(LED, HIGH);
}

void ledoff(){
  digitalWrite(LED, LOW);
}

void BlinkInternalLedShort(int pocet){
  for(int i=0; i<pocet; i++){
  digitalWrite(LED, HIGH);
  delay(20);
  digitalWrite(LED, LOW);
  delay(20);
  }
}


//proste reaguje na tlacitka, u leveho vraci -0.25, u praveho +0.25
float increase(){ //return 0 (nothing or both pressed) or 1 (SW2 pressed) or -1 (SW1 pressed)
   if((!(digitalRead(SW2)))&&(!(digitalRead(SW1)))) {lasttime=millis(); return 0;} //error both pressed
    if(!(digitalRead(SW2))) {
      lasttime=millis();
      return(0.25);
      }
   if(!(digitalRead(SW1))) {
    lasttime=millis();
    return(-0.25);
    }
  presstime=millis();  
  return 0;
}
  

bool writeEEw(byte pos, word val){ //pos must be even-numbered (need 2 bytes)
  if(pos%2!=0) return 0;
  EEPROM.update(pos,(byte)val); //need #include <EEPROM.h>
  EEPROM.update(pos+1,(byte)(val>>8));
  if(val!=(EEPROM.read(pos) + ((EEPROM.read(pos+1)) << 8))) return 0;
  return 1;
}

word readEEw(byte pos){
  word eew;
  eew=((EEPROM.read(pos)) + ((EEPROM.read(pos+1)) << 8));
  if(eew<4096) return eew;
}

//nacte ze dvou pametovych mist hodnotu posunutou o 30
//rozsah je -30 az +30
float readEE(){
  word celocis, setiny;
  float vysledek;
  celocis=readEEw(eepos1);
  setiny=readEEw(eepos2);
  vysledek=((float)(celocis+((float)setiny/100))-30);
  if(vysledek<(-30) || vysledek>30) vysledek=0;
  return vysledek;
}

//rozsah zapisu je -30.00 stupne az +30.00 stupne
//celociselna cast se zapisuje zvlast a setiny tez zvlast
//vraci 1 = v poradku nebo 0=chyba
bool storeEE(float stemp){
  stemp=stemp+30; //posun
  if(stemp<0) stemp=0;
  word celocis, setiny;
  bool ret;
  celocis=(word)((int)stemp);
  setiny=(word)((int)((stemp-celocis)*100));
  ret=writeEEw(eepos1,celocis);
  if(ret) ret=writeEEw(eepos2,setiny);
  return ret;
}

float nactiTeplotu(){
  DallasDS.requestTemperatures();
  return(DallasDS.getTempCByIndex(0));
}

void selectBoostOrStop(){
}

float settingsTempMode(float lastval){
  float newval=lastval;
  bool zapis;
  lasttime=millis();
  while((millis()-lasttime<5000))
      {
      newval=newval+increase();
      if(newval>30) newval=30;
      if(newval<-30) newval=-30;
      wdl2("", String(newval));
      }
  zapis=storeEE(newval);    
  wd("zapis EE "+ String(zapis));
  delay(3000);
  return newval;
}

bool cooling(bool yesno)
  {
  if((millis()-switchtime)<(time_hysterezis*60000)) return(0);
  else{
    if ((yesno)&& (!runon))
          {
            digitalWrite(relaySW, HIGH);
            switchtime=millis();
            u8g.begin();
            runon=1;
            return(1);
          }
     if((!yesno) && (runon))
          {
            digitalWrite(relaySW, LOW);
            switchtime=millis();
            u8g.begin();
            runon=0;
            return(1);
           }
    }
  }
  

float setTemp=0, realTemp=0;

void loop() {
  wd(vers);
  delay(1000);
  setTemp=readEE();
  

/*
if(!storeEE(LL,ML,HL,chan)) panic();
wd("EEPROM OK");
digitalWrite(LED,HIGH);
delay(2000);
digitalWrite(LED,LOW);
}     //END SETTINGS
*/

wd("MAIN LOOP");
for(;;){ //USER MAIN LOOP PUT HERE
  overflowCheck();
  realTemp=nactiTeplotu();
  if((millis()-timer01)>defreezingPeriod) defreezing();

  if((!(digitalRead(SW2)))&&(!(digitalRead(SW1)))) {   //both pressed 
             defreezing();
             }            
  if((!(digitalRead(SW1))) || !(digitalRead(SW2))){ //if any switch pressed START SETTINGS 
              setTemp=settingsTempMode(setTemp);
              }

delay(100);

if(realTemp<setTemp) cooling(0);
if(realTemp>setTemp) cooling(1);

//zobrazeni na display
//wdl2(String(realTemp)+ "    " + String(runon), String(setTemp)+ "  " + String((millis()-switchtime)/1000));
wdl2PlusOn(String(realTemp), String(setTemp)+ "  " + String((millis()-switchtime)/1000), runon);

} //end of user main loop



} //end of main loop


void setMillis(unsigned long new_millis){
  uint8_t oldSREG = SREG;
  cli();
  timer0_millis = new_millis;
  SREG = oldSREG;
}

