#include <TFT_eSPI.h>
#include <TinyGPSPlus.h>

// TFT display setup
TFT_eSPI tft = TFT_eSPI();

// GPS setup
HardwareSerial SerialGPS(2); // UART2 (RX2: GPIO 16, TX2: GPIO 17)
TinyGPSPlus gps;

// Color definitions
#define TFT_LIGHTGRAY  0xD69A
#define TFT_DARKGRAY   0x3186
#define TFT_CYAN       0x07FF
#define TFT_GREEN      0x0660
#define TFT_YELLOW     0xFEA0
#define TFT_ORANGE     0xFD20
#define TFT_RED        0xC100
#define TFT_DARKBLUE   0x0010
#define TFT_BACKGROUND 0x18E3 // Dark slate gray

// Display variables
uint32_t targetTime = 0;
float currentSpeed = 0;
float targetSpeed = 0;
float maxSpeed = 0; // For max speed recall
float tripDistance = 0;
float lat = 0.0, lon = 0.0;
int satellites = 0;
bool gpsValid = false;
bool nightMode = false;
uint32_t lastUpdateTime = 0;

// Gauge parameters
#define GAUGE_CENTER_X 120
#define GAUGE_CENTER_Y 140
#define GAUGE_RADIUS 90
#define NEEDLE_LENGTH 70
#define MAX_SPEED 180
#define MIN_SPEED 0
#define SPEED_SMOOTHING 0.15
#define NEEDLE_WIDTH 7

// Layout parameters
#define DIGITAL_X 120
#define DIGITAL_Y 40
#define INFO_X 10
#define INFO_Y 200
#define MAX_SPEED_X 200
#define MAX_SPEED_Y 40

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize TFT display
  tft.init();
  tft.setRotation(1); // Landscape with USB port on the left
  tft.fillScreen(TFT_BACKGROUND);

  // Draw static elements
  drawGaugeBackground();
  drawDataFields();

  targetTime = millis() + 1000;
}

void loop() {
  // Read GPS data
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // Update display every 50ms for smooth animation
  if (millis() >= targetTime) {
    targetTime = millis() + 50;
    lastUpdateTime = millis();

    // Get GPS data
    if (gps.location.isValid() && gps.speed.isValid()) {
      targetSpeed = gps.speed.kmph();
      if (targetSpeed > maxSpeed) maxSpeed = targetSpeed;
      
      // Calculate trip distance if moving
      if (targetSpeed > 2) { // 2 km/h threshold
        tripDistance += (targetSpeed / 3600.0) * (50.0 / 1000.0); // km
      }
      
      lat = gps.location.lat();
      lon = gps.location.lng();
      satellites = gps.satellites.value();
      gpsValid = true;
    } else {
      gpsValid = false;
      targetSpeed = 0;
    }

    // Smooth speed transition
    currentSpeed = currentSpeed + (targetSpeed - currentSpeed) * SPEED_SMOOTHING;

    // Check for no GPS signal after 5 seconds
    if (millis() > 5000 && gps.charsProcessed() < 10) {
      showNoSignalScreen();
    } else {
      // Update display
      updateDisplay();
    }
  }
}

void showNoSignalScreen() {
  tft.fillScreen(TFT_BACKGROUND);
  tft.setTextColor(TFT_ORANGE, TFT_BACKGROUND);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("NO GPS SIGNAL", 120, 100);
  tft.setTextSize(1);
  tft.drawString("Check GPS module connection", 120, 130);
  delay(1000);
  drawGaugeBackground();
  drawDataFields();
  currentSpeed = 0;
  targetSpeed = 0;
  gpsValid = false;
}

