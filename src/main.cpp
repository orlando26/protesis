

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "EMGFilters.h"
#include <Servo.h>

#define SensorInputPin A0   //sensor input pin number

#define servoPin 9

/*
  Define the `threshold` variable as 0 to calibrate the baseline value of input sEMG signals first.
  After wiring the sEMG sensors to the Arduino board, wear the sEMG sensors. Relax your muscles for a few seconds,
  you will be able to see a series of squared sEMG signals values get printed on your serial terminal.
  Choose the maximal one as the baseline by setting the `threshold` variable. Then rebuild this project.
  The `envelope`, which is the squared sEMG signal data, will be printed to the serial line.
  The developer can plot it using the Arduino SerialPlotter.

  Note:
      After calibration, Any squared value of sEMG sigal below the baseline will be treated as zero.
      It is recommended that you do calibration every time you wear the sEMG sensor.
*/
unsigned long threshold = 200;  // threshold: Relaxed baseline values.(threshold=0:in the calibration process)
unsigned long EMG_num = 0;      // EMG_num: The number of statistical signals

EMGFilters myFilter;
/*
  Set the input frequency.
  The filters work only with fixed sample frequency of
  `SAMPLE_FREQ_500HZ` or `SAMPLE_FREQ_1000HZ`.
  Inputs at other sample rates will bypass
*/
SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_500HZ;
/*
  Set the frequency of power line hum to filter out.
  For countries with 60Hz power line, change to "NOTCH_FREQ_60HZ"
*/
NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;

Servo servo;

bool fist = false;

/*
   if get EMG signal,return 1;
*/
int getEMGCount(int gforce_envelope)
{
  static long integralData = 0;
  static long integralDataEve = 0;
  static bool remainFlag = false;
  static unsigned long timeMillis = 0;
  static unsigned long timeBeginzero = 0;
  static long fistNum = 0;
  static int  TimeStandard = 200;
  /*
    The integral is processed to continuously add the signal value
    and compare the integral value of the previous sampling to determine whether the signal is continuous
   */
  integralDataEve = integralData;
  integralData += gforce_envelope;
  /* 
    If the integral is constant, and it doesn't equal 0, then the time is recorded;
    If the value of the integral starts to change again, the remainflag is true, and the time record will be re-entered next time
  */
  if ((integralDataEve == integralData) && (integralDataEve != 0))
  {
    timeMillis = millis();
    if (remainFlag)
    {
      timeBeginzero = timeMillis;
      remainFlag = false;
      return 0;
    }
    /* If the integral value exceeds 200 ms, the integral value is clear 0,return that get EMG signal */
    if ((timeMillis - timeBeginzero) > TimeStandard)
    {
      integralDataEve = integralData = 0;
      return 1;
    }
    return 0; 
  }
  else {
    remainFlag = true;
    return 0;
   }
}

void setup()
{
  myFilter.init(sampleRate, humFreq, true, true, true);
  servo.attach(servoPin);
  Serial.begin(9600);
}

void cerrar() {

}

void loop()
{
  int data = analogRead(SensorInputPin);
  int dataAfterFilter = myFilter.update(data);  // filter processing
  int envelope = sq(dataAfterFilter);   //Get envelope by squaring the input
  envelope = (envelope > threshold) ? envelope : 0;    // The data set below the base value is set to 0, indicating that it is in a relaxed state

  /* if threshold=0,explain the status it is in the calibration process,the code bollow not run.
     if get EMG singal,number++ and print
  */
  if (threshold > 0)
  {
    if (getEMGCount(envelope))
    {
      if (fist) {
        servo.write(50);
        Serial.println("No Fist");
      } else {
        servo.write(180);
        Serial.println("Fist");
      }
      fist = !fist;
    }
  }
  else {
    Serial.println(envelope);
  }
  delayMicroseconds(500);
}

