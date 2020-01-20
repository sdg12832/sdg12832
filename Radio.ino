#include "SSD1306AsciiWire.h"
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

uint32_t r, TuneTim, StatusTim;

int But1 = 2;//tune
int But2 = 3;//volume

uint32_t But1Up, But1Dn, Cmd1;
uint32_t But2Up, But2Dn, Cmd2;

SSD1306AsciiWire oled;

char* Service    = "               ";//reserved memory
char* Empty      = " ************* ";//for station without RDS

int v;     //volume
uint16_t f;//frequency

RDA5807M radio;   /// Create an instance of a RDA5807 chip radio
RDSParser rds;    /// get a RDS parser
RADIO_INFO ri;    /// radio info
uint16_t g_block1;
// - - - - - - - - - - - - - - - - - - - - - - - - - -
// to catch the block1 value (used for sender identification)
void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) 
{
g_block1 = block1;
rds.processData(block1, block2, block3, block4);
}

void GetServiceName(char *name)//
{
bool found = false; uint8_t n;
for (n = 0; n < 8; n++) if (name[n] != ' ') found = true;
  if (found) {
bool fnd = false;
if ((name[0] < 'A')||(name[1] < 'A')||(name[2] < 'A')||(name[3] < 'A')||
    (name[4] < 'A')||(name[5] < 'A')) fnd = true;
if (fnd == false) {
  Service[ 0] = char(0x20); Service[ 1] = char(0x20); Service[ 2] = char(0x20);
  Service[ 3] = char(0x20); Service[ 4] = char(0x20); Service[ 5] = char(0x20);
  Service[ 6] = char(0x20); Service[ 7] = char(0x20); Service[ 8] = char(0x20);
  Service[ 9] = char(0x20); Service[10] = char(0x20); Service[11] = char(0x20);
  Service[12] = char(0x20); Service[13] = char(0x20); Service[14] = char(0x20);
  strcpy(Service+4,name);
                  }
             }
}

void DisplayTune()
{
oled.setCursor(0,2);
oled.set2X();
oled.setFont(lcdnums12x16);
char s[12];
radio.formatFrequency(s, sizeof(s));
s[5] = (char)0;
oled.print(s);
oled.set1X();
oled.setFont(ZevvPeep8x16);
}

void DisplayVolume()
{
oled.setCursor(40,2);
oled.set2X();
oled.setFont(lcdnums12x16);
v = radio.getVolume();
if (v < 10) oled.print('0');
oled.print(v);
oled.set1X();
oled.setFont(ZevvPeep8x16);
}

void DisplayService()
{
oled.setCursor(0,6);
oled.print(Service);  
}

void DisplayStatus()
{
oled.setCursor(2,0); 
radio.getRadioInfo(&ri);
if (ri.rssi > 10) oled.print('*'); else oled.print(' ');
if (ri.rssi > 15) oled.print('*'); else oled.print(' ');
if (ri.rssi > 20) oled.print('*'); else oled.print(' ');
if (ri.rssi > 25) oled.print('*'); else oled.print(' ');
if (ri.rssi > 30) oled.print('*'); else oled.print(' ');
if (ri.rssi > 35) oled.print('*'); else oled.print(' ');
if (ri.rssi > 40) oled.print('*'); else oled.print(' ');
oled.print(':');//
if (getVcc() <  280) oled.print('*'); else oled.print(' ');// > 3.9v
if (getVcc() <  295) oled.print('*'); else oled.print(' ');// > 3.8v
if (getVcc() <  310) oled.print('*'); else oled.print(' ');// > 3.6v
if (getVcc() <  330) oled.print('*'); else oled.print(' ');// > 3.4v
if (getVcc() <  350) oled.print('*'); else oled.print(' ');// > 3.2v
if (getVcc() <  370) oled.print('*'); else oled.print(' ');// > 3.0v
if (getVcc() <  390) oled.print('*'); else oled.print(' ');// > 2.7v
DisplayService();
}
//------------------------------------------------------------------------------
void setup()
{
Wire.begin();
Wire.setClock(400000L);

pinMode(But1,INPUT_PULLUP);
pinMode(But2,INPUT_PULLUP);

oled.begin(&Adafruit128x64, 0x3C);

radio.init();
radio.setBandFrequency(RADIO_BAND_FM, 8760);//your favourite station
radio.setMono(false);
radio.setMute(false);
radio.setVolume(01);
// setup the information chain for RDS data.
radio.attachReceiveRDS(RDS_process);
//rds.attachTimeCallback(GetTime);
rds.attachServicenNameCallback(GetServiceName);
//rds.attachTextCallback(GetRDStext);

delay(300);// DisplayTune();

r = millis(); TuneTim = millis(); StatusTim = millis();
But1Up = millis(); But1Dn = millis(); Cmd1 = millis();
But2Up = millis(); But2Dn = millis(); Cmd2 = millis();
// Read 1.1V reference against AVcc
ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
}
//------------------------------------------------------------------------------
void loop() 
{
//reading RDS string
if ((millis() - r > 50)&&(millis() - Cmd2 > 1300)) {
  radio.checkRDS(); r = millis();
                                                   }
//refresh status
if (millis() - StatusTim > 500) {
  v = radio.getVolume(); DisplayStatus(); StatusTim = millis();
char s[12];
radio.formatFrequency(s, sizeof(s));
s[5] = (char)0;
                                }
//read buttons
if (digitalRead(But1) == HIGH) {
  But1Up = millis();
                               }
if (digitalRead(But2) == HIGH) {
  But2Up = millis();
  if (millis() - Cmd2 == 1302) { DisplayTune(); }
                               }
//process tune
if (digitalRead(But1) == LOW ) {
  But1Dn = millis(); 
  if ((But1Dn - But1Up > 10)&&(But1Dn - But1Up < 100)&&(But1Up - Cmd1 > 600)) {
    Cmd1 = millis(); radio.seekUp(true); TuneTim = millis();
    oled.clear(); strcpy(Service, Empty);
      do { radio.getRadioInfo(&ri); if (millis() - TuneTim > 10000) break; }
      while ((ri.tuned == false)||(ri.rssi < 10)||(millis() - TuneTim < 500));
    DisplayTune(); DisplayStatus(); Cmd1 = millis();
                                                                              }
                               }
//process volume
if (digitalRead(But2) == LOW ) {
  But2Dn = millis();
  if ((But2Dn - But2Up > 10)&&(But2Dn - But2Up < 100)&&(But2Up - Cmd2 > 1000)) {
   if (millis() - Cmd2 > 1000) {
    radio.setVolume(0); Cmd2 = millis(); oled.clear(); DisplayVolume();
                               }
                                                                               }
  if ((But2Dn - But2Up > 10)&&(But2Dn - But2Up < 100)&&(But2Up - Cmd2 <  900)) {
    if ((v < 15)&&(radio.getVolume() == v)) radio.setVolume(v+1); Cmd2 = millis();
                                         oled.clear(); DisplayVolume();
                                                                               }
                               }

}
//--
long getVcc() 
{
  // Refresh 1.1v reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
 
  ADCSRA |= _BV(ADSC);              // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low; //4.37v = 232 4v = 356  3.7v = 398

  return result;
}
//-------------------------------------------------------------------------------
