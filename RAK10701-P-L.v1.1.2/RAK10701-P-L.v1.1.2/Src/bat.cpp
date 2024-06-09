#include "../Inc/bat.h"

void batInit()
{
  batRead();
  BAT_LOG("INIT");
}

uint8_t batRead(void)
{ 
  int adcValue ;
  uint8_t batLevel = 0;
	
	adcValue = analogRead(AIN_PIN);
  
  if(adcValue >= 635) // 4.0
  {
    batLevel = 4;
  }
  else if(adcValue >= 604) // 3.8
  {
    batLevel = 3;
  }
  else if(adcValue >= 584) // 3.7
  {
    batLevel = 2;
  }
  else if(adcValue >= 562) // 3.6 567
  {
    batLevel = 1;
  }
  else if(adcValue >= 540) // 3.4
  {
    batLevel = 0;
  }
	else
	{
		batLevel = 255;
	}
  return batLevel;
}
