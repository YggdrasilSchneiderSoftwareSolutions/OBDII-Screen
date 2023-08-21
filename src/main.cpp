
#include <Arduino.h>
#include <BluetoothSerial.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <ELMduino.h>
#include <Adafruit_SSD1306.h>

void errLeds(void);
void drawtriangle(void);  // Draw triangles (outlines)
void filltriangle(void);  // Draw triangles (filled)
void drawbitmap(void);    // Draw a small bitmap image
void printinfo(const String&, const bool = false, const bool = true);

BluetoothSerial SerialBT;
ELM327 myELM327;

#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial
// Der ESP32-Core definiert kein LED_BUILTIN, die LED ist aber standardmäßig auf GPIO 2
// https://www.az-delivery.de/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/esp32-jetzt-mit-boardverwalter-installieren?page=2
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT 64
#define LOGO_WIDTH 108
// 'toyota-logo', 108x64px
const unsigned char epd_bitmap_toyota_logo [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x0f, 0xff,
  0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xe0, 0x7c, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xc0, 0x3f, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xff, 0xfe, 0x01, 0xff, 0x80, 0x1f, 0xf8, 0x07, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xfc,
  0x07, 0xff, 0x86, 0x1f, 0xfe, 0x03, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xff,
  0x0f, 0x0f, 0xff, 0x01, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0x1f, 0x8f,
  0xff, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0x1f, 0x8f, 0xff, 0x00,
  0x7f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xfe, 0x1f, 0x87, 0xfe, 0x00, 0x3f, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0x80, 0x03, 0xfe, 0x3f, 0xc7, 0xfc, 0x00, 0x1f, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xff, 0x02, 0x01, 0xfe, 0x3f, 0xc7, 0xf8, 0x04, 0x0f, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xff, 0x06, 0x00, 0x3e, 0x3f, 0xc7, 0xc0, 0x06, 0x0f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfe, 0x07,
  0x00, 0x00, 0x3f, 0xc0, 0x00, 0x0e, 0x07, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfe, 0x0f, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x07, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfe, 0x0f, 0xe0, 0x00, 0x00, 0x00,
  0x00, 0x7f, 0x07, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfc, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x03, 0xff,
  0x83, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x1f, 0xff, 0x83, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xfc, 0x00, 0x03, 0xff, 0xff, 0x83, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xfc, 0x1f, 0xff, 0xfc, 0x7f, 0xc3, 0xff, 0xff, 0x83, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xfc, 0x1f, 0xff, 0xfc, 0x3f, 0xc3, 0xff, 0xff, 0x83, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfc, 0x0f,
  0xff, 0xfc, 0x3f, 0xc3, 0xff, 0xff, 0x87, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xfc,
  0x3f, 0xc3, 0xff, 0xff, 0x07, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xfc, 0x3f, 0xc7,
  0xff, 0xff, 0x07, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xfe, 0x3f, 0xc7, 0xff, 0xff,
  0x0f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x07, 0xff, 0xfe, 0x1f, 0xc7, 0xff, 0xfe, 0x0f, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0x83, 0xff, 0xfe, 0x1f, 0x87, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xff, 0x83, 0xff, 0xfe, 0x1f, 0x87, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xff, 0xc1, 0xff, 0xfe, 0x0f, 0x07, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xe0,
  0xff, 0xff, 0x0e, 0x0f, 0xff, 0xf0, 0x7f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xf0, 0x7f, 0xff,
  0x00, 0x0f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0x00, 0x0f,
  0xff, 0x81, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0x80, 0x1f, 0xff, 0x07,
  0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0x80, 0x1f, 0xfc, 0x0f, 0xff, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xc0, 0x3f, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xc0, 0x7f, 0x80, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0x00, 0xe0, 0x70, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xc0, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
  0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xf8, 0x03, 0xc1, 0xe3, 0xe3, 0x81, 0xc0, 0x0e, 0x1f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf8, 0x03,
  0x80, 0x63, 0xc7, 0x00, 0xc0, 0x0e, 0x1f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x1f, 0x1c, 0x31,
  0xce, 0x38, 0x7c, 0x7c, 0x0f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x1e, 0x3e, 0x39, 0x8c, 0x7c,
  0x7c, 0xfc, 0x0f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x1e, 0x3f, 0x38, 0x1c, 0x7e, 0x3c, 0xfc,
  0xcf, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x1e, 0x7f, 0x1c, 0x3c, 0x7e, 0x3c, 0xf8, 0xc7, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0x1e, 0x7f, 0x1c, 0x3c, 0x7e, 0x3c, 0xf8, 0xc7, 0xff, 0xff, 0xf0,
  0xff, 0xff, 0xff, 0x1e, 0x3f, 0x3e, 0x7c, 0x7e, 0x7c, 0xf0, 0x03, 0xff, 0xff, 0xf0, 0xff, 0xff,
  0xff, 0x1e, 0x1e, 0x3e, 0x7c, 0x3c, 0x7c, 0xf0, 0x03, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x1f,
  0x00, 0x7e, 0x7e, 0x00, 0xfc, 0xf3, 0xe3, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0x1f, 0x80, 0xfe,
  0x7f, 0x01, 0xfc, 0xe3, 0xf1, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xc7,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0
};

