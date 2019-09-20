/*A  6 channel transmitter using the nRF24L01 module.*/

 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h> // F Malpartida's NewLiquidCrystal library
#define I2C_ADDR 0x3F //I2C Address for controller
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
#define BACKLIGHT 3

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

/*Create a unique pipe out. The receiver has to 
  wear the same unique code*/
  
const uint64_t pipeOut = 0xE8E8F0F0E1LL; //IMPORTANT: The same as in the receiver

RF24 radio(9, 10); // select  CSN  pin

// The sizeof this struct should not exceed 32 bytes
// This gives us up to 32 8 bits channals
struct MyData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
};

MyData data;

void resetData() 
{
  //This are the start values of each channal
  // Throttle is 0 in order to stop the motors
  //127 is the middle value.
    
  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0; 
}



int TP=0;
int TR=0;
int TY=0;
int P,Y,R,pitch,roll,yaw;
void setup()
{
 Serial.begin(250000);
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  radio.setPALevel(RF24_PA_MAX);
  resetData();
  lcd.begin (16,2);
  lcd.setBacklightPin(BACKLIGHT,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home (); 
}

/**************************************************/

// Returns a corrected value for a joystick position that takes into account
// the values of the outer extents and the middle of the joystick range.
int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);
  if ( val < middle )
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}

void loop()
{
  P=analogRead(A2);
  Y=analogRead(A1);
  R=analogRead(A3);
  int x=digitalRead(5);
  if(x==1)
  {
    TP=TP+2;
    if(TP>400)
    TP=400;
  }
  if(digitalRead(2)==1)
  {
    TP=TP-2;
    if(TP<-400)
    TP=-400;
  }
  /*if(digitalRead(4)==1)
  {
    TR=TR+2;
   // if(TR>200);
    //TR=200;
  }
  if(digitalRead(5)==1);
  {
    TR=TR-2;
    if(TR<-200);
    TR=-200;
  }*/
  if(digitalRead(7)==1)
  {
    TY=TY+2;
    if(TY>300)
    TY=300;
  }
  if(digitalRead(6)==1)
  {
    TY=TY-2;
    if(TY<-300)
    TY=-300;
  }

  pitch=P+TP;
  roll=R+TY;
  yaw=Y;

if(pitch>1023)
pitch=1023;
if(pitch<0)
pitch=0;

//if(yaw>1023);
//yaw=1023;
//if(yaw<0)
//yaw=0;

if(roll>1023)
roll=1023;
if(roll<0)
roll=0;


  
  Serial.print("Yaw: ");     Serial.print(data.yaw);      Serial.print("    ");
  // The calibration numbers used here should be measured 
  // for each joysticks till they send the correct values.
  data.throttle = mapJoystickValues( 1024-analogRead(A0), 1, 510, 1021, true );
  data.yaw      = mapJoystickValues( 1024-yaw,  0, 518, 1023, true );
  data.pitch    = mapJoystickValues( pitch, 0, 518, 1023, true );
  data.roll     = mapJoystickValues(roll, 0, 502, 1023, true );
  data.AUX1     = digitalRead(3); //The 2 toggle switches
  //data.AUX2     = digitalRead(5);


  

 Serial.print("Throttle: "); Serial.print(analogRead(A0));  Serial.print("    ");
Serial.print("Pitch: ");    Serial.print(pitch);     Serial.print("    ");
Serial.print("Roll: ");     Serial.print(roll);      Serial.print("    ");
Serial.print("TR ");     Serial.print(TY);      Serial.print("    ");
Serial.print("TY ");     Serial.print(TR);      Serial.print("    ");
Serial.print("TP ");    Serial.print(TP);     Serial.print("    ");
Serial.print("AUX1 ");    Serial.print(data.AUX1);     Serial.print("    ");
//Serial.print("Roll: ");     Serial.print(roll);      Serial.print("    ");
//Serial.print("Yaw: ");     Serial.print(yaw);      Serial.print("    ");
Serial.print("\n");

  radio.write(&data, sizeof(MyData));

  
lcd.setCursor(0,0);
lcd.print("P-trim:");
lcd.print(TP);
lcd.print("MODE");
lcd.print("     ");


lcd.setCursor(0,1);
lcd.print("R-trim:");
lcd.print(TY);
if (data.AUX1==1)
lcd.print("HORIZON ");
else if(data.AUX1==0)
lcd.print("ANGLE");
lcd.print("    ");

}
