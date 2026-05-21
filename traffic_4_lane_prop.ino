/*
  ============================================================
  4-Lane Traffic Signal
  + ALL-RED Transition between every lane change
  + Sound Triggered Lane 1 Priority (3 seconds)
  + Cooldown System (15 seconds)
  ============================================================

  PIN CONNECTIONS:
  Lane 1: RED=2, YELLOW=3, GREEN=4
  Lane 2: RED=5, YELLOW=6, GREEN=7
  Lane 3: RED=8, YELLOW=9, GREEN=10
  Lane 4: RED=11, YELLOW=12, GREEN=13
  Sound Sensor D0 → A0
  ============================================================
*/

// Lane 1
const int L1_RED    = 2;
const int L1_YELLOW = 3;
const int L1_GREEN  = 4;

// Lane 2
const int L2_RED    = 5;
const int L2_YELLOW = 6;
const int L2_GREEN  = 7;

// Lane 3
const int L3_RED    = 8;
const int L3_YELLOW = 9;
const int L3_GREEN  = 10;

// Lane 4
const int L4_RED    = 11;
const int L4_YELLOW = 12;
const int L4_GREEN  = 13;

// Sound Sensor
const int SOUND_PIN = A0;

// ── Timing ────────────────────────────────────────────────
const unsigned long GREEN_DURATION   = 5000;  // Normal green  = 5 sec
const unsigned long YELLOW_DURATION  = 2000;  // Yellow        = 2 sec
const unsigned long ALL_RED_DURATION = 1000;  // ✅ All RED     = 1 sec
const unsigned long SOUND_GREEN_TIME = 3000;  // Sound green   = 3 sec
const unsigned long COOLDOWN_TIME    = 15000; // Cooldown      = 15 sec

// ── States ────────────────────────────────────────────────
enum TrafficState {
  LANE1_GREEN,
  LANE1_YELLOW,
  ALL_RED_1,       // ✅ All RED after Lane 1 → before Lane 2
  LANE2_GREEN,
  LANE2_YELLOW,
  ALL_RED_2,       // ✅ All RED after Lane 2 → before Lane 3
  LANE3_GREEN,
  LANE3_YELLOW,
  ALL_RED_3,       // ✅ All RED after Lane 3 → before Lane 4
  LANE4_GREEN,
  LANE4_YELLOW,
  ALL_RED_4,       // ✅ All RED after Lane 4 → before Lane 1
  SOUND_OVERRIDE
};

TrafficState currentState = LANE1_GREEN;
unsigned long stateStartTime = 0;
unsigned long cooldownStart  = 0;
bool cooldownActive          = false;

// ── Turn off ALL LEDs ─────────────────────────────────────
void allOff() {
  digitalWrite(L1_RED, LOW); digitalWrite(L1_YELLOW, LOW); digitalWrite(L1_GREEN, LOW);
  digitalWrite(L2_RED, LOW); digitalWrite(L2_YELLOW, LOW); digitalWrite(L2_GREEN, LOW);
  digitalWrite(L3_RED, LOW); digitalWrite(L3_YELLOW, LOW); digitalWrite(L3_GREEN, LOW);
  digitalWrite(L4_RED, LOW); digitalWrite(L4_YELLOW, LOW); digitalWrite(L4_GREEN, LOW);
}

// ── Turn ALL lanes RED ────────────────────────────────────
void allRed() {
  allOff();
  digitalWrite(L1_RED, HIGH);
  digitalWrite(L2_RED, HIGH);
  digitalWrite(L3_RED, HIGH);
  digitalWrite(L4_RED, HIGH);
}