#define BUTTON_PIN 17
// variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;  
unsigned long last_button_time = 0;

uint32_t rpm = 0;
uint32_t kmh = 0;
float oel_temp = 0.0;
float tank_fuellstand = 0;
uint16_t motor_laufzeit = 0;
float verbrauch_liter_pro_stunde = 0.0;
float verbrauch_liter_pro_100km = 0.0;

/* Schalter, um den Zeitpunkt der Berechnung für l/100km zu bestimmen */
bool kmh_ermittelt = false;
bool liter_pro_stunde_ermittelt = false;

enum class OBD_PID_State { DREHZAHL, KMH, OEL_TEMP, LAUFZEIT, TANK_FUELLSTAND, VERBRAUCH_STUNDE };
OBD_PID_State aktuellePID = OBD_PID_State::DREHZAHL;

// Klassendefinitionen für verschiedene Screens auf dem SSD1306
class OBDDataScreen
{
  protected:
    Adafruit_SSD1306 &ssd1306;
    // virtual method to override
    virtual void getDisplayText() = 0;
  public:
    OBDDataScreen(Adafruit_SSD1306 &ssd1306) : ssd1306(ssd1306) {}
    void displayScreen()
    {
      ssd1306.clearDisplay();
      getDisplayText();
      ssd1306.display();
    }
};

class FuelConsumptionScreen : public OBDDataScreen
{
  protected:
    void getDisplayText() {
      ssd1306.setTextSize(1);
      ssd1306.cp437(true);
      ssd1306.setTextColor(SSD1306_WHITE);
      ssd1306.setCursor(0,0);
      ssd1306.println(F("Verbrauch"));
      ssd1306.println(F("              l/100km\n"));
      ssd1306.setTextSize(4); 
      //ssd1306.println(F(" 6.5"));
      if (verbrauch_liter_pro_100km < 10) ssd1306.print(" ");
      ssd1306.println(verbrauch_liter_pro_100km, 1);
    }
  public:
    FuelConsumptionScreen(Adafruit_SSD1306 &ssd1306) : OBDDataScreen(ssd1306){}
};

class OilTemperatureScreen : public OBDDataScreen
{
  protected:
    void getDisplayText()
    {
      ssd1306.setTextSize(1);
      ssd1306.cp437(true); // Use full 256 char 'Code Page 437' font
      ssd1306.setTextColor(SSD1306_WHITE);
      ssd1306.setCursor(0,0);
      ssd1306.println(F("\231l-Temp"));
      ssd1306.println(F("              CELCIUS\n"));
      ssd1306.setTextSize(4); 
      //ssd1306.println(F("34.5"));
      if (oel_temp < 10) ssd1306.print(" ");
      ssd1306.println(oel_temp, 1);
    }
  public:
    OilTemperatureScreen(Adafruit_SSD1306 &ssd1306) : OBDDataScreen(ssd1306){}
};