void drawGaugeBackground() {
  // Clear screen with background color
  tft.fillScreen(TFT_BACKGROUND);
  
  // Draw outer arc with 3D effect
  for (int r = GAUGE_RADIUS; r > GAUGE_RADIUS - 5; r--) {
    uint16_t color = (r == GAUGE_RADIUS) ? TFT_LIGHTGRAY : 
                    (r == GAUGE_RADIUS - 1) ? TFT_DARKGRAY : 
                    (r == GAUGE_RADIUS - 2) ? 0x2104 : TFT_BACKGROUND;
    tft.drawSmoothArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, r, r - 8, 0, 180, color, TFT_BACKGROUND);
  }

  // Draw colored zones with gradient effect
  drawGradientArcSegment(0, 60, TFT_RED, TFT_ORANGE);    // 120-180 km/h
  drawGradientArcSegment(60, 120, TFT_ORANGE, TFT_YELLOW); // 60-120 km/h
  drawGradientArcSegment(120, 180, TFT_YELLOW, TFT_GREEN);  // 0-60 km/h

  // Draw major ticks with 3D effect
  tft.setTextColor(TFT_LIGHTGRAY, TFT_BACKGROUND);
  tft.setTextSize(1);
  
  for (int i = 0; i <= MAX_SPEED; i += 20) {
    float angle = map(i, 0, MAX_SPEED, 0, 180);
    float radAngle = radians(angle);
    
    // Tick marks
    int x1 = GAUGE_CENTER_X + (GAUGE_RADIUS - 15) * cos(radAngle - PI / 2);
    int y1 = GAUGE_CENTER_Y + (GAUGE_RADIUS - 15) * sin(radAngle - PI / 2);
    int x2 = GAUGE_CENTER_X + GAUGE_RADIUS * cos(radAngle - PI / 2);
    int y2 = GAUGE_CENTER_Y + GAUGE_RADIUS * sin(radAngle - PI / 2);
    
    // 3D effect
    tft.drawLine(x1, y1, x2, y2, TFT_LIGHTGRAY);
    tft.drawLine(x1+1, y1+1, x2+1, y2+1, TFT_DARKGRAY);

    // Labels for major ticks
    if (i % 60 == 0) {
      int labelX = GAUGE_CENTER_X + (GAUGE_RADIUS - 30) * cos(radAngle - PI / 2);
      int labelY = GAUGE_CENTER_Y + (GAUGE_RADIUS - 30) * sin(radAngle - PI / 2);
      tft.setTextDatum(MC_DATUM);
      tft.drawNumber(i, labelX, labelY);
    }
  }

  // Draw center cap with 3D effect
  tft.fillCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 10, TFT_DARKGRAY);
  tft.drawCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 10, TFT_LIGHTGRAY);
  tft.fillCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 8, TFT_BLACK);
}

void drawGradientArcSegment(int startAngle, int endAngle, uint16_t startColor, uint16_t endColor) {
  int segments = endAngle - startAngle;
  for (int i = 0; i < segments; i++) {
    int angle = startAngle + i;
    float ratio = (float)i / (float)segments;
    uint16_t color = interpolateColor(startColor, endColor, ratio);
    
    float radAngle = radians(angle);
    int x1 = GAUGE_CENTER_X + (GAUGE_RADIUS - 20) * cos(radAngle - PI / 2);
    int y1 = GAUGE_CENTER_Y + (GAUGE_RADIUS - 20) * sin(radAngle - PI / 2);
    int x2 = GAUGE_CENTER_X + (GAUGE_RADIUS - 10) * cos(radAngle - PI / 2);
    int y2 = GAUGE_CENTER_Y + (GAUGE_RADIUS - 10) * sin(radAngle - PI / 2);
    
    tft.drawLine(x1, y1, x2, y2, color);
  }
}

uint16_t interpolateColor(uint16_t color1, uint16_t color2, float ratio) {
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;
  
  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;
  
  uint8_t r = r1 + (r2 - r1) * ratio;
  uint8_t g = g1 + (g2 - g1) * ratio;
  uint8_t b = b1 + (b2 - b1) * ratio;
  
  return (r << 11) | (g << 5) | b;
}

void drawDataFields() {
  // Draw digital speed background
  tft.fillRoundRect(DIGITAL_X - 60, DIGITAL_Y - 25, 120, 50, 5, TFT_DARKBLUE);
  tft.drawRoundRect(DIGITAL_X - 60, DIGITAL_Y - 25, 120, 50, 5, TFT_CYAN);
  
  // Draw max speed background
  tft.fillRoundRect(MAX_SPEED_X - 40, MAX_SPEED_Y - 15, 80, 30, 5, TFT_DARKBLUE);
  tft.drawRoundRect(MAX_SPEED_X - 40, MAX_SPEED_Y - 15, 80, 30, 5, TFT_CYAN);
  
  // Draw GPS info background
  tft.fillRoundRect(INFO_X, INFO_Y - 5, 220, 40, 5, TFT_DARKBLUE);
  tft.drawRoundRect(INFO_X, INFO_Y - 5, 220, 40, 5, TFT_CYAN);
  
  // Draw static labels
  tft.setTextColor(TFT_CYAN, TFT_DARKBLUE);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("MAX", MAX_SPEED_X, MAX_SPEED_Y - 8);
  tft.drawString("km/h", DIGITAL_X, DIGITAL_Y + 15);
}

void updateDisplay() {
  // Limit speed to gauge range
  if (currentSpeed > MAX_SPEED) currentSpeed = MAX_SPEED;
  if (currentSpeed < MIN_SPEED) currentSpeed = MIN_SPEED;

  // Update digital speed display
  tft.setTextColor(TFT_CYAN, TFT_DARKBLUE);
  tft.setTextSize(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(String(currentSpeed, 1), DIGITAL_X, DIGITAL_Y - 10);

  // Update max speed display
  tft.setTextSize(2);
  tft.drawString(String(maxSpeed, 0), MAX_SPEED_X, MAX_SPEED_Y + 8);

  // Update needle position
  drawNeedle(currentSpeed);

  // Update GPS info
  updateGPSInfo();
}

void updateGPSInfo() {
  tft.setTextColor(TFT_LIGHTGRAY, TFT_DARKBLUE);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);

  // GPS status with icon
  tft.fillRect(INFO_X + 5, INFO_Y, 100, 10, TFT_DARKBLUE);
  tft.drawString("GPS:", INFO_X + 5, INFO_Y);
  tft.drawString(gpsValid ? "Valid" : "Invalid", INFO_X + 40, INFO_Y);
  tft.fillCircle(INFO_X + 30, INFO_Y + 5, 3, gpsValid ? TFT_GREEN : TFT_RED);

  // Satellite count with quality indicator
  tft.fillRect(INFO_X + 120, INFO_Y, 100, 10, TFT_DARKBLUE);
  tft.drawString("SAT:", INFO_X + 120, INFO_Y);
  tft.drawString(String(satellites), INFO_X + 155, INFO_Y);
  uint16_t fixColor = satellites >= 6 ? TFT_GREEN : (satellites >= 3 ? TFT_YELLOW : TFT_RED);
  tft.fillCircle(INFO_X + 175, INFO_Y + 5, 3, fixColor);

  // Coordinates
  tft.fillRect(INFO_X + 5, INFO_Y + 15, 210, 10, TFT_DARKBLUE);
  tft.drawString("LAT:", INFO_X + 5, INFO_Y + 15);
  tft.drawString("LON:", INFO_X + 120, INFO_Y + 15);
  
  if (gpsValid) {
    tft.drawString(String(lat, 5), INFO_X + 35, INFO_Y + 15);
    tft.drawString(String(lon, 5), INFO_X + 150, INFO_Y + 15);
  } else {
    tft.drawString("---", INFO_X + 35, INFO_Y + 15);
    tft.drawString("---", INFO_X + 150, INFO_Y + 15);
  }

  // Trip distance
  tft.fillRect(INFO_X + 5, INFO_Y + 30, 210, 10, TFT_DARKBLUE);
  tft.drawString("TRIP:", INFO_X + 5, INFO_Y + 30);
  tft.drawString(String(tripDistance, 2) + " km", INFO_X + 40, INFO_Y + 30);
}

void drawNeedle(float speed) {
  static float oldAngle = 0;
  float angle = map(speed, 0, MAX_SPEED, 0, 180);

  // Only redraw if angle changed significantly
  if (abs(angle - oldAngle) > 0.5) {
    // Erase old needle
    drawNeedleTriangle(oldAngle, TFT_BACKGROUND);

    // Draw new needle with shadow effect
    drawNeedleTriangle(angle - 1, TFT_DARKGRAY); // Shadow
    drawNeedleTriangle(angle, TFT_RED); // Main needle

    oldAngle = angle;
  }
}

void drawNeedleTriangle(float angle, uint16_t color) {
  float radAngle = radians(angle - 90);
  float radAngleLeft = radians(angle - 90 + 20); // Wider angle for better visibility
  float radAngleRight = radians(angle - 90 - 20);

  // Calculate triangle vertices
  int x1 = GAUGE_CENTER_X + NEEDLE_LENGTH * cos(radAngle);
  int y1 = GAUGE_CENTER_Y + NEEDLE_LENGTH * sin(radAngle);
  int x2 = GAUGE_CENTER_X + NEEDLE_WIDTH * cos(radAngleLeft);
  int y2 = GAUGE_CENTER_Y + NEEDLE_WIDTH * sin(radAngleLeft);
  int x3 = GAUGE_CENTER_X + NEEDLE_WIDTH * cos(radAngleRight);
  int y3 = GAUGE_CENTER_Y + NEEDLE_WIDTH * sin(radAngleRight);

  // Draw filled triangle
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);

  // Redraw center cap
  tft.fillCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 10, TFT_DARKGRAY);
  tft.drawCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 10, TFT_LIGHTGRAY);
  tft.fillCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 8, TFT_BLACK);
}
