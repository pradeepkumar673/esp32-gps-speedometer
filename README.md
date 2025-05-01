# esp32-gps-speedometer
a speedometer using tft display, neo6m gps module and an esp32, very interesting project took me 3 days to do perfect ui fot the display
ESP32 GPS Speedometer
Yo! This is my ESP32 speedometer that uses a GPS module to track speed and shows it on a cool TFT display with a gauge. I built it to check my bike speed, but it works for anything moving. It also shows latitude, longitude, and satellite info.
Stuff You Need

ESP32 Board (e.g., DevKitC)
GPS Module (like NEO-6M)
TFT Display (SPI, supported by TFT_eSPI, e.g., 240x320 ILI9341)
Jumper wires, breadboard
USB or battery for power
Arduino IDE with ESP32 support

Libraries
Grab these in Arduino IDE (Sketch > Include Library > Manage Libraries):

TFT_eSPI (for the display)
TinyGPSPlus (for GPS)

Wiring



ESP32 Pin
Component



GPIO 16 (RX2)
GPS TX


GPIO 17 (TX2)
GPS RX


3.3V or 5V
GPS VCC


GND
GPS GND


(TFT Pins)
Check User_Setup.h in TFT_eSPI library


Note: Set up your TFT pins in the TFT_eSPI/User_Setup.h file. Mine uses SPI defaults (e.g., MOSI 23, SCK 18, CS 5, DC 2, RST 4).
Setup

Clone this repo: git clone https://github.com/[YourGitHubUsername]/[YourRepoName].git
Open the .ino file in Arduino IDE.
Select your ESP32 board (Tools > Board > ESP32 Dev Module).
Config TFT_eSPI:
Edit User_Setup.h in the TFT_eSPI library for your display type (e.g., ILI9341).
Set correct pins if needed.


Upload the code to your ESP32.

How It Works

Plug in the ESP32 and make sure the GPS has a clear sky view.
Wait for GPS to lock (takes a sec). The display shows:
Speed (km/h) with a gauge and digital number
GPS status (Valid/Invalid)
Number of satellites (green dot if 6+, yellow if 3-5, red if <3)
Latitude and longitude


If no GPS signal after 5 seconds, it shows "No GPS Signal".

Files
ESP32-GPS-Speedometer/
├── src/          # The Arduino code
├── LICENSE       # MIT License
└── README.md     # This file

License
MIT License. See LICENSE. Use it, tweak it, just give me a shoutout!
Tips

If the display looks weird, double-check User_Setup.h and your TFT wiring.
GPS needs open sky. Indoors = sad GPS.
Got issues? Open an issue on GitHub or ping me!

Have fun riding! 🚀
