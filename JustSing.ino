#include "Tlc5940.h"

int ERROR_OK_NOT_ZERO[3] = {1, 5, 5};

int RGB_LEN = 10;

int RGB_CHANNEL[] = {
  0,   1,  2,
  3,   4,  5,
  6,   7,  8,
  9,  10, 11,
  12, 13, 14,
  
  16, 17, 18,
  19, 20, 21,
  22, 23, 24,
  25, 26, 27,
  28, 29, 30
};

float newRGB[10 * 3];
float currentRGB[10 * 3];

byte blinkCount = 0;

void setup()
{
  pinMode(13, OUTPUT);
  Tlc.init();
  Serial.begin(115200);
  
  blinkCount = 0;
  digitalWrite(13, blinkColor());
}

void loop()
{
  blinkCount = 0;
  digitalWrite(13, blinkColor());
  readRGB();
  sendOk();
  updateTLC();  
  return;
}

void waitOk()
{ 
  boolean missingHandshake = true;
  while(missingHandshake)
  {
    if (Serial.available() == 1)
    {
        if (Serial.read() != 0)
	  loopError(ERROR_OK_NOT_ZERO);
	missingHandshake = false;      
    }
    delay(1);
  }
}

void readRGB()
{
  while(true)
  {
    if (Serial.available() == 3 * RGB_LEN)
    {
      for(int index = 0; index < RGB_LEN; index++)
      {
        newRGB[(index * 3) + 0] = ((float)Serial.read()) / 255.0;
        newRGB[(index * 3) + 1] = ((float)Serial.read()) / 255.0;
        newRGB[(index * 3) + 2] = ((float)Serial.read()) / 255.0;
      }
      return;
    }
    delay(1);
  }
}

void updateTLC()
{
  Tlc.clear();

  for(int index = 0; index < RGB_LEN; index++)
  {
    currentRGB[(index * 3) + 0] += (newRGB[(index * 3) + 0] - currentRGB[(index * 3) + 0]) * 0.3;
    currentRGB[(index * 3) + 1] += (newRGB[(index * 3) + 1] - currentRGB[(index * 3) + 1]) * 0.3;
    currentRGB[(index * 3) + 2] += (newRGB[(index * 3) + 2] - currentRGB[(index * 3) + 2]) * 0.3;
    setRGB(index, currentRGB[(index * 3) + 0], currentRGB[(index * 3) + 1], currentRGB[(index * 3) + 2]);
  }    
  
  while(Tlc.update())
  {
    delay(1);
  }
}

void sendOk()
{
  Serial.write((byte)0);
}

float sin2(float rad)
{
  return sin(rad) * 0.5 + 0.5;
}

void setRGB(int index, float r, float g, float b)
{  
    int channelR = RGB_CHANNEL[(index * 3) + 0];
    int channelG = RGB_CHANNEL[(index * 3) + 1];
    int channelB = RGB_CHANNEL[(index * 3) + 2];
    
    Tlc.set(channelR, 1.0 + r * 4094.0);
    Tlc.set(channelG, 1.0 + g * 4094.0);
    Tlc.set(channelB, 1.0 + b * 4094.0);
}

void statusLED(byte r, byte g, byte b)
{
  digitalWrite(13, (r + g + b) > 0 ? HIGH : LOW);
  analogWrite(3, r);
  analogWrite(4, g);
  analogWrite(5, b);
}

void loopError(int code [3])
{
  while(true)
  {
    delay(1000);
    statusLED(0, 5, 0);
    delay(100);
    statusLED(0, 0, 0);
    delay(1000);
    
    for(int i = 0; i < 3; i++)
    {
      statusLED(10, 0, 0);
      delay(code[i] * 100);
      statusLED(0, 0, 0);
      delay(100);
    }
    
    while(Serial.available())
    {
      Serial.write(222);
      Serial.write(code[0]);
      Serial.write(code[1]);
      Serial.write(code[2]);
      Serial.write(Serial.read());
    }
  }  
}

float blinkColor()
{
  return blinkCount++ % 2 == 0 ? 64 : 0;
}
