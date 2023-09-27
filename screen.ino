#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define MINUTE 60000 // milliseconds

#define OPTIONS 6
const int optionPin[OPTIONS] = {2,3,4,5,6,A1};

#define GREEN 7
#define YELLOW 8
#define RED 9

#define START 10 
#define PAUSE 11 
#define RESET 12

Adafruit_AlphaNum4 screen = Adafruit_AlphaNum4();

long startTime;
const long times[OPTIONS][3] = 
   {{1  * MINUTE, 1.5 * MINUTE,  2 * MINUTE},
    {2  * MINUTE, 2.5 * MINUTE,  3 * MINUTE},
    {3  * MINUTE,   4 * MINUTE,  5 * MINUTE},
    {4  * MINUTE,   5 * MINUTE,  6 * MINUTE},
    {5  * MINUTE,   6 * MINUTE,  7 * MINUTE},
    {7 * MINUTE,  8.5 * MINUTE, 10 * MINUTE}};

bool running = false;
long elapsed = 0;


void drawTime(int seconds) {
    int n[4];
    n[3] = '0' + seconds % 10;
    seconds /= 10;
    n[2] = seconds?'0' + seconds % 6:' ';
    seconds /= 6;
    n[1] = seconds?'0' + seconds % 10:' ', true;
    seconds /= 10;
    n[0] = seconds?'0' + seconds % 10:' ';
    
    screen.writeDigitAscii(3, n[3]);
    screen.writeDigitAscii(2, n[2]);
    screen.writeDigitAscii(1, n[1], true); // true for decimal point
    screen.writeDigitAscii(0, n[0]);
    screen.blinkRate((!running && elapsed)?HT16K33_BLINK_1HZ:0); // hertz
    screen.writeDisplay();
}


void drawSelection(long myTimes[3]) {    
    char n[4];

    n[0] = '0'+(myTimes[0]/MINUTE)/10;
    n[1] = '0'+(myTimes[0]/MINUTE)%10;
    n[2] = '0'+(myTimes[2]/MINUTE)/10;
    n[3] = '0'+(myTimes[2]/MINUTE)%10;

    screen.writeDigitAscii(0, n[0]=='0'?' ':n[0]);
    screen.writeDigitAscii(1, n[1], true); // true for decimal point
    screen.writeDigitAscii(2, n[2]=='0'?' ':n[2]);
    screen.writeDigitAscii(3, n[3]);

    screen.blinkRate(HT16K33_BLINK_2HZ);
    screen.writeDisplay();
}


void checkButtons() {
    if (digitalRead(START) == LOW && !running) {
        running = true;
        startTime = millis() - elapsed;
    }
    if (digitalRead(PAUSE) == LOW && running) {
        running = false;
    }
    if (digitalRead(RESET) == LOW && !running && elapsed) {
        elapsed = 0;
    }
}


void displayLight(long times[3]) {
    if (elapsed < times[0]) {
        digitalWrite(GREEN,  LOW);
        digitalWrite(YELLOW,  LOW);
        digitalWrite(RED, LOW);
        return;
    }
    if (elapsed < times[1]) {
        digitalWrite(GREEN, HIGH);
        digitalWrite(YELLOW,  LOW);
        digitalWrite(RED,  LOW);
        return;        
    }
    if (elapsed < times[2]) {
        digitalWrite(GREEN,  LOW);
        digitalWrite(YELLOW, HIGH);
        digitalWrite(RED,  LOW);
        return;
    } 
    
    digitalWrite(GREEN,  LOW);
    digitalWrite(YELLOW,  LOW);
    digitalWrite(RED, HIGH);

}


void playWelcome() {
    char message[] = "    TRAFFIC LIGHT BY ROBERT GRZAZEK    ";
    screen.blinkRate(HT16K33_BLINK_OFF);
    for (int i=0; i<sizeof(message)/sizeof(char) - 4; i++) {
        screen.writeDigitAscii(0, message[i]);
        screen.writeDigitAscii(1, message[i+1]);
        screen.writeDigitAscii(2, message[i+2]);
        screen.writeDigitAscii(3, message[i+3]);
        screen.writeDisplay(); 
        delay(400);
    }
}


void setup() {
    Serial.begin(9600);
    startTime = millis();
    screen.begin(0x70);  // Initialize the LED backpack with the correct I2C address
    screen.setBrightness(15);
    
    pinMode(RED, OUTPUT);
    pinMode(YELLOW, OUTPUT);
    pinMode(GREEN,  OUTPUT);

    // INPUT_PULLUP mode means we detect a circuit to ground (LOW) through the switch
    pinMode(START, INPUT_PULLUP);
    pinMode(PAUSE, INPUT_PULLUP);
    pinMode(RESET, INPUT_PULLUP);
  
    for (int i=0; i<OPTIONS; i++)
        pinMode(optionPin[i], INPUT_PULLUP);

    // Scroll some text across the screen
    playWelcome();
}

void loop() {
    // tick-tock... startTime is adjusted for pauses, we always "believe" it naively
    if (running)
        elapsed = millis() - startTime;
    
    // Handle button presses
    checkButtons();

    // Work out what mode we're in, eg 5-7 minutes
    int option = -1;
    for (int i = 0; i < OPTIONS; i++)
        if (digitalRead(optionPin[i]) == LOW)
            option=i;

    // Display speech length option until we start (if possible), else show time
    if (elapsed == 0 && option >= 0)
        drawSelection(times[option]);
    else
        drawTime(elapsed/1000);

    // Run traffic light if we know the time selection
    if (option >= 0)
        displayLight(times[option]);
}
