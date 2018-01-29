#include <DS3231.h>

#include <LiquidCrystal.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "parameters.h"
#include "Display.h"
#include "Relay.h"
#include "Sensors.h"

Display display;
Relay relay;
Sensors sensors;
DS3231  rtc(SDA, SCL);

volatile uint16_t timercount;
int updateTimers = 0;
int displayMode = 0;  // toggle switch for display mode.

void setup() {
  pinMode(PIN_DISPLAY_MODE_TOGGLESW, INPUT);
  
  Serial.begin(115200);

  // In the beginnining...
  rtc.begin();
  relay.begin();
  display.begin();
  sensors.begin();

  wdt_enable(WDTO_8S);  // watchdog threshold to 8 secs
  configureTimers();
}

void loop() {
  wdt_reset();  // reset watchdog, still ok here.

  if (updateTimers != 0)
  {
    timercount++;
    checkScheduler();
    updateTimers = 0;
  }

}

/*
  Configure the timer/s and interrupts.
  This goes out of sight and out of mind quick, read about it, remember it!
*/
void configureTimers() {
  cli();                  // disable global interrupts
  TCCR1A = TCCR1B = 0;    // set entire TCCR1A and B registers to 0

  OCR1A = 15624;          // set interrupt to desired timer value
  // 16 000 000 cpu clock / (1024 prescaler * 1 hz desired interrupt frequency) - 1

  TIMSK1 |= (1 << OCIE1A);  // set the OCIE1A bit to enable timer compare interrupt

  TCCR1B |= (1 << WGM12);   // set WGM12 bit to enable CTC mode

  TCCR1B |= (1 << CS10);    // setting the CS10 bit on the TCCR1B register, the timer is running.

  // ISR(TIMER1_OVF_vect) will be called when it overflows.
  // ISR(TIMER1_COMPA_vect) will be called when it compare matches.

  TCCR1B |= (1 << CS12);    // Setting CS10 and CS12 bits = clock/1024 prescaling

  sei();                    // enable global interrupts
}

/**
  The handler for timer1 compare match interrupt
*/
ISR(TIMER1_COMPA_vect) {
  updateTimers = 1;
}

/**
   This function checks the timercount variable as it is ticked by the interupt compares, and fires off the task functions.
   Do not over-read this, it is simple.
*/
void checkScheduler() {
  if ((timercount % COUNTER_1S) == 0) task_1S();
  if ((timercount % COUNTER_10S) == 0) task_10S();
  if ((timercount % COUNTER_1M) == 0) task_1M();
  if ((timercount % COUNTER_15M) == 0) task_15M();
}

// This function is called every 5 seconds
void task_1S() {
  Serial.print(F("1S"));
  triggerScreenUpdate();
}

// This function is called every 10 Seconds
void task_10S() {
  Serial.print(F("10S"));
  //serialPrintDebug();
}

// This function is called every minute
void task_1M() {
  Serial.print(F("1M"));
  //sensors.getActualReadings();
}

// This function is called every 15 minutes
void task_15M() {
  Serial.print(F("15M"));
}

// Get the display toggle switch position.
int getDisplayToggleSwitchPosition() {
  int switchPos = digitalRead(PIN_DISPLAY_MODE_TOGGLESW);
  displayMode = (switchPos == HIGH) ? 1 : 0;
  return displayMode;
}

// Keep screen update logic separated out of the timer code.
void triggerScreenUpdate() {

  displayMode = getDisplayToggleSwitchPosition();
  displayMode = 1;

  // Update the display every 5 seconds with statuses depending on the toggle switch position.
  if (displayMode == 0) {
    // display the relay statuses
    display.updateStatuses(
      relay.statusPumpA(),
      relay.statusPumpB(),
      relay.statusFan(),
      relay.statusSolenoid());  

  } else {
    // display the time, date, temp, humidity
    display.updateSensorReadings(
      sensors.getLastTemperature(),
      sensors.getLastHumidity(),
      sensors.getLastSoilMoisture(),
      getRealDate(),
      getRealTime());
  }
}

// Print to serial monitor, the debug of readings and other values. Can be called on a whim :)
void serialPrintDebug() {
  Serial.println(F("=============== Debug start ==============="));
  Serial.println(F("== statusPumpA, statusPumpB, statusFan, statusSolenoid, getLastTemperature, getLastHumidity, getLastSoilMoisture, getRealDate, getRealTime =="));
  Serial.println(relay.statusPumpA());
  Serial.println(relay.statusPumpB());
  Serial.println(relay.statusFan());
  Serial.println(relay.statusSolenoid());
  Serial.println(sensors.getLastTemperature());
  Serial.println(sensors.getLastHumidity());
  Serial.println(sensors.getLastSoilMoisture());
  Serial.println(getRealDate());
  Serial.println(getRealTime());
  Serial.println(F("=============== Debug stop ==============="));
}

/**
 * Set the RTC time.
 * This is only to be used when time is not set.
 */
void setRealTime(byte day, byte month, int year,  byte hour, byte minute, byte second) {
  rtc.setTime(hour, minute, second);
  rtc.setDate(day, month, year);
}

/**
 * Return the RTC time string.
 */
String getRealTime() {
  return rtc.getTimeStr();
}

/**
 * Return the RTC date string.
 */
String getRealDate() {
  return rtc.getDateStr();
}

