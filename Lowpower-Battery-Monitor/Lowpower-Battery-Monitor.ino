/** Arduino Pro Mini Lipo Battery Monitor
 *  Requires rocketscream/Low-Power library:
 *  https://github.com/rocketscream/Low-Power (or available in library manager)
 *
 * Outputs battery voltage * 100 on TTL Serial TX pin
 * Outputs "run state" to tell rPi when to shut down to allow battery to charge
 * 
 * Output strings:  (example 3.95V, run=true):
 *  Vbat: 395
 *  run: 1
 *
 * See companion program at https://github.com/phreakmonkey/pizerosolar/
 **/

#include <LowPower.h>

// POWOFF = When to instruct rPi to shutdown.
// POWON = What voltage level to reset rPi if POWOFF had been reached prior
// Don't set these too close together to avoid "whipsawing" from voltage sag
#define POWOFF 335
#define POWON 370

const int battPin = A3; // Connect directly to 3.7V battery (+) terminal
const int resetPin = 9; // RESET - connect to square "RUN" pin on Pi Zero

int sensorValue = 0;
int voltage = 0;
bool run = true;

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW); // Reset in case we're cold booting
  delay(100);
  pinMode(resetPin, INPUT);  // tri-state float for run
  Serial.begin(57600);
}

void loop() {
  sensorValue = analogRead(battPin);
  if (voltage == 0) voltage = map(sensorValue, 0, 1023, 0, 500);
  else voltage = (voltage * 3 + map(sensorValue, 0, 1023, 0, 500)) >> 2;  
  Serial.print("Vbat: ");
  Serial.println(voltage);
  if (run && voltage < POWOFF) 
      run = false;  // WARNING: Will reset board when voltage rises to POWON
  if ((!run) && voltage > POWON) {
      run = true;
      pinMode(resetPin, OUTPUT);
      digitalWrite(resetPin, LOW);  // Ground resetPin to reboot rPi
      delay(100);
      pinMode(resetPin, INPUT);  // tri-state float for run
  }
  Serial.print("run: ");
  Serial.println(run);
  delay(10);  // Allow serial data to finish transmitting
  // Go to sleep for 16 seconds:
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
