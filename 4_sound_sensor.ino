/*
  ============================================================
  4-Lane Traffic Signal
  + DELTA-based sound detection (change from last reading)
  + Whoever changes the most triggers — works for any idle level
  + ALL-RED between lanes, Cooldown 15s
  ============================================================
  Lane 1: RED=2,  YELLOW=3,  GREEN=4   | Sound → A0
  Lane 2: RED=5,  YELLOW=6,  GREEN=7   | Sound → A1
  Lane 3: RED=8,  YELLOW=9,  GREEN=10  | Sound → A2
  Lane 4: RED=11, YELLOW=12, GREEN=13  | Sound → A3
  ============================================================
*/

// ── Lane Pins ─────────────────────────────────────────────
const int L1_RED=2,  L1_YELLOW=3,  L1_GREEN=4;
const int L2_RED=5,  L2_YELLOW=6,  L2_GREEN=7;
const int L3_RED=8,  L3_YELLOW=9,  L3_GREEN=10;
const int L4_RED=11, L4_YELLOW=12, L4_GREEN=13;

// ── Sound Sensor Pins ─────────────────────────────────────
const int S1=A0, S2=A1, S3=A2, S4=A3;

// ── Delta threshold — minimum change to count as sound ────
// From your data, idle noise is ~2-3 units, clap spikes ~15+
// Set to 10 to be safe — tune up if false triggers happen
const int DELTA_THRESHOLD = 10;

// ── Timing ────────────────────────────────────────────────
const unsigned long GREEN_DURATION   = 5000;
const unsigned long YELLOW_DURATION  = 2000;
const unsigned long ALL_RED_DURATION = 1000;
const unsigned long SOUND_GREEN_TIME = 3000;
const unsigned long COOLDOWN_TIME    = 2000;

// ── Previous readings for delta calculation ───────────────
int prev1=0, prev2=0, prev3=0, prev4=0;

// ── States ────────────────────────────────────────────────
enum TrafficState {
  LANE1_GREEN,  LANE1_YELLOW,  ALL_RED_1,
  LANE2_GREEN,  LANE2_YELLOW,  ALL_RED_2,
  LANE3_GREEN,  LANE3_YELLOW,  ALL_RED_3,
  LANE4_GREEN,  LANE4_YELLOW,  ALL_RED_4,
  SOUND_OVERRIDE_1, SOUND_OVERRIDE_2,
  SOUND_OVERRIDE_3, SOUND_OVERRIDE_4
};

const char* stateNames[] = {
  "LANE1_GREEN",  "LANE1_YELLOW",  "ALL_RED_1",
  "LANE2_GREEN",  "LANE2_YELLOW",  "ALL_RED_2",
  "LANE3_GREEN",  "LANE3_YELLOW",  "ALL_RED_3",
  "LANE4_GREEN",  "LANE4_YELLOW",  "ALL_RED_4",
  "SOUND_OVERRIDE_1","SOUND_OVERRIDE_2",
  "SOUND_OVERRIDE_3","SOUND_OVERRIDE_4"
};

TrafficState currentState = LANE1_GREEN;
unsigned long stateStartTime = 0;
unsigned long cooldownStart  = 0;
bool cooldownActive          = false;

// ── All off ───────────────────────────────────────────────
void allOff() {
  digitalWrite(L1_RED,LOW); digitalWrite(L1_YELLOW,LOW); digitalWrite(L1_GREEN,LOW);
  digitalWrite(L2_RED,LOW); digitalWrite(L2_YELLOW,LOW); digitalWrite(L2_GREEN,LOW);
  digitalWrite(L3_RED,LOW); digitalWrite(L3_YELLOW,LOW); digitalWrite(L3_GREEN,LOW);
  digitalWrite(L4_RED,LOW); digitalWrite(L4_YELLOW,LOW); digitalWrite(L4_GREEN,LOW);
}

// ── All RED ───────────────────────────────────────────────
void allRed() {
  allOff();
  digitalWrite(L1_RED,HIGH); digitalWrite(L2_RED,HIGH);
  digitalWrite(L3_RED,HIGH); digitalWrite(L4_RED,HIGH);
}