// ── Apply LEDs based on state ─────────────────────────────
void applyState(TrafficState state) {
  allOff();
  switch (state) {

    // ── ALL RED STATES ✅ ─────────────────────────────────
    case ALL_RED_1:
    case ALL_RED_2:
    case ALL_RED_3:
    case ALL_RED_4:
      allRed();
      break;

    // ── LANE 1 ────────────────────────────────────────────
    case LANE1_GREEN:
      digitalWrite(L1_GREEN, HIGH);
      digitalWrite(L2_RED,   HIGH);
      digitalWrite(L3_RED,   HIGH);
      digitalWrite(L4_RED,   HIGH);
      break;

    case LANE1_YELLOW:
      digitalWrite(L1_YELLOW, HIGH);
      digitalWrite(L2_RED,    HIGH);
      digitalWrite(L3_RED,    HIGH);
      digitalWrite(L4_RED,    HIGH);
      break;

    // ── LANE 2 ────────────────────────────────────────────
    case LANE2_GREEN:
      digitalWrite(L2_GREEN, HIGH);
      digitalWrite(L1_RED,   HIGH);
      digitalWrite(L3_RED,   HIGH);
      digitalWrite(L4_RED,   HIGH);
      break;

    case LANE2_YELLOW:
      digitalWrite(L2_YELLOW, HIGH);
      digitalWrite(L1_RED,    HIGH);
      digitalWrite(L3_RED,    HIGH);
      digitalWrite(L4_RED,    HIGH);
      break;

    // ── LANE 3 ────────────────────────────────────────────
    case LANE3_GREEN:
      digitalWrite(L3_GREEN, HIGH);
      digitalWrite(L1_RED,   HIGH);
      digitalWrite(L2_RED,   HIGH);
      digitalWrite(L4_RED,   HIGH);
      break;

    case LANE3_YELLOW:
      digitalWrite(L3_YELLOW, HIGH);
      digitalWrite(L1_RED,    HIGH);
      digitalWrite(L2_RED,    HIGH);
      digitalWrite(L4_RED,    HIGH);
      break;

    // ── LANE 4 ────────────────────────────────────────────
    case LANE4_GREEN:
      digitalWrite(L4_GREEN, HIGH);
      digitalWrite(L1_RED,   HIGH);
      digitalWrite(L2_RED,   HIGH);
      digitalWrite(L3_RED,   HIGH);
      break;

    case LANE4_YELLOW:
      digitalWrite(L4_YELLOW, HIGH);
      digitalWrite(L1_RED,    HIGH);
      digitalWrite(L2_RED,    HIGH);
      digitalWrite(L3_RED,    HIGH);
      break;

    // ── SOUND OVERRIDE ────────────────────────────────────
    case SOUND_OVERRIDE:
      digitalWrite(L1_GREEN, HIGH);
      digitalWrite(L2_RED,   HIGH);
      digitalWrite(L3_RED,   HIGH);
      digitalWrite(L4_RED,   HIGH);
      break;
  }
}

// ── Transition to new state ───────────────────────────────
void transitionTo(TrafficState newState) {
  currentState   = newState;
  stateStartTime = millis();
  applyState(newState);

  const char* names[] = {
    "LANE1_GREEN", "LANE1_YELLOW", "ALL_RED_1",
    "LANE2_GREEN", "LANE2_YELLOW", "ALL_RED_2",
    "LANE3_GREEN", "LANE3_YELLOW", "ALL_RED_3",
    "LANE4_GREEN", "LANE4_YELLOW", "ALL_RED_4",
    "SOUND_OVERRIDE"
  };
  Serial.print("[State] → ");
  Serial.println(names[newState]);
}

// ─────────────────────────────────────────────────────────
void setup() {
  pinMode(L1_RED, OUTPUT); pinMode(L1_YELLOW, OUTPUT); pinMode(L1_GREEN, OUTPUT);
  pinMode(L2_RED, OUTPUT); pinMode(L2_YELLOW, OUTPUT); pinMode(L2_GREEN, OUTPUT);
  pinMode(L3_RED, OUTPUT); pinMode(L3_YELLOW, OUTPUT); pinMode(L3_GREEN, OUTPUT);
  pinMode(L4_RED, OUTPUT); pinMode(L4_YELLOW, OUTPUT); pinMode(L4_GREEN, OUTPUT);
  pinMode(SOUND_PIN, INPUT);

  Serial.begin(9600);
  Serial.println("4-Lane Traffic System Ready.");
  transitionTo(LANE1_GREEN);
}