class RunTimeScreen : public OBDDataScreen
{
  private:
    String getLaufzeitFormatiert()
    {
      auto fuehrendeNull = [&](int zahl) -> String
      {
        if (zahl == 0) return "00";
        else if (zahl < 10) return "0" + String(zahl);
        return String(zahl);
      };

      int stunden = 0;
      int minuten = 0;
      int sekunden = 0;

      stunden = motor_laufzeit / 3600;
      minuten = (motor_laufzeit / 60) % 60;
      sekunden = motor_laufzeit % 60;

      return fuehrendeNull(stunden) 
                + ":" 
                + fuehrendeNull(minuten) 
                + ":" 
                + fuehrendeNull(sekunden);
    }
  protected:
    void getDisplayText()
    {
      ssd1306.setTextSize(1);
      ssd1306.cp437(true);
      ssd1306.setTextColor(SSD1306_WHITE);
      ssd1306.setCursor(0,0);
      ssd1306.println(F("             Laufzeit"));
      ssd1306.setTextSize(2);
      //ssd1306.println(F(" 00:12:34\n"));
      ssd1306.println(" " + getLaufzeitFormatiert());
      ssd1306.setTextSize(1);
      ssd1306.println(F("\n        Tankf\201llung \045"));
      ssd1306.setTextSize(2); 
      //ssd1306.println(F(" 76.8"));
      ssd1306.print(" ");
      ssd1306.println(tank_fuellstand, 1);
    }
  public:
    RunTimeScreen(Adafruit_SSD1306 &ssd1306) : OBDDataScreen(ssd1306){}
};

class KmhRpmScreen : public OBDDataScreen
{
  protected:
    void getDisplayText() {
      ssd1306.setTextSize(1);
      ssd1306.cp437(true);
      ssd1306.setTextColor(SSD1306_WHITE);
      ssd1306.setCursor(0,0);
      ssd1306.println(F("                 Km/h"));
      ssd1306.setTextSize(3); 
      //ssd1306.println(F(" 58"));
      ssd1306.print(" ");
      ssd1306.println(kmh);
      ssd1306.setTextSize(1);
      ssd1306.println(F("             Drehzahl"));
      ssd1306.setTextSize(2);
      // Pfeile werden zwischen 0 und 5000 Umdrehungen berechnet
      // maximal können 10 Zeichen angezeigt werden, d.h. pro 500 ein Pfeil
      int anzahlRpmPfeile = rpm / 500;
      //ssd1306.println(F(">>>>>>>"));
      for (int i = 0; i < anzahlRpmPfeile; ++i)
      {
        ssd1306.print(">");
      }
    }
  public:
    KmhRpmScreen(Adafruit_SSD1306 &ssd1306) : OBDDataScreen(ssd1306){}
};

// Da ein Array aus virtuellen abstrakten Klassen erstellt wird, muss mit Pointern gearbeitet werden, 
// da keine Objekte von dieser Klasse erstellt werden dürfen
FuelConsumptionScreen* fuel_screen = new FuelConsumptionScreen(display);
OilTemperatureScreen* oil_screen = new OilTemperatureScreen(display);
RunTimeScreen* run_time_screen = new RunTimeScreen(display);
KmhRpmScreen* kmh_rpm_screen = new KmhRpmScreen(display);
OBDDataScreen* screens[] = { fuel_screen, oil_screen, run_time_screen, kmh_rpm_screen };
byte screenIndex = 0;
const unsigned int numScreens = sizeof(screens) / sizeof(OBDDataScreen*);

