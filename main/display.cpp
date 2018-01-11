/*
 * display.cpp - library for GreenButler specific display control
 * Copyright Marlon van der Linde <marlon250f@gmail.com> 2018
 */
#include <LiquidCrystal.h>
#include "Arduino.h"
#include "display.h"
#include "parameters.h"

/**
 * lcd parameters:
 * rs, enable, d4, d5, d6, d7
 */
Display::Display() : lcd(2, 3, 4, 5, 6, 7) {
  lcd.begin(LCD_CHARS, LCD_LINES); 
}

void Display::begin() {
  delay(500); 
  lcd.home();
  lcd.print(SYS_GREETING); 
  delay(1500); 
  lcd.clear(); 
  lcd.setCursor(0, 1); 
  lcd.print(SYS_NAME_VERSION);   
  delay(1500); 
}

void Display::reset() {
  lcd.clear();
  lcd.home();
}

void Display::announce(String message, int seconds) {
  // todo - announce on same location/line everytime
  lcd.blink();
  lcd.print(message);
  delay(seconds * 1000);
  lcd.noBlink();
}
