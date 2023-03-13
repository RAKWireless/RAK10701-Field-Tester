#include "../Inc/button.h"

uint8_t  g_screenOffFlag = 0;

volatile static time_t pressTime = 0;

volatile static uint8_t pressCount 	= 0;

void buttonIntHandle(void);

initState_t buttonInit(void)
{
  pinMode(BUTTON_INT_PIN, INPUT_PULLUP); 
  attachInterrupt(BUTTON_INT_PIN, buttonIntHandle  ,FALLING);
	return NONE; 
}

static time_t firstPressTime = 0;

void buttonIntHandle(void)
{
	pressTime = millis();
	if((millis() - firstPressTime) > 200) // for button debounce.
	{
		firstPressTime = millis(); 
		pressCount+=1;
	}
	APP_LOG("pressCount = %d",pressCount);
}

uint8_t getButtonStatus(void)
{
  uint8_t lowCount = 0;
	if(((millis() - pressTime) > 4000 ) && (digitalRead(BUTTON_INT_PIN) == LOW) && (pressCount >= 1))
	{
    pressCount = 0;
    pressTime = 0;
		
    return LONG_PRESS;
	}
	
	if(((millis() - pressTime) >= 400 ) && (pressCount == 1) && (digitalRead(BUTTON_INT_PIN) == HIGH))
	{
		uint8_t highCount = 0;
		for(uint8_t i; i < 200 ; i++)
		{
			if(digitalRead(BUTTON_INT_PIN) == HIGH)
				highCount++;
			delay(1);
		}
		if( highCount> 198 ) // Have no option but to.
		{
			pressCount = 0;
			pressTime = 0;
			return SINGLE_CLICK;
		}
	}
	
	if(((millis() - pressTime) >= 400 ) && (pressCount > 1))
	{
		uint8_t highCount = 0;
		for(uint8_t i; i < 200 ; i++)
		{
			if(digitalRead(BUTTON_INT_PIN) == HIGH)
				highCount++;
			delay(1);
		}
		if( highCount> 198 ) // Have no option but to.
		{
			pressCount = 0;
			pressTime = 0;
			return DOUBLE_CLICK;
		}
	}
	
	return BUTTONSTATE_NONE;
}

// static time_t reentrancyTime = 0;

// void buttonIntHandle(void)
// {
	// static time_t time = 0;
	// pressTime = millis();
	
	// if((millis() - reentrancyTime) > 200) // Prevent reentrancy 200ms.
	// {
		// if((millis() - time) > 100) // for button debounce.
		// {
			// time = millis(); 
			// pressCount+=1;
		// }
	// }
	// APP_LOG("pressCount = %d",pressCount);
// }

// uint8_t getButtonStatus(void)
// {
	// if(g_screenOffFlag == 1)  // Respond only to key presses when the screen is off.
	// {
		// if(((millis() - pressTime) >= 200 ) && (pressCount >= 1) && (digitalRead(BUTTON_INT_PIN) == HIGH))
		// {
			// pressCount = 0;
			// pressTime = 0;
			
			// reentrancyTime = millis();
			// return SINGLE_CLICK;
		// }
	// }
	
	// else
	// {
		// if(((millis() - pressTime) >= 300 ) && (pressCount == 1) && (digitalRead(BUTTON_INT_PIN) == HIGH))
		// {
			// pressCount = 0;
			// pressTime = 0;
			
			// reentrancyTime = millis();
			// return SINGLE_CLICK;
		// }
		// /*
		 // * Long press to turn off/on, the long press time is 5s.
		 // */
		// if(((millis() - pressTime) > 5000 ) && (digitalRead(BUTTON_INT_PIN) == LOW) && (pressCount == 1)) // Modify the long button time to 5s.
		// {
			// pressCount = 0;
			// pressTime = 0;
			
			// reentrancyTime = millis();
			// return LONG_PRESS;
		// }
		
		// if(((millis() - pressTime) < 500 ) && (pressCount > 1))
		// {
			// pressCount = 0;
			// pressTime = 0;
			
			// reentrancyTime = millis();
			// return DOUBLE_CLICK;
		// }
	// }
	
	// return BUTTONSTATE_NONE;
// }

// uint8_t getButtonStatus(void)
// {
  // uint8_t lowCount = 0;
	// /*
	 // * Long press to turn off/on, the long press time is 5s.
	 // */
	// if(((millis() - pressTime) > 5000 ) && (digitalRead(BUTTON_INT_PIN) == LOW) && (pressCount == 1)) // Modify the long button time to 5s.
	// {
    // pressCount = 0;
    // pressTime = 0;
    // return LONG_PRESS;
	// }
	
	// if(((millis() - pressTime) >= 400 ) && (pressCount == 1) && (digitalRead(BUTTON_INT_PIN) == HIGH))
	// {
    // pressCount = 0;
		// pressTime = 0;
		// return SINGLE_CLICK;
	// }
	
	// if(((millis() - pressTime) < 400 ) && (pressCount > 1))
	// {
    // pressCount = 0;
		// pressTime = 0;
		// return DOUBLE_CLICK;
	// }
	
	// return BUTTONSTATE_NONE;
// }