// Laut Doku wird bei ESP das Attribut IRAM_ATTR für Interrupts benötigt
void IRAM_ATTR isr()
{
  // Interrupt muss ebenfalls entrauscht werden, sonst wird er mehrfach hintereinander ausgelöst
  // https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/?utm_content=cmp-true
  button_time = millis();
  if (button_time - last_button_time > 500) // Wait 500ms between each button pressed
  {
    ++screenIndex;
    if (screenIndex == numScreens) screenIndex = 0;
    last_button_time = button_time;
  }
}

void setup()
{
  // Baudrate und PIN gibt der ELM 327 vor
  DEBUG_PORT.begin(38400);

  // Durch PULLUP wird der interne Resistor des Chips verwendet, um Spannung auf dem Button zu entrauschen.
  // Dadurch braucht der Button auch keine eigene Spannung, sondern nur GND und Data
  // State ist bei Drücken LOW sonst HIGH
  // https://roboticsbackend.com/arduino-input_pullup-pinmode/
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Interrupt Mode FALLING wird immer bei Wechsel von HIGH zu LOW ausgelöst
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr, FALLING);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;) errLeds(); // Don't proceed, loop forever
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(0);

  // Show initial display buffer contents on the screen
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(3000); // Pause for 3 seconds

  // Clear the buffer
  display.clearDisplay();

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(1000);
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

  drawtriangle();  // Draw triangles (outlines)

  filltriangle();  // Draw triangles (filled)

  drawbitmap();    // Draw a small bitmap image

  display.clearDisplay();
  display.display();
  delay(500);

  // Beim Bluetooth-Aufbau bricht die Spannung teilweise ein und ein Brownout wird erkannt
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  DEBUG_PORT.println(F("Starte Aufbau"));
  printinfo(F("Starte Bluetooth..."));

  // Bluetooth als Master erstellen
  ELM_PORT.begin("ToyotaInfoScreen", true);
  ELM_PORT.setPin("1234");
  
  // Bluetooth-Verbindung über MAC ist schneller, als über Bluetoothname
  uint8_t address[6] = {0x00, 0x10, 0xCC, 0x4F, 0x36, 0x03};
  DEBUG_PORT.println("Verbinde mit MAC 00:10:CC:4F:36:03");
  printinfo(F("Verbinde mit\nMAC 00:10:CC:4F:36:03"));
  if (!ELM_PORT.connect(address))
  {
    DEBUG_PORT.println("Verbindung konnte via MAC-Adresse nicht hergestellt werden!");
    DEBUG_PORT.println("Versuche Verbindung mit Gerät 'OBDII' herzustellen...");
    if (!ELM_PORT.connect("OBDII"))
    {
      DEBUG_PORT.println("Verbindung via Bluetoothname nicht möglich!");
      printinfo(F("Bluetooth\nVerbindungsfehler!"));
      while(1) errLeds();
    }
  }

  if (!myELM327.begin(ELM_PORT, true, 2000, ISO_9141_5_BAUD_INIT, 40, 200))
  {
    DEBUG_PORT.println("Verbindung zum OBD-Scanner fehlgeschlagen!");
    printinfo(F("ELM327\nVerbindungsfehler!"));
    while (1) errLeds();
  }

  DEBUG_PORT.println("Verbindung mit ELM327 hergestellt");
  printinfo(F("Mit ELM327 verbunden"));

  // Beim Bluetooth-Aufbau bricht die Spannung teilweise ein und ein Brownout wird erkannt
  delay(1000);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); // enable brownout detector
}


