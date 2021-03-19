#include <LiquidCrystal.h> // including liquid crystal library
#include <Thread.h> // including Thread class of ArduinoThread library (https://github.com/ivanseidel/ArduinoThread) 
                    //to display count data while also generating clk and rst

// Description
/* This program generates a clock and reset signal 
 * and simultaneously also displays count values from the RIC
 * on a LCD.
 */

// Problems
/* In order to display the count data on the LCD while also generating the clock 
 * and reset signals, I schedule tasks with ArduinoThreads after the reset is de-asserted. This causes the 
 * de-assertion time of the Reset signal to be much greater than 1us.
 * However, the reset signal has an assertion time of about 1us without use of the Arduino Threads to display the count data
 * on the LCD.  A solution may be to use another Arduino for displaying the count data.
 */


//Thread for function which creates the reset signal
Thread rstThread = Thread();

int clk = 11;  // pin 11 is OC1A output pin for Arduino mega. 
int rst = 10; 
int pin_no = 22;
int randval = 0;
int count[18]; 
long totcnt = 0; //32-bits max
// Initializing pins for the LCD
const int rs = 12, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// Function to generate reset signal
void rst_signal(){
  digitalWrite(rst, HIGH);
  delayMicroseconds(425); // 1700 ccs for a 4MHz clock (0.25us * 1700 = 425us)
  digitalWrite(rst, LOW);
  delayMicroseconds(1); 
}

void setup () {
  // generates clk signal by internal timer clk  
  pinMode (clk, OUTPUT); 
  pinMode (rst, OUTPUT); 
  rstThread.onRun(rst_signal);
  rstThread.setInterval(0.426); // scheduling reset thread to run for 426us.
  randomSeed(analogRead(0)); // for random count data.
  lcd.begin(16, 2);

  // Initializing pins for the Count data.
  for(int i = 0; i < 18; i++){
    count[i] = pin_no + i;
    pinMode(count[i], OUTPUT); // for testing purposes. When testing with the RIC remove as count data will come from there
  }
  
  // Setting up 4 MHz clk signal on clk pin (register OC1A)
  // setting up using Timer 1 (16-bit timer)
  TCCR1A = 1 << COM1A0;  // Setting Compare Output Mode bits toggle pin OC1A for CTC.
  TCCR1B = (1 << WGM12) | (1 << CS10);   // Clear Timer on Compare (CTC) and no prescaling to divide clk frequency more.
  OCR1A = 1; // Counter value increases until matches 1. 
  // Please see pg. 146 in the ATmega2560 datasheet PDF for more information.
  // Also please see https://www.arduino.cc/en/Hacking/PinMapping2560 for the mapping between
  // the Output Compare (OCnx) registers and the pins on the Arduino Mega.
  
  
  digitalWrite(rst, LOW); // initially asserts reset for 1 microsecond
  delayMicroseconds(1);
}  


void loop (){
    // If rstthread scheduled to run, then runs for 426 us.
    if(rstThread.shouldRun())
      rstThread.run();
    // Displays count data on the LCD.
    get_count();   
}  


// Function to display the count data from the RIC on the LCD.
void get_count(){
  
  lcd.setCursor(0, 1);
  for(int i = 0; i < 18; i++){
    randval = random(0, 2); 
    if(randval == 1){
      digitalWrite(count[i], HIGH);
      totcnt += (long)bit(i);
    }
    else{
      digitalWrite(count[i], LOW);
    }
  }
  
  lcd.print(totcnt, 10);
  lcd.print("      "); // 6 whitespaces faster than lcd.clear()
  totcnt = 0;
  
}
