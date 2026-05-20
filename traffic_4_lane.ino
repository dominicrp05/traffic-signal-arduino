/*
  4-Lane Traffic Signal Controller
  Pins: Each lane has RED, YELLOW, GREEN
  Cycle: Each lane gets 25s green → 5s yellow → red, in order
*/

// ── Pin definitions ──────────────────────────────────────────
const int lane[4][3] = {
  //  RED   YELLOW  GREEN
  {   2,     3,      4  },   // Lane 1 – North
  {   5,     6,      7  },   // Lane 2 – South
  {   8,     9,      10 },   // Lane 3 – East
  {  11,    12,      13 }    // Lane 4 – West
};

// ── Timing (milliseconds) ────────────────────────────────────
const unsigned long GREEN_TIME  = 25000;
const unsigned long YELLOW_TIME =  5000;

// ── Helper: set a lane to a specific light ───────────────────
void setLight(int l, int red, int yellow, int green) {
  digitalWrite(lane[l][0], red);
  digitalWrite(lane[l][1], yellow);
  digitalWrite(lane[l][2], green);
}

void allRed() {
  for (int l = 0; l < 4; l++)
    setLight(l, HIGH, LOW, LOW);
}

// ── Setup ────────────────────────────────────────────────────
void setup() {
  for (int l = 0; l < 4; l++)
    for (int c = 0; c < 3; c++)
      pinMode(lane[l][c], OUTPUT);

  allRed();
  delay(2000);   // 2s all-red safety pause at startup
}

// ── Main loop: cycle through each lane ───────────────────────
void loop() {
  for (int active = 0; active < 4; active++) {

    // All other lanes → RED
    allRed();

    // Active lane → GREEN
    setLight(active, LOW, LOW, HIGH);
    delay(GREEN_TIME);

    // Active lane → YELLOW
    setLight(active, LOW, HIGH, LOW);
    delay(YELLOW_TIME);

    // Active lane → RED (brief all-red before next lane)
    setLight(active, HIGH, LOW, LOW);
    delay(1000);   // 1s all-red clearance between phases
  }
}
