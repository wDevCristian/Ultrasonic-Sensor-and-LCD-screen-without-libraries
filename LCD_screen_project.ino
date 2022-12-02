// Define PINS 
// Define LCD-pins
#define RS 12
#define E 11
#define D7 2
#define D6 3
#define D5 4
#define D4 5
#define CHOICE A0

// commands 
#define LCD_FUNCTIONSET 0x20 // Function Set to 4-bit mode, 1 line, 5x8 dots format
#define LCD_CLEAR 0x01 // Clear all the display and return cursor to home 
#define LCD_DISPLAYCONTROL 0x0C // Display ON/OFF control set to display ON, cursor OFF, blinking cursor OFF
#define LCD_ENTRYMODE 0x06 // Entry MODE set to move cursor right, shift display OFF
#define LCD_SETDDRAM 0x80 // Set DDRAM Address

// Define Ultrasonic-pins
#define TRIG 9
#define ECHO 8

// Create LCD var
byte data_pins[4];

// Ultrasonic vars
int duration = 0;
int distance = 0;
int distFilt = 0;

void setup()
{
  // Setup ultrasonic sensor's pins
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(CHOICE, INPUT);
  
  initLCD();
  loadInterface();
 
  Serial.begin(9600);
}

void loop()
{
  distance = getCorrectedDistance();
  (analogRead(CHOICE) > 900) ? printDist((byte)distance) : printDistAlternative((byte)distance);
  delay(1);
}

// --- Functions for Ultrasonic Sensor
int getDistance(){
  digitalWrite(TRIG, LOW); 
  delayMicroseconds(2); 

  digitalWrite(TRIG, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(TRIG, LOW); 
  
  duration = pulseIn(ECHO, HIGH); 

  return duration / 58.2;
}

byte getMedianFilteresSignal(byte *vals) {
  return (vals[0] < vals[1]) ? ((vals[1] < vals[2]) ? vals[1]: ((vals[2] < vals[0]) ? vals[0] : vals[2])) : ((vals[0] < vals[2]) ? vals[0] : ((vals[2] < vals[1]) ? vals[1] : vals[2]));
}

int getCorrectedDistance() {
  byte dist[3]; 
  byte i;

  // Exponential moving average filter + median filter
  for( i = 0; i < 2; i++) {
    dist[0] = (byte)(getDistance());
    delay(17);
    dist[1] = (byte)(getDistance());
    delay(17);
    dist[2] = (byte)(getDistance());
    delay(17);
    dist[0] = getMedianFilteresSignal(dist);
    distFilt += ((dist[0] << 4) - distFilt) >> 3;
  }
  
  return (distFilt >> 4);
}


// --- Functions for LCD Display
// High level functions

void initLCD() {
  // Set and configure ports
  data_pins[0] = D4;
  data_pins[1] = D5;
  data_pins[2] = D6;
  data_pins[3] = D7;

  pinMode(RS, OUTPUT); 
  pinMode(E, OUTPUT); 

  for (byte i = 0; i < 4; i++) {
    pinMode(data_pins[i], OUTPUT);
  }

  delay(50); 
  
  // Init LCM in 4 bit mode
  digitalWrite(RS, LOW); 
  digitalWrite(E, LOW); 
  delay(5);

  write4bits(0x03);
  delayMicroseconds(4500); 

  write4bits(0x03);
  delayMicroseconds(4500); 

  write4bits(0x03);
  delayMicroseconds(150);

  write4bits(LCD_FUNCTIONSET >> 4);
  delayMicroseconds(150);
  
  command(LCD_FUNCTIONSET);
  delayMicroseconds(50); 
  
  command(LCD_DISPLAYCONTROL);
  delayMicroseconds(50);
  
  command(LCD_CLEAR); 
  delay(2);
  
  command(LCD_ENTRYMODE);
  delayMicroseconds(50);
}

void loadInterface() {
  data('D'); delay(1);
  data('i'); delay(1);
  data('s'); delay(1);
  data('t'); delay(1);
  data('a'); delay(1);
  data('n'); delay(1);
  data('c'); delay(1);
  data('e'); delay(1);
  data(':'); delay(1);
  data(' '); delay(1);

  command(LCD_SETDDRAM | 0x0E);
  data('c'); delay(1);
  data('m'); delay(1);

  command(LCD_SETDDRAM | 0x0A);
}

void printDist(byte dist) {
  byte number[3];

  number[0] = dist / 100;
  number[1] = dist / 10 % 10;
  number[2] = dist % 10;

  for( byte i = 0; i < 3; i++) {
    data(number[i] + 48);
  }
  
  command(LCD_SETDDRAM | 0x0A);
}

void printDistAlternative(byte dist) {
  byte digit = dist;
  byte divider;
  byte num = dist;
  
  if(dist >= 100) {
    divider = 100;

    while(divider > 0) {
      digit /= divider; 
      num %= divider;
      divider /= 10;
      data(digit + 48);
      digit = num;
    }
    
  } else if(dist < 10) {
    data(' '); data(' ');
    data(digit + 48);
    
  } else {
    data(' ');
    divider = 10;

    while(divider > 0) {
      digit /= divider; 
      num %= divider;
      divider /= 10;
      data(digit + 48);
      digit = num;
    }
    
  }

  command(LCD_SETDDRAM | 0x0A);
}

// Mid level commands, for sending data/commands
void command(byte value) {
  digitalWrite(RS, LOW); 
  write4bits(value >> 4); 
  write4bits(value);
}

void data(byte value) {
  digitalWrite(RS, HIGH); 
  write4bits(value >> 4); 
  write4bits(value);
}

// Low level data pushing commands
void enable() {
  digitalWrite(E, LOW);
  delayMicroseconds(1);
  digitalWrite(E, HIGH); 
  delayMicroseconds(1);
  digitalWrite(E, LOW); 
  delayMicroseconds(5);
}

void write4bits(byte value) {
  for (byte i = 0; i < 4; i++) {
    digitalWrite(data_pins[i], (value >> i) & 0x01);
  }

  enable();
}
