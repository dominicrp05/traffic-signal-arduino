/*
  ============================================================
  2-Lane Traffic Signal with Sound-Triggered Lane 1 Priority
  + COOLDOWN SYSTEM (prevents re-triggering)
  ============================================================

  HARDWARE CONNECTIONS:
  Lane 1 (Sound priority lane):
    RED    → Pin 2
    YELLOW → Pin 3
    GREEN  → Pin 4

  Lane 2 (Opposite lane):
    RED    → Pin 5
    YELLOW → Pin 6
    GREEN  → Pin 7

  Sound Sensor (KY-037 or similar):
    D0     → Pin 8
    VCC    → 5V
    GND    → GND
  ============================================================
*/

// Lane 1 Pins
const int L1_RED    = 2;
const int L1_YELLOW = 3;
const int L1_GREEN  = 4;

// Lane 2 Pins
const int L2_RED    = 5;
const int L2_YELLOW = 6;
const int L2_GREEN  = 7;

// Sound Sensor Pin
const int SOUND_PIN = 8;

// Timing (milliseconds)
const unsigned long GREEN_DURATION   = 5000;   // Normal green phase = 5 sec
const unsigned long YELLOW_DURATION  = 2000;   // Yellow phase = 2 sec
const unsigned long SOUND_GREEN_TIME = 5000;   // ✅ Lane 1 green on trigger = 5 sec
const unsigned long COOLDOWN_TIME    = 15000;  // Ignore sounds for 15 sec after trigger

// Traffic States
enum TrafficState {
  LANE1_GREEN,
  LANE1_YELLOW,
  LANE2_GREEN,
  LANE2_YELLOW,
  SOUND_OVERRIDE
};

TrafficState currentState = LANE1_GREEN;
unsigned long stateStartTime  = 0;
unsigned long cooldownStart   = 0;
bool cooldownActive           = false;

void allOff() {
  digitalWrite(L1_RED,    LOW);
  digitalWrite(L1_YELLOW, LOW);
  digitalWrite(L1_GREEN,  LOW);
  digitalWrite(L2_RED,    LOW);
  digitalWrite(L2_YELLOW, LOW);
  digitalWrite(L2_GREEN,  LOW);
}

void applyState(TrafficState state) {
  allOff();
  switch (state) {
    case LANE1_GREEN:
      digitalWrite(L1_GREEN, HIGH);
      digitalWrite(L2_RED,   HIGH);
      break;
    case LANE1_YELLOW:
      digitalWrite(L1_YELLOW, HIGH);
      digitalWrite(L2_RED,    HIGH);
      break;
    case LANE2_GREEN:
      digitalWrite(L1_RED,   HIGH);
      digitalWrite(L2_GREEN, HIGH);
      break;
    case LANE2_YELLOW:
      digitalWrite(L1_RED,    HIGH);
      digitalWrite(L2_YELLOW, HIGH);
      break;
    case SOUND_OVERRIDE:
      digitalWrite(L1_GREEN, HIGH);
      digitalWrite(L2_RED,   HIGH);
      break;
  }
}

void transitionTo(TrafficState newState) {
  currentState   = newState;
  stateStartTime = millis();
  applyState(newState);
}

void setup() {
  pinMode(L1_RED,    OUTPUT);
  pinMode(L1_YELLOW, OUTPUT);
  pinMode(L1_GREEN,  OUTPUT);
  pinMode(L2_RED,    OUTPUT);
  pinMode(L2_YELLOW, OUTPUT);
  pinMode(L2_GREEN,  OUTPUT);
  pinMode(SOUND_PIN, INPUT);

  Serial.begin(9600);
  Serial.println("System Ready.");
  transitionTo(LANE1_GREEN);
}

void loop() {
  unsigned long now     = millis();
  unsigned long elapsed = now - stateStartTime;

  // STEP 1: Check if cooldown has finished
  if (cooldownActive) {
    if (now - cooldownStart >= COOLDOWN_TIME) {
      cooldownActive = false;
      Serial.println("Cooldown over. Sound trigger ready again.");
    }
  }

  // STEP 2: Check sound sensor
  bool soundDetected = (digitalRead(SOUND_PIN) == HIGH);

  if (soundDetected) {
    if (!cooldownActive) {
      if (currentState != SOUND_OVERRIDE && currentState != LANE1_GREEN) {
        Serial.println("Sound detected! Lane 1 override START (5 seconds).");
        Serial.println("Cooldown started — ignoring sounds for 15 seconds.");
        cooldownActive = true;
        cooldownStart  = now;
        transitionTo(SOUND_OVERRIDE);
        return;
      } else {
        Serial.println("Sound detected but Lane 1 already green. Ignored.");
      }
    } else {
      unsigned long remaining = (COOLDOWN_TIME - (now - cooldownStart)) / 1000;
      Serial.print("Sound ignored. Cooldown remaining: ");
      Serial.print(remaining);
      Serial.println(" sec");
    }
  }

  // STEP 3: Normal state machine
  switch (currentState) {

    case SOUND_OVERRIDE:
      if (elapsed >= SOUND_GREEN_TIME) {  // ✅ Now waits 5 seconds
        Serial.println("Sound override done. Resuming normal cycle.");
        transitionTo(LANE1_YELLOW);
      }
      break;

    case LANE1_GREEN:
      if (elapsed >= GREEN_DURATION) {
        transitionTo(LANE1_YELLOW);
      }
      break;

    case LANE1_YELLOW:
      if (elapsed >= YELLOW_DURATION) {
        transitionTo(LANE2_GREEN);
      }
      break;

    case LANE2_GREEN:
      if (elapsed >= GREEN_DURATION) {
        transitionTo(LANE2_YELLOW);
      }
      break;

    case LANE2_YELLOW:
      if (elapsed >= YELLOW_DURATION) {
        transitionTo(LANE1_GREEN);
      }
      break;
  }
}