void loop()
{
  switch (aktuellePID)
  {
    case OBD_PID_State::DREHZAHL:
    {
      float tempRPM = myELM327.rpm();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        rpm = (uint32_t) tempRPM;
        DEBUG_PORT.print("RPM: "); Serial.println(rpm);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        aktuellePID = OBD_PID_State::OEL_TEMP;
      }
      break;
    }

    case OBD_PID_State::OEL_TEMP:
    {
      float tempOelTemp = myELM327.oilTemp();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        oel_temp = tempOelTemp;
        DEBUG_PORT.print("Öltemperatur: "); Serial.println(oel_temp);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        aktuellePID = OBD_PID_State::TANK_FUELLSTAND;
      }
      break;
    }

    case OBD_PID_State::TANK_FUELLSTAND:
    {
      float tempTankFuellstand = myELM327.fuelLevel();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        tank_fuellstand = tempTankFuellstand;
        DEBUG_PORT.print("Tank Füllstand: "); Serial.println(tank_fuellstand);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        aktuellePID = OBD_PID_State::LAUFZEIT;
      }
      break;
    }

    case OBD_PID_State::LAUFZEIT:
    {
      uint16_t tempLaufzeit = myELM327.runTime();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        motor_laufzeit = tempLaufzeit;
        DEBUG_PORT.print("Motorlaufzeit: "); Serial.println(motor_laufzeit);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        aktuellePID = OBD_PID_State::KMH;
      }
      break;
    }

    case OBD_PID_State::KMH:
    {
      int32_t tempKMH = myELM327.kph();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        kmh = (uint32_t) tempKMH;
        kmh_ermittelt = true;
        DEBUG_PORT.print("KMH: "); Serial.println(kmh);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        aktuellePID = OBD_PID_State::VERBRAUCH_STUNDE;
      }
      break;
    }

    case OBD_PID_State::VERBRAUCH_STUNDE:
    {
      float tempLiterProStunde = myELM327.fuelRate();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        verbrauch_liter_pro_stunde = tempLiterProStunde;
        liter_pro_stunde_ermittelt = true;
        DEBUG_PORT.print("l/h: "); Serial.println(verbrauch_liter_pro_stunde);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        aktuellePID = OBD_PID_State::DREHZAHL;
      }
      break;
    }
  
    default:
      break;
  }

  if (kmh_ermittelt && liter_pro_stunde_ermittelt)
  {
    // Wir haben die Parameter für die Berechnung l/100km ermittelt und können rechnen
    // Mögliche Ansätze:
    // https://stackoverflow.com/questions/17170646/what-is-the-best-way-to-get-fuel-consumption-mpg-using-obd2-parameters
    // https://stackoverflow.com/questions/72659339/fuel-consumtion-data-via-obd2-is-wrong-can-you-help-me-out
  
    // Da Toyota den PID 5E (Fuel Rate l/h) unterstützt, wird hier damit gerechnet.
    // Um generisch zu funktionieren, sollte die Formel des ersten Links ggf. zusätzlich eingebaut
    // und die beiden Ergebnisse verglichen werden.
    verbrauch_liter_pro_100km = verbrauch_liter_pro_stunde / kmh;
  
    kmh_ermittelt = false;
    liter_pro_stunde_ermittelt = false;
  }

  // Ausgabe auf Display
  screens[screenIndex]->displayScreen();
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

void drawtriangle(void)
{
  display.clearDisplay();

  for(int16_t i = 0; i < max(display.width(), display.height()) / 2; i += 5)
  {
    display.drawTriangle(
      display.width() / 2, display.height() / 2 - i,
      display.width() / 2 - i, display.height() / 2 + i,
      display.width() / 2 + i, display.height() / 2 + i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(1000);
}

void filltriangle(void)
{
  display.clearDisplay();

  for(int16_t i = max(display.width(), display.height()) / 2; i > 0; i -= 5)
  {
    // The INVERSE color is used so triangles alternate white/black
    display.fillTriangle(
      display.width() / 2, display.height() / 2 - i,
      display.width() / 2 - i, display.height() / 2 + i,
      display.width() / 2 + i, display.height() / 2 + i, SSD1306_INVERSE);
    display.display();
    delay(1);
  }

  delay(1000);
}

void drawbitmap(void)
{
  display.clearDisplay();

  // Invert and restore display, pausing in-between
  display.invertDisplay(true);
  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    epd_bitmap_toyota_logo, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(3000);
  display.invertDisplay(false);
}

void printinfo(const String& text, const bool clear_display, const bool new_line)
{
  if (clear_display) 
    display.clearDisplay();

  if (new_line) 
    display.println(text);
  else 
    display.print(text);

  display.display();
}