// ─────────────────────────────────────────────────────────
void loop() {
  unsigned long now     = millis();
  unsigned long elapsed = now - stateStartTime;

  // STEP 1 — Check cooldown
  if (cooldownActive) {
    if (now - cooldownStart >= COOLDOWN_TIME) {
      cooldownActive = false;
      Serial.println("Cooldown over. Ready for next trigger.");
    }
  }

  // STEP 2 — Check sound sensor
  bool soundDetected = (digitalRead(SOUND_PIN) == HIGH);

  if (soundDetected) {
    if (!cooldownActive) {
      if (currentState != SOUND_OVERRIDE && currentState != LANE1_GREEN) {

        Serial.println("Sound detected! Lane 1 → GREEN | Others → RED (3 sec)");
        Serial.println("Cooldown started (15 sec).");

        cooldownActive = true;
        cooldownStart  = now;
        transitionTo(SOUND_OVERRIDE);
        return;

      } else {
        Serial.println("Sound detected — Lane 1 already green. Ignored.");
      }
    } else {
      unsigned long remaining = (COOLDOWN_TIME - (now - cooldownStart)) / 1000;
      Serial.print("Sound ignored. Cooldown: ");
      Serial.print(remaining);
      Serial.println(" sec remaining.");
    }
  }

  // STEP 3 — Full cycle with ALL RED transitions
  switch (currentState) {

    // ── SOUND OVERRIDE ────────────────────────────────────
    case SOUND_OVERRIDE:
      if (elapsed >= SOUND_GREEN_TIME) {
        Serial.println("3 sec done. Returning to normal cycle.");
        transitionTo(LANE1_YELLOW);  // resume cleanly
      }
      break;

    // ── LANE 1 ────────────────────────────────────────────
    case LANE1_GREEN:
      if (elapsed >= GREEN_DURATION)   transitionTo(LANE1_YELLOW);
      break;
    case LANE1_YELLOW:
      if (elapsed >= YELLOW_DURATION)  transitionTo(ALL_RED_1);   // ✅ ALL RED
      break;
    case ALL_RED_1:
      if (elapsed >= ALL_RED_DURATION) transitionTo(LANE2_GREEN); // ✅ then Lane 2
      break;

    // ── LANE 2 ────────────────────────────────────────────
    case LANE2_GREEN:
      if (elapsed >= GREEN_DURATION)   transitionTo(LANE2_YELLOW);
      break;
    case LANE2_YELLOW:
      if (elapsed >= YELLOW_DURATION)  transitionTo(ALL_RED_2);   // ✅ ALL RED
      break;
    case ALL_RED_2:
      if (elapsed >= ALL_RED_DURATION) transitionTo(LANE3_GREEN); // ✅ then Lane 3
      break;

    // ── LANE 3 ────────────────────────────────────────────
    case LANE3_GREEN:
      if (elapsed >= GREEN_DURATION)   transitionTo(LANE3_YELLOW);
      break;
    case LANE3_YELLOW:
      if (elapsed >= YELLOW_DURATION)  transitionTo(ALL_RED_3);   // ✅ ALL RED
      break;
    case ALL_RED_3:
      if (elapsed >= ALL_RED_DURATION) transitionTo(LANE4_GREEN); // ✅ then Lane 4
      break;

    // ── LANE 4 ────────────────────────────────────────────
    case LANE4_GREEN:
      if (elapsed >= GREEN_DURATION)   transitionTo(LANE4_YELLOW);
      break;
    case LANE4_YELLOW:
      if (elapsed >= YELLOW_DURATION)  transitionTo(ALL_RED_4);   // ✅ ALL RED
      break;
    case ALL_RED_4:
      if (elapsed >= ALL_RED_DURATION) transitionTo(LANE1_GREEN); // ✅ back to Lane 1
      break;
  }
}
