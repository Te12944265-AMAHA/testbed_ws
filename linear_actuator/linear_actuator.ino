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
float POS_MIN = 20.0;
float POS_MAX = 905.0;
float MID_VAL = 462.5;

float D_MID = 1.0;

void setup() {
  // put your setup code here, to run once:
  t_begin_set = false;
  has_reached_middle = false;
  has_button_pressed = false;
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
  float cur_val = readPot();
  if (digitalRead(fw_button) == LOW && has_button_pressed == false){
    has_button_pressed = true;
    Serial.println("Starting...");
  }
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
  else if (has_button_pressed) {
    unsigned long t = millis();
    if (t_begin_set == false) {
      t_begin = t;
      t_begin_set = true;
    }
    float power = 0;
    float t_float = float(t - t_begin) / 1000.0;
    if (cur_val < POS_MAX + D_MID && cur_val > POS_MIN - 10){
      power = 255.0 * cos(0.11 * t_float);
    }
    Serial.println("t(s): " + String(t_float) + "\tVal: " + String(cur_val) + "\tPow: " + String(power));
    controlPow(power);
    if (digitalRead(bw_button) == LOW && has_button_pressed == true){
      has_button_pressed = false;
      has_reached_middle = false;
      t_begin_set = false;
      controlPow(0);
      delay(1000);
    }
  }
  else{
    controlPow(0);
    Serial.println("Not started, Val: " + String(cur_val));
  }
  delay(1);
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
