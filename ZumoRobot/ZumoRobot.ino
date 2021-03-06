/* This example drives each motor on the Zumo forward, then
backward.  The yellow user LED is on when a motor should be
running forward and off when a motor should be running backward.
If a motor on your Zumo has been flipped, you can correct its
direction by uncommenting the call to flipLeftMotor() or
flipRightMotor() in the setup() function. */

#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4LCD lcd;
LSM303 compass;
L3G gyro;

char sz[] = "Here; is some; sample;100;data;1.414;1020";
String serialResponse = "";
unsigned long previousMillis = 0;
struct inputData
{
     int valueX;
     int valueY;
};
void setup()
{
  Serial.begin(115200);
  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);

  Wire.begin();
  compass.init();
  compass.enableDefault();
  gyro.init();
  gyro.enableDefault();

  // Delay so that the robot does not move away while the user is
  // still touching it.
  delay(1000);
}
inputData convertXYDifferential(inputData in) {
    int nJoyX = in.valueX;              // Joystick X input                     (-128..+127)
    int nJoyY = in.valueY;              // Joystick Y input     
   // OUTPUTS
    int     nMotMixL;           // Motor (left)  mixed output           (-128..+127)
    int     nMotMixR;           // Motor (right) mixed output           (-128..+127)
    
    // CONFIG
    // - fPivYLimt  : The threshold at which the pivot action starts
    //                This threshold is measured in units on the Y-axis
    //                away from the X-axis (Y=0). A greater value will assign
    //                more of the joystick's range to pivot actions.
    //                Allowable range: (0..+127)
    float fPivYLimit = 32.0;
          
    // TEMP VARIABLES
    float   nMotPremixL;    // Motor (left)  premixed output        (-128..+127)
    float   nMotPremixR;    // Motor (right) premixed output        (-128..+127)
    int     nPivSpeed;      // Pivot Speed                          (-128..+127)
    float   fPivScale;      // Balance scale b/w drive and pivot(   0..1   )
    inputData out;
    // Calculate Drive Turn output due to Joystick X input
    if (nJoyY >= 0) {
      // Forward
      nMotPremixL = (nJoyX>=0)? 127.0 : (127.0 + nJoyX);
      nMotPremixR = (nJoyX>=0)? (127.0 - nJoyX) : 127.0;
    } else {
      // Reverse
      nMotPremixL = (nJoyX>=0)? (127.0 - nJoyX) : 127.0;
      nMotPremixR = (nJoyX>=0)? 127.0 : (127.0 + nJoyX);
    }
    
    // Scale Drive output due to Joystick Y input (throttle)
    nMotPremixL = nMotPremixL * nJoyY/128.0;
    nMotPremixR = nMotPremixR * nJoyY/128.0;
    
    // Now calculate pivot amount
    // - Strength of pivot (nPivSpeed) based on Joystick X input
    // - Blending of pivot vs drive (fPivScale) based on Joystick Y input
    nPivSpeed = nJoyX;
    fPivScale = (abs(nJoyY)>fPivYLimit)? 0.0 : (1.0 - abs(nJoyY)/fPivYLimit);
    
    // Calculate final mix of Drive and Pivot
    nMotMixL = (1.0-fPivScale)*nMotPremixL + fPivScale*( nPivSpeed);
    nMotMixR = (1.0-fPivScale)*nMotPremixR + fPivScale*(-nPivSpeed);
    out.valueX = nMotMixL;
    out.valueY = nMotMixR;
    return out;
}
void loop()
{
  unsigned long currentMillis = millis();
  if ( Serial.available()) {
    previousMillis = currentMillis;
    lcd.clear();
    serialResponse = Serial.readStringUntil('\r\n');

    // Convert from String Object to String.
    char buf[sizeof(sz)];
    serialResponse.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    int counter = 0;
    int nJoyX;              // Joystick X input                     (-128..+127)
    int nJoyY;              // Joystick Y input                     (-128..+127)
    while ((str = strtok_r(p, ";", &p)) != NULL) { // delimiter is the semicolon
      if(counter == 1) {
        nJoyX = atoi(str);
      }
      if(counter == 2) {
        nJoyY = atoi(str);
      }
      counter++;
    }
    inputData in = { nJoyX, nJoyY };
    inputData out = convertXYDifferential(in);
   
    motors.setLeftSpeed(map(out.valueX, -127, 127, -400, 400));  
    motors.setRightSpeed(map(out.valueY, -127, 127, -400, 400));  
  }
  if(currentMillis - previousMillis >= 100)
  {
    lcd.clear();
    motors.setRightSpeed(0);  
    motors.setLeftSpeed(0);  
    lcd.print("No recent commands");
  }
  compass.read();
  gyro.read();
  float Pi = 3.14159;
 
  // Calculate the angle of the vector y,x
  //float heading = (atan2(compass.m.y,compass.m.x) * 180) / Pi;
  float heading = compass.heading();
  // Normalize to 0-360

  Serial.print("heading;");
  Serial.println(heading);

  delay(10);
}
