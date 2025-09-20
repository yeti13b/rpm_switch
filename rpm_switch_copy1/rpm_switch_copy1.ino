//define pins
const byte dataPin = 2;                                 //Digital pin 2
const int GROUND_PIN = 8;                               //switch relay ground
const int VCC_PIN = 9;                               //switch relay vcc
const int PINS_COUNT = 2;                               //how many switched points
const int SWITCH_PINS[PINS_COUNT] = {10,    11};         //set the pins that'll have the relays on it.
const int SWITCH_RPM[PINS_COUNT]  = {1000, 1300};       //set the activation RPM points this coincides with the above pins.

//define global variables
const int number_Of_Cylinders = 4;                      //2 rotor wankel engines fire like a 4 cylinder.
const int fires_Per_Rev = (number_Of_Cylinders / 2);

//    RPM range and time intervals.
//0.016667 (1/60) * max_Rpm is converting RPM to Hz (or RPS (revs/sec)) then multiply by how many fires_per_rev since the coil will be
// firing this many times per revoultion.
//The formula 1 / Hz (or RPS) converts Hz(RPS) to seconds. And to convert to milliseconds you need to multiply by 1000.
// example: (1 / (0.0166667 * 10000 * 2)) * 1000 = 2.99 millisecs or 0.0029 seconds
//This number is important to help eliminate noise in the pulses. So pulse received quicker than 2.99 milliseconds is ignored.

// minimum rpm interval. any "pulse" slower (greater, more time) than the interval is ignored.
const int min_Rpm = 250;                                 //120000 is 120ms or 250 RPM
const unsigned int minRpmInterval = (1 / (0.0166667 * min_Rpm * fires_Per_Rev)) * 1000 * 1000; //See below. The same but returns MS.
//debounce variables, any "pulse" quicker (smaller, less time) than the debounce is ignored.
const int max_Rpm = 8000;
const unsigned int debounce = (1 / (0.0166667 * max_Rpm * fires_Per_Rev)) * 1000 * 1000;

//const unsigned int update_Interval = minRpmInterval / 1000;

volatile unsigned long update_Interval = 100000UL;
volatile unsigned long interval = 0;

volatile unsigned int rpm;
//volatile unsigned int currentRpm;

volatile long pulseCount = 0;
volatile unsigned long now;
volatile unsigned long pulseInterval = 0;
volatile unsigned long lastPulseTime;
volatile unsigned long hertz;
volatile unsigned long addhertz;


//functions
void rpmtrigger() {
  pulseInterval = now - lastPulseTime;

  if (pulseInterval > debounce)
  {
    if (pulseInterval < minRpmInterval)
    {
      interval = interval + pulseInterval;
      hertz = (update_Interval / pulseInterval);
      addhertz = addhertz + hertz;
      pulseCount++;
    }

    lastPulseTime = now;
  }
}

void setSwitchState(int rpm) {
  for (int i = 0; i < PINS_COUNT; i++) {
    if (rpm >= SWITCH_RPM[i]) {
      digitalWrite(SWITCH_PINS[i], LOW); //LOW is typically off but with opto-isolated relays it's reverse
    } else {
      digitalWrite(SWITCH_PINS[i], HIGH); //HIGH is typically on but with opto-isolated relays it's reverse
    }
  }
}

void setup() {
  // Define all switch pins as outputs
  for (int i = 0; i < PINS_COUNT; i++) {
    digitalWrite(SWITCH_PINS[i], HIGH); //HIGH is typically on but with opto-isolated relays it's reverse
    pinMode(SWITCH_PINS[i], OUTPUT);
  }

  digitalWrite(VCC_PIN, HIGH);
  pinMode(VCC_PIN, OUTPUT);

  digitalWrite(GROUND_PIN, LOW);
  pinMode(GROUND_PIN, OUTPUT);

  Serial.begin(9600);
  pinMode(dataPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(dataPin), rpmtrigger, FALLING);
}

void loop() {
  //if (interval > update_Interval)
  while (interval > update_Interval)
  {

    rpm = ((addhertz / pulseCount) * (1000000UL / update_Interval) * 60UL) / fires_Per_Rev;
    //currentRpm = ((addhertz / pulseCount) * (1000000UL / update_Interval) * 60UL) / fires_Per_Rev;
    //rpm = (rpm + currentRpm) / 2;

    setSwitchState(rpm);

    /*
    Serial.print("RPM = ");
    Serial.print(rpm);
    Serial.print(" hertz = ");
    Serial.println(addhertz / pulseCount);
    Serial.print(" Time = ");
    Serial.print(micros());
    Serial.print(" Interval = ");
    Serial.print(interval);
    Serial.println();
    */

    addhertz = 0;
    pulseCount = 0;
    interval = 0;
  }

  now = micros();
}