#include <Servo.h>

//Set the servo angle to the claw open position
void OpenClaw();
//Set the servo angle to the claw closed position
void CloseClaw();
//Apply a one-pole digital low pass fiter to the distance read by the ultrasonic sensor
void SmoothDistance();
//Read the distance from the ultrasonic sensor
float ReadDistance();


const int MAX_USS_DISTANCE_CM = 200; //The maximum reading for the ultrasonic sensor, used in initialization in setup()
const int GROUND_THRESHOLD_HEIGHT_CM = 20; 
const int GROUND_HEIGHT_CM = 15;

const int SERVO_OPEN = 0; //Position of servo for claw to be open  //For first claw this value is 30, for second this value is 0
const int SERVO_CLOSED = 120; //Position of servo for claw to be closed //For first claw this value is 120, for second this value is 180

//Servo pins
const int SERVO_DATA = 9;

//Ultrasonic sensor(USS) pins
const int USS_VCC = 4;
const int USS_TRIG = 5;
const int USS_ECHO = 6;
const int USS_GND = 7;

//Height indicator LED
const int LED_INDICATOR_VCC = 2;
const int LED_INDICATOR_GND = 3;



bool clawEnabled = false;
bool clawActuated = false; //Store whether claw has opened or closed while it is below the threshold height
bool clawPosition = false; //Closed Claw is false, Open claw is true

Servo clawServo;

float distance = -1.0;

void setup() {
  //Ultrasonic sensor(USS) initialization
  pinMode(USS_TRIG, OUTPUT);
  pinMode(USS_ECHO, INPUT);
  pinMode(USS_VCC, OUTPUT);
  pinMode(USS_GND, OUTPUT);

  digitalWrite(USS_VCC, HIGH);
  digitalWrite(USS_GND, LOW);


  //Servo initialization
  clawServo.attach(SERVO_DATA);
  clawServo.write(SERVO_OPEN);


  //LED indicator initialization
  pinMode(LED_INDICATOR_VCC, OUTPUT);
  pinMode(LED_INDICATOR_GND, OUTPUT);

  digitalWrite(LED_INDICATOR_VCC, LOW);
  digitalWrite(LED_INDICATOR_GND, LOW);

  //Serial Initialization
  Serial.begin(115200);

}

void loop() {
  Serial.print(distance);
  Serial.println();
  //Read a new distance value from the ultrasonic sensor and apply filtering
  SmoothDistance();


  //When the claw passes below the height threshold it becomes enabled
  if(distance < GROUND_THRESHOLD_HEIGHT_CM && !clawEnabled && !clawActuated)
  {
    clawEnabled = true;
  }

  //When the claw passes above the height threshold it becomes disabled but it is now able to actuate again
  //Claw can oly actuate once while it is below the height threshold before needing to be brought above the height threshold
  if(distance > GROUND_THRESHOLD_HEIGHT_CM)
  {
    clawEnabled = false;
    clawActuated = false;

    digitalWrite(LED_INDICATOR_VCC, HIGH);
  }
  else
  {
    digitalWrite(LED_INDICATOR_VCC, LOW);
  }

  if(distance <= GROUND_HEIGHT_CM && clawEnabled && !clawActuated)
  {
    if(clawPosition == true)
    {
      CloseClaw();
      clawActuated = true;
    }

    else if(clawPosition == false)
    {
      OpenClaw();
      clawActuated = true;
    }
  }

  delay(100);
}

void OpenClaw(){
  clawServo.write(SERVO_OPEN);
  clawPosition = true;
  clawActuated = true;
}

void CloseClaw(){
  clawServo.write(SERVO_CLOSED);
  clawPosition = false;
  clawActuated = true;
}

void SmoothDistance(){
  if(distance == -1.0){
    distance = ReadDistance();
    return distance;
  }

  //By updating the distance by the change in distance, the average of multiple values can be use without actually storing multiple values
  //using factor of 0.1 approximates average of the last 10 readings
  distance += 0.1 * (ReadDistance() - distance); 

  if(distance >= MAX_USS_DISTANCE_CM)
  {
    distance = MAX_USS_DISTANCE_CM;
  }
  return distance;
}

float ReadDistance(){
  //Clear trigger pin to prevent weird stuff
  digitalWrite(USS_TRIG, LOW);
  delayMicroseconds(2);

  //Pulse the trigger pin high to send out a 40KHz signal, 
  digitalWrite(USS_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(USS_TRIG, LOW);

  //Measure the time until the echo pin flips high, this happens when the sensor detects it's signal returning
  long duration = pulseIn(USS_ECHO, HIGH);

  //Calculate the distance in using (velocity * time) and divide by two to find the one way distance
  //duration is in units microseconds
  //velocity is the speed of sound in cm/ms which is 34.3cm/ms, the duration is converted to ms using the 1/1000 conversion ratio
  //duration is divided by 2 because duration is the time for a round trip rather than one way
  return (duration * 0.001 * 34.3)/2;  
}