// ── Apply state ───────────────────────────────────────────
void applyState(TrafficState state) {
  allOff();
  switch(state) {

    case ALL_RED_1: case ALL_RED_2:
    case ALL_RED_3: case ALL_RED_4:
      allRed(); break;

    case LANE1_GREEN: case SOUND_OVERRIDE_1:
      digitalWrite(L1_GREEN,HIGH);
      digitalWrite(L2_RED,HIGH); digitalWrite(L3_RED,HIGH); digitalWrite(L4_RED,HIGH);
      break;
    case LANE1_YELLOW:
      digitalWrite(L1_YELLOW,HIGH);
      digitalWrite(L2_RED,HIGH); digitalWrite(L3_RED,HIGH); digitalWrite(L4_RED,HIGH);
      break;

    case LANE2_GREEN: case SOUND_OVERRIDE_2:
      digitalWrite(L2_GREEN,HIGH);
      digitalWrite(L1_RED,HIGH); digitalWrite(L3_RED,HIGH); digitalWrite(L4_RED,HIGH);
      break;
    case LANE2_YELLOW:
      digitalWrite(L2_YELLOW,HIGH);
      digitalWrite(L1_RED,HIGH); digitalWrite(L3_RED,HIGH); digitalWrite(L4_RED,HIGH);
      break;

    case LANE3_GREEN: case SOUND_OVERRIDE_3:
      digitalWrite(L3_GREEN,HIGH);
      digitalWrite(L1_RED,HIGH); digitalWrite(L2_RED,HIGH); digitalWrite(L4_RED,HIGH);
      break;
    case LANE3_YELLOW:
      digitalWrite(L3_YELLOW,HIGH);
      digitalWrite(L1_RED,HIGH); digitalWrite(L2_RED,HIGH); digitalWrite(L4_RED,HIGH);
      break;

    case LANE4_GREEN: case SOUND_OVERRIDE_4:
      digitalWrite(L4_GREEN,HIGH);
      digitalWrite(L1_RED,HIGH); digitalWrite(L2_RED,HIGH); digitalWrite(L3_RED,HIGH);
      break;
    case LANE4_YELLOW:
      digitalWrite(L4_YELLOW,HIGH);
      digitalWrite(L1_RED,HIGH); digitalWrite(L2_RED,HIGH); digitalWrite(L3_RED,HIGH);
      break;
  }
}

// ── Transition ────────────────────────────────────────────
void transitionTo(TrafficState newState) {
  currentState   = newState;
  stateStartTime = millis();
  applyState(newState);
  Serial.print("[State] → ");
  Serial.println(stateNames[newState]);
}

// ── Is lane already green? ────────────────────────────────
bool isLaneGreen(int lane) {
  if(lane==1) return(currentState==LANE1_GREEN||currentState==SOUND_OVERRIDE_1);
  if(lane==2) return(currentState==LANE2_GREEN||currentState==SOUND_OVERRIDE_2);
  if(lane==3) return(currentState==LANE3_GREEN||currentState==SOUND_OVERRIDE_3);
  if(lane==4) return(currentState==LANE4_GREEN||currentState==SOUND_OVERRIDE_4);
  return false;
}

// ── Get override state ────────────────────────────────────
TrafficState getOverrideState(int lane) {
  if(lane==1) return SOUND_OVERRIDE_1;
  if(lane==2) return SOUND_OVERRIDE_2;
  if(lane==3) return SOUND_OVERRIDE_3;
  return SOUND_OVERRIDE_4;
}

// ─────────────────────────────────────────────────────────
void setup() {
  pinMode(L1_RED,OUTPUT); pinMode(L1_YELLOW,OUTPUT); pinMode(L1_GREEN,OUTPUT);
  pinMode(L2_RED,OUTPUT); pinMode(L2_YELLOW,OUTPUT); pinMode(L2_GREEN,OUTPUT);
  pinMode(L3_RED,OUTPUT); pinMode(L3_YELLOW,OUTPUT); pinMode(L3_GREEN,OUTPUT);
  pinMode(L4_RED,OUTPUT); pinMode(L4_YELLOW,OUTPUT); pinMode(L4_GREEN,OUTPUT);

  pinMode(S1,INPUT); pinMode(S2,INPUT);
  pinMode(S3,INPUT); pinMode(S4,INPUT);

  Serial.begin(9600);

  // Warm up — take initial readings
  prev1 = analogRead(S1);
  prev2 = analogRead(S2);
  prev3 = analogRead(S3);
  prev4 = analogRead(S4);
  delay(100);

  Serial.println("4-Lane Traffic System Ready.");
  Serial.print("Delta threshold: +/-");
  Serial.println(DELTA_THRESHOLD);

  transitionTo(LANE1_GREEN);
}

