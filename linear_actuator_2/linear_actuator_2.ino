#define EXTEND 0
#define RETRACT 1

int lpwm = 10; // bw
int rpwm = 11; // fw

int fw_button = 2;
int bw_button = 3;

int pot_pin = A0;

unsigned long t_begin;
bool t_begin_set = false;
bool has_reached_middle = false;
bool has_button_pressed = false;

// limit the extension / retraction
float POT_MIN = 20.0;
float POT_LOW = 80.0;
float POT_HIGH = 840.0;
float POT_MAX = 900.0;

float D_MID = 1.0;

int state = EXTEND;

void setup() {
  // put your setup code here, to run once:
  t_begin_set = false;
  has_reached_middle = false;
  has_button_pressed = false;
  state = EXTEND;
  pinMode(lpwm, OUTPUT);
  pinMode(rpwm, OUTPUT);
  pinMode(fw_button, INPUT_PULLUP);
  pinMode(bw_button, INPUT_PULLUP);

  Serial.begin(9600);
//  while (readPot() > 2)
//    controlPow(-255);
//   exit(0);
}

void loop() {
  // each actuator has 2 states: extending (positive power) and retracting (negative power)
  // when pot reads [min, low], power is polynomial wrt pot
  // when pot reads [low, high], power is linear wrt pot
  // when pot reads [high, max], power is polynomial wrt pot
  // first, go to middle
  float cur_val = readPot();
  float power = 255.0;
  if (has_reached_middle == false){
    if (MID_VAL - cur_val > D_MID) { // not there yet
      controlPow(255);
    }
    else if (cur_val - MID_VAL > D_MID) { // too far
      controlPow(-255);
    }
    else
      has_reached_middle = true;
  }
  else { // has initialized to middle
    power = get_power(cur_val);
    controlPow(power);
  }
  delay(1);
}

float get_power(float cur_val){
  float power = 255.0;
  if (state == EXTEND){
    if (cur_val < POT_LOW)       // [, lo]
      power = get_power_low(cur_val);
    else if (cur_val < POT_HIGH) // [lo, hi]
      power = 255;
    else if (cur_val < POT_MAX)  // [hi, max]
      power = get_power_high(cur_val);
    else {                       // [max, ]
      power = 0;
      state = RETRACT;
    }  
  }
  else { // state == RETRACT
    if (cur_val > POT_HIGH)      // [hi, ]
      power = get_power_high(cur_val);
    else if (cur_val > POT_LOW)  // [lo, hi]
      power = 255;
    else if (cur_val > POT_MIN)  // [min, lo]
      power = get_power_low(cur_val);
    else {                       // [, min]
      power = 0;
      state = EXTEND;
    }
    power = -power;  
  }
  return power;
}

float get_power_low(float pot_val){
  float power = -0.05694 * pot_val*pot_val + 9.111 * pot_val - 109.4;
  return power;
}

float get_power_high(float pot_val){
  float power = -0.05694 * pot_val*pot_val + 95.66 * pot_val - 39925.0;
  return power;
}

float readPot() {
  return analogRead(pot_pin);
}

void controlPow(float power) {
  int power_int = int(power);
  Serial.println(power_int);
  if (power_int >= 0) {
    if (power_int > 255)
      power_int = 255;
    analogWrite(lpwm, 0);
    analogWrite(rpwm, power_int);
  }
  else {
    if (power_int < -255)
      power_int = -255;
    analogWrite(lpwm, -power_int);
    analogWrite(rpwm, 0);
  }
}
