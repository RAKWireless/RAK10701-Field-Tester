#include "../Inc/pwm.h"

void pwmInit()
{
	pinMode(BL_PIN, OUTPUT);
}

void pwmBacklightFades(uint8_t duty, uint8_t backLight)
{
	uint8_t dc; // duty cycle
	dc  = map(backLight , 0 , 10 , 100 , 255);

  for(uint16_t i = 0 ;i < dc ;i++ ) 
  {
    analogWrite(BL_PIN, i);
    delay(duty);
  }
}

void setBackLight(uint8_t backLight)
{
	uint8_t dc; // duty cycle
	dc  = map(backLight , 0 , 10 , 100 , 255);
	analogWrite(BL_PIN, dc);
}