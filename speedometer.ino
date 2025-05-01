#include <TFT_eSPI.h>
#include <TinyGPSPlus.h>

TFT_eSPI tft = TFT_eSPI();

HardwareSerial SerialGPS(2);
TinyGPSPlus gps;

#define TFT_LIGHTGRAY 0xD69A
#define TFT_DARKGRAY  0x3186
#define TFT_CYAN      0x07FF
#define TFT_GREEN2    0x0660
#define TFT_YELLOW2   0xFEA0
#define TFT_RED2      0xC100
uint32_t targetTime = 0;
float currentSpeed = 0;
float targetSpeed = 0;
float lat = 0.0, lon = 0.0;
int satellites = 0;
bool gpsValid = false;

#define GAUGE_CENTER_X 120
#define GAUGE_CENTER_Y 160
#define GAUGE_RADIUS 80
#define NEEDLE_LENGTH 60
#define MAX_SPEED 180
#define MIN_SPEED 0
#define SPEED_SMOOTHING 0.2
#define NEEDLE_WIDTH 5
#define DIGITAL_X 60
#define DIGITAL_Y 50

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  drawGaugeBackground();
  drawDataFields();

  targetTime = millis() + 1000;
}

void loop() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  if (millis() >= targetTime) {
    targetTime = millis() + 80;

    if (gps.location.isValid() && gps.speed.isValid()) {
      targetSpeed = gps.speed.kmph();
      lat = gps.location.lat();
      lon = gps.location.lng();
      satellites = gps.satellites.value();
      gpsValid = true;
    } else {
      gpsValid = false;
      targetSpeed = 0;
    }

    currentSpeed = currentSpeed + (targetSpeed - currentSpeed) * SPEED_SMOOTHING;

    if (millis() > 5000 && gps.charsProcessed() < 10) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(10, 100);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(2);
      tft.println("No GPS Signal");
      delay(1000);
      drawGaugeBackground();
      drawDataFields();
      currentSpeed = 0;
      targetSpeed = 0;
      gpsValid = false;
    } else {
      updateDisplay();
    }
  }
}

void drawGaugeBackground() {
  tft.drawSmoothArc(GAUGE_CENTER_X, GAUGE_CENTER_Y, GAUGE_RADIUS, GAUGE_RADIUS - 8, 0, 180, TFT_LIGHTGRAY, TFT_BLACK);

  drawArcSegment(0, 60, TFT_RED2);
  drawArcSegment(60, 120, TFT_YELLOW2);
  drawArcSegment(120, 180, TFT_GREEN2);

  tft.setTextColor(TFT_LIGHTGRAY, TFT_BLACK);
  tft.setTextSize(1);

  for (int i = 0; i <= MAX_SPEED; i += 20) {
    float angle = map(i, 0, MAX_SPEED, 0, 180);
    float radAngle = radians(angle);
    int x1 = GAUGE_CENTER_X + (GAUGE_RADIUS - (i % 60 == 0 ? 15 : 8)) * cos(radAngle - PI / 2);
    int y1 = GAUGE_CENTER_Y + (GAUGE_RADIUS - (i % 60 == 0 ? 15 : 8)) * sin(radAngle - PI / 2);
    int x2 = GAUGE_CENTER_X + GAUGE_RADIUS * cos(radAngle - PI / 2);
    int y2 = GAUGE_CENTER_Y + GAUGE_RADIUS * sin(radAngle - PI / 2);
    tft.drawLine(x1, y1, x2, y2, TFT_LIGHTGRAY);

    if (i % 60 == 0) {
      int labelX = GAUGE_CENTER_X + (GAUGE_RADIUS - 25) * cos(radAngle - PI / 2);
      int labelY = GAUGE_CENTER_Y + (GAUGE_RADIUS - 25) * sin(radAngle - PI / 2);
      tft.setTextDatum(MC_DATUM);
      tft.drawNumber(i, labelX, labelY);
    }
  }

  tft.fillCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 8, TFT_DARKGRAY);
}

void drawArcSegment(int startAngle, int endAngle, uint16_t color) {
  for (int angle = startAngle; angle < endAngle; angle += 2) {
    float radAngle = radians(angle);
    int x1 = GAUGE_CENTER_X + (GAUGE_RADIUS - 20) * cos(radAngle - PI / 2);
    int y1 = GAUGE_CENTER_Y + (GAUGE_RADIUS - 20) * sin(radAngle - PI / 2);
    int x2 = GAUGE_CENTER_X + (GAUGE_RADIUS - 10) * cos(radAngle - PI / 2);
    int y2 = GAUGE_CENTER_Y + (GAUGE_RADIUS - 10) * sin(radAngle - PI / 2);
    tft.drawLine(x1, y1, x2, y2, color);
  }
}

void drawDataFields() {
  tft.fillRect(0, 0, 120, 60, TFT_BLACK);
  tft.fillRect(0, 200, 240, 40, TFT_BLACK);

  tft.setTextColor(TFT_LIGHTGRAY, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("GPS: ", 10, 210);
  tft.drawString("SAT: ", 120, 210);
  tft.drawString("LAT: ", 10, 225);
  tft.drawString("LON: ", 120, 225);
}

void updateDisplay() {
  if (currentSpeed > MAX_SPEED) currentSpeed = MAX_SPEED;
  if (currentSpeed < MIN_SPEED) currentSpeed = MIN_SPEED;

  tft.fillRect(DIGITAL_X - 50, DIGITAL_Y - 40, 100, 50, TFT_BLACK);
  if (gpsValid) {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(4);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(String(currentSpeed, 1), DIGITAL_X - 50, DIGITAL_Y - 40);
    tft.setTextSize(2);
    tft.drawString("km/h", DIGITAL_X - 50, DIGITAL_Y + 5);
  } else {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Invalid", DIGITAL_X - 50, DIGITAL_Y - 40);
    tft.drawString("Speed", DIGITAL_X - 50, DIGITAL_Y - 15);
  }

  drawNeedle(currentSpeed);

  tft.setTextColor(TFT_LIGHTGRAY, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);

  tft.fillRect(50, 210, 60, 10, TFT_BLACK);
  tft.drawString(gpsValid ? "Valid" : "Invalid", 50, 210);

  tft.fillRect(160, 210, 60, 10, TFT_BLACK);
  tft.drawString(String(satellites), 160, 210);
  uint16_t fixColor = satellites >= 6 ? TFT_GREEN : (satellites >= 3 ? TFT_YELLOW : TFT_RED);
  tft.fillCircle(200, 210, 4, fixColor);

  tft.fillRect(50, 225, 60, 10, TFT_BLACK);
  tft.fillRect(160, 225, 60, 10, TFT_BLACK);
  if (gpsValid) {
    tft.drawString(String(lat, 6), 50, 225);
    tft.drawString(String(lon, 6), 160, 225);
  } else {
    tft.drawString("---", 50, 225);
    tft.drawString("---", 160, 225);
  }
}

void drawNeedle(float speed) {
  static float oldAngle = 0;
  float angle = map(speed, 0, MAX_SPEED, 0, 180);

  if (abs(angle - oldAngle) > 0.3) {
    drawNeedleTriangle(oldAngle, TFT_BLACK);
    drawNeedleTriangle(angle, TFT_RED);
    oldAngle = angle;
  }
}

void drawNeedleTriangle(float angle, uint16_t color) {
  float radAngle = radians(angle - 90);
  float radAngleLeft = radians(angle - 90 + 90);
  float radAngleRight = radians(angle - 90 - 90);

  int x1 = GAUGE_CENTER_X + NEEDLE_LENGTH * cos(radAngle);
  int y1 = GAUGE_CENTER_Y + NEEDLE_LENGTH * sin(radAngle);
  int x2 = GAUGE_CENTER_X + (NEEDLE_WIDTH / 2) * cos(radAngleLeft);
  int y2 = GAUGE_CENTER_Y + (NEEDLE_WIDTH / 2) * sin(radAngleLeft);
  int x3 = GAUGE_CENTER_X + (NEEDLE_WIDTH / 2) * cos(radAngleRight);
  int y3 = GAUGE_CENTER_Y + (NEEDLE_WIDTH / 2) * sin(radAngleRight);

  tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);

  tft.fillCircle(GAUGE_CENTER_X, GAUGE_CENTER_Y, 8, TFT_DARKGRAY);
}