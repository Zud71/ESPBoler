#include "config.h"
#include "lcd.h"

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_NOKIA_DC, LCD_NOKIA_CE,LCD_NOKIA_RST);

void setupLCD()   {

  display.begin();
  display.clearDisplay(); 

}


void setLCD(String txt) 
{
  log_i("Message: %s", txt.c_str());
  display.println(txt.c_str());
  display.display();
}

void setLCDReady() 
{
  display.clearDisplay(); 
  display.setTextSize(2);
  display.setCursor(0,15);
  display.setTextColor(WHITE, BLACK); 
  display.println(" READY ");
  display.display();
}