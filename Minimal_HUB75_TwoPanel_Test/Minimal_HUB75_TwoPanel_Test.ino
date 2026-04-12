// ================================================================
//  HUB75 Two-Panel Test — Minimal Protomatter Sketch
//  MCU:    Waveshare RP2040-LoRa (same chip as Raspberry Pi Pico)
//  Target: Two 64×64 P3 RGB panels, FM6124 driver IC, 1/16 scan
//  Goal:   Drive both panels as a single 128×64 display
//  Lib:    Adafruit Protomatter
// ================================================================
//
//  HARDWARE WIRING (HUB75 connector → RP2040 GPIO)
//  R1=GP0  G1=GP1  B1=GP2
//  R2=GP3  G2=GP4  B2=GP5
//  CLK=GP6  LAT=GP7  OE=GP8
//  A=GP9  B=GP10  C=GP11  D=GP12
//
//  PANEL CHAIN:
//  RP2040 → Panel-R (IN) → Panel-R (OUT) → Panel-L (IN)
//  Panel-L OUT — nothing connected
//
//  PROBLEM ENCOUNTERED:
//  Panel-R OUT → Panel-L IN does NOT pass RGB data through.
//  Measured no continuity on R1/G1/B1/R2/G2/B2 between the
//  two connectors on these specific panels.
//  Both panels display identical content (first 64px only).
//  Seeking advice on whether this is a panel hardware limitation
//  or a wiring/config error.
//
// ================================================================

#include <Adafruit_Protomatter.h>   // Adafruit RGB matrix library

// ── Pin definitions ──────────────────────────────────────────────
uint8_t rgbPins[]  = {0, 1, 2, 3, 4, 5};   // R1,G1,B1,R2,G2,B2
uint8_t addrPins[] = {9, 10, 11, 12};       // A,B,C,D (1/16 scan = 4 pins)
uint8_t clockPin   = 6;
uint8_t latchPin   = 7;
uint8_t oePin      = 8;

// ── Panel dimensions ─────────────────────────────────────────────
#define PANEL_WIDTH   128   // two 64px panels side by side
#define PANEL_HEIGHT   64   // 64 rows

// ── Protomatter object ───────────────────────────────────────────
Adafruit_Protomatter matrix(
  PANEL_WIDTH,  // total canvas width (128 = two panels)
  4,            // colour bit depth
  1,            // number of parallel chains (1 = daisy chain)
  rgbPins,
  4,            // number of address pins
  addrPins,
  clockPin,
  latchPin,
  oePin,
  false         // no double buffering
);

// ================================================================
//  FM6124 INIT SEQUENCE
//  These panels use FM6124 driver ICs which require a proprietary
//  unlock sequence before they will display correctly.
//  Without this, panels either stay dark or show garbage.
//  Sequence: clock 128 bits (64 per panel) with all RGB high,
//  and LAT pulsed high at specific bit positions per panel.
//  Must run BEFORE matrix.begin().
// ================================================================
void fm6124_init(void) {
  pinMode(latchPin,  OUTPUT);
  pinMode(oePin,     OUTPUT);
  pinMode(clockPin,  OUTPUT);
  for (uint8_t i = 0; i < 6; i++) pinMode(rgbPins[i], OUTPUT);

  digitalWrite(oePin,    HIGH);  // disable output during init
  digitalWrite(latchPin, LOW);

  for (uint16_t i = 0; i < 128; i++) {                         // 128 clocks = 64 per panel
    for (uint8_t p = 0; p < 6; p++) digitalWrite(rgbPins[p], HIGH);  // all RGB high
    // LAT pulses at positions 52,53 (panel 1) and 116,117 (panel 2)
    digitalWrite(latchPin, (i == 52 || i == 53 || i == 116 || i == 117) ? HIGH : LOW);
    digitalWrite(clockPin, HIGH);   // clock rising edge
    digitalWrite(clockPin, LOW);    // clock falling edge
  }

  digitalWrite(latchPin, LOW);   // latch low after init
  digitalWrite(oePin,    LOW);   // re-enable output
  delay(10);                     // settle
}

// ================================================================
//  FM6124 ROW REMAP
//  FM6124 physically swaps the two middle 16-row bands.
//  Rows 16-31 and 32-47 are exchanged in hardware.
//  This function corrects coordinates before passing to Protomatter.
// ================================================================
void tjDrawPixel(int16_t x, int16_t y, uint16_t colour) {
  int16_t m = y;
  if      (y >= 16 && y < 32) m = y + 16;  // band 1→2
  else if (y >= 32 && y < 48) m = y - 16;  // band 2→1
  matrix.drawPixel(x, m, colour);
}

// ================================================================
//  setup()
// ================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("HUB75 Two-Panel Test booting...");

  fm6124_init();   // unlock FM6124 on both panels before Protomatter starts
  fm6124_init();   // second pass for reliability

  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter status: ");
  Serial.println((int)status);   // 0 = OK, anything else = error

  if (status != PROTOMATTER_OK) {
    Serial.println("Protomatter init failed — halting.");
    while (1);   // stop here if init failed
  }

  // ── Test pattern ────────────────────────────────────────────
  // Four vertical lines at known x positions.
  // Expected if 128px chain works correctly:
  //   x=0   → far left of Panel-R  (red)
  //   x=63  → far right of Panel-R (green)
  //   x=64  → far left of Panel-L  (blue)
  //   x=127 → far right of Panel-L (yellow)
  // Actual result: all four lines appear on BOTH panels identically,
  // suggesting only 64px of data is being clocked regardless of config.

  matrix.fillScreen(0);
  matrix.drawFastVLine(0,   0, 64, matrix.color565(200, 0,   0));    // red
  matrix.drawFastVLine(63,  0, 64, matrix.color565(0,   200, 0));    // green
  matrix.drawFastVLine(64,  0, 64, matrix.color565(0,   0,   200));  // blue
  matrix.drawFastVLine(127, 0, 64, matrix.color565(200, 200, 0));    // yellow
  matrix.show();
}

// ================================================================
//  loop() — nothing needed, test pattern is static
// ================================================================
void loop() {
  // empty
}