// ─────────────────────────────────────────────────────────
void loop() {
  unsigned long now     = millis();
  unsigned long elapsed = now - stateStartTime;

  // STEP 1 — Cooldown check
  if(cooldownActive) {
    if(now - cooldownStart >= COOLDOWN_TIME) {
      cooldownActive = false;
      Serial.println("Cooldown over. Ready.");
    }
  }

  // STEP 2 — Read current values
  int s1 = analogRead(S1);
  int s2 = analogRead(S2);
  int s3 = analogRead(S3);
  int s4 = analogRead(S4);

  // STEP 3 — Calculate delta (change from last reading)
  int d1 = abs(s1 - prev1);
  int d2 = abs(s2 - prev2);
  int d3 = abs(s3 - prev3);
  int d4 = abs(s4 - prev4);

  // Save current as previous for next loop
  prev1=s1; prev2=s2; prev3=s3; prev4=s4;

  // STEP 4 — Find biggest delta
  if(!cooldownActive) {

    int maxDelta = DELTA_THRESHOLD;  // must beat this
    int triggeredLane = 0;

    if(d1 > maxDelta) { maxDelta = d1; triggeredLane = 1; }
    if(d2 > maxDelta) { maxDelta = d2; triggeredLane = 2; }
    if(d3 > maxDelta) { maxDelta = d3; triggeredLane = 3; }
    if(d4 > maxDelta) { maxDelta = d4; triggeredLane = 4; }

    if(triggeredLane > 0) {

      Serial.print("Delta → D1:"); Serial.print(d1);
      Serial.print(" D2:"); Serial.print(d2);
      Serial.print(" D3:"); Serial.print(d3);
      Serial.print(" D4:"); Serial.println(d4);

      if(!isLaneGreen(triggeredLane)) {
        Serial.print("Loudest delta → Lane ");
        Serial.print(triggeredLane);
        Serial.print(" (delta:+"); Serial.print(maxDelta);
        Serial.println(") → GREEN 3sec");

        cooldownActive = true;
        cooldownStart  = now;
        transitionTo(getOverrideState(triggeredLane));
        return;

      } else {
        Serial.print("Lane ");
        Serial.print(triggeredLane);
        Serial.println(" already green. Ignored.");
      }
    }

  } else {
    if(d1>DELTA_THRESHOLD || d2>DELTA_THRESHOLD ||
       d3>DELTA_THRESHOLD || d4>DELTA_THRESHOLD) {
      unsigned long rem = (COOLDOWN_TIME-(now-cooldownStart))/1000;
      Serial.print("Sound ignored. Cooldown: ");
      Serial.print(rem); Serial.println("s left.");
    }
  }

  // STEP 5 — Normal state machine
  switch(currentState) {

    case SOUND_OVERRIDE_1:
      if(elapsed>=SOUND_GREEN_TIME) transitionTo(LANE1_YELLOW); break;
    case SOUND_OVERRIDE_2:
      if(elapsed>=SOUND_GREEN_TIME) transitionTo(LANE2_YELLOW); break;
    case SOUND_OVERRIDE_3:
      if(elapsed>=SOUND_GREEN_TIME) transitionTo(LANE3_YELLOW); break;
    case SOUND_OVERRIDE_4:
      if(elapsed>=SOUND_GREEN_TIME) transitionTo(LANE4_YELLOW); break;

    case LANE1_GREEN:
      if(elapsed>=GREEN_DURATION)   transitionTo(LANE1_YELLOW); break;
    case LANE1_YELLOW:
      if(elapsed>=YELLOW_DURATION)  transitionTo(ALL_RED_1);    break;
    case ALL_RED_1:
      if(elapsed>=ALL_RED_DURATION) transitionTo(LANE2_GREEN);  break;

    case LANE2_GREEN:
      if(elapsed>=GREEN_DURATION)   transitionTo(LANE2_YELLOW); break;
    case LANE2_YELLOW:
      if(elapsed>=YELLOW_DURATION)  transitionTo(ALL_RED_2);    break;
    case ALL_RED_2:
      if(elapsed>=ALL_RED_DURATION) transitionTo(LANE3_GREEN);  break;

    case LANE3_GREEN:
      if(elapsed>=GREEN_DURATION)   transitionTo(LANE3_YELLOW); break;
    case LANE3_YELLOW:
      if(elapsed>=YELLOW_DURATION)  transitionTo(ALL_RED_3);    break;
    case ALL_RED_3:
      if(elapsed>=ALL_RED_DURATION) transitionTo(LANE4_GREEN);  break;

    case LANE4_GREEN:
      if(elapsed>=GREEN_DURATION)   transitionTo(LANE4_YELLOW); break;
    case LANE4_YELLOW:
      if(elapsed>=YELLOW_DURATION)  transitionTo(ALL_RED_4);    break;
    case ALL_RED_4:
      if(elapsed>=ALL_RED_DURATION) transitionTo(LANE1_GREEN);  break;
  }
}
