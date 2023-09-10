
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
// The ESP32-Core doesn't define LED_BUILTIN but it's GPIO 2 by default
// https://www.az-delivery.de/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/esp32-jetzt-mit-boardverwalter-installieren?page=2
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address
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
float oil_temp = 0.0;
float fuel_level = 0;
uint16_t engine_runtime = 0;
float consumption_liters_per_hour = 0.0;
float consumption_liters_per_100km = 0.0;

// Switches to determine the time for l/100km calculation
bool kmh_determined = false;
bool liters_per_hour_determined = false;

enum class OBD_PID_State { RPM, KMH, OIL_TEMPERATURE, RUNTIME, FUEL_LEVEL, CONSUMPTION_HOUR };
OBD_PID_State currentPID = OBD_PID_State::RPM;

// Class definitions for different Screens on the SSD1306
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
      if (consumption_liters_per_100km < 10) ssd1306.print(" ");
      ssd1306.println(consumption_liters_per_100km, 1);
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
      if (oil_temp < 10) ssd1306.print(" ");
      ssd1306.println(oil_temp, 1);
    }
  public:
    OilTemperatureScreen(Adafruit_SSD1306 &ssd1306) : OBDDataScreen(ssd1306){}
};

class RunTimeScreen : public OBDDataScreen
{
  private:
    String getRuntimeFormatted()
    {
      auto leadingZero = [&](int number) -> String
      {
        if (number == 0) return "00";
        else if (number < 10) return "0" + String(number);
        return String(number);
      };

      int hours = 0;
      int minutes = 0;
      int seconds = 0;

      hours = engine_runtime / 3600;
      minutes = (engine_runtime / 60) % 60;
      seconds = engine_runtime % 60;

      return leadingZero(hours) 
                + ":" 
                + leadingZero(minutes) 
                + ":" 
                + leadingZero(seconds);
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
      ssd1306.println(" " + getRuntimeFormatted());
      ssd1306.setTextSize(1);
      ssd1306.println(F("\n        Tankf\201llung \045"));
      ssd1306.setTextSize(2); 
      //ssd1306.println(F(" 76.8"));
      ssd1306.print(" ");
      ssd1306.println(fuel_level, 1);
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
      // Arrows are calculated between 0 and 5000 rpm
      // a maximum of 10 characters can be displayed, i.e. one arrow per 500
      int numberOfRpmArrows = rpm / 500;
      //ssd1306.println(F(">>>>>>>"));
      for (int i = 0; i < numberOfRpmArrows; ++i)
      {
        ssd1306.print(">");
      }
    }
  public:
    KmhRpmScreen(Adafruit_SSD1306 &ssd1306) : OBDDataScreen(ssd1306){}
};

// Since an array of virtual abstract classes is created, you have to work with pointers,
// as no objects may be created from this class
FuelConsumptionScreen* fuel_screen = new FuelConsumptionScreen(display);
OilTemperatureScreen* oil_screen = new OilTemperatureScreen(display);
RunTimeScreen* run_time_screen = new RunTimeScreen(display);
KmhRpmScreen* kmh_rpm_screen = new KmhRpmScreen(display);
OBDDataScreen* screens[] = { fuel_screen, oil_screen, run_time_screen, kmh_rpm_screen };
byte screenIndex = 0;
const unsigned int numScreens = sizeof(screens) / sizeof(OBDDataScreen*);

// According the documentation you need to define the attribute IRAM_ATTR for interrupts
void IRAM_ATTR isr()
{
  // Interrupt must also be denoised, otherwise it will be triggered several times in a row
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
  // Baudrate and PIN are defined by ELM 327
  DEBUG_PORT.begin(38400);

  // PULLUP uses the chip's internal resistor to denoise voltage on the button.
  // As a result, the button does not need its own voltage, only GND and data
  // State is LOW when pressed otherwise HIGH
  // https://roboticsbackend.com/arduino-input_pullup-pinmode/
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Interrupt Mode FALLING is triggered when HIGH changes to LOW
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

  // During Bluetooth setup, the voltage partially collapses and a brownout is detected
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  DEBUG_PORT.println(F("Starte Aufbau"));
  printinfo(F("Starte Bluetooth..."));

  // Start Bluetooth as master
  ELM_PORT.begin("ToyotaInfoScreen", true);
  ELM_PORT.setPin("1234");
  
  // Bluetooth-connection via MAC is faster than via Bluetoothname
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

  // During Bluetooth setup, the voltage partially collapses and a brownout is detected
  delay(1000);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); // enable brownout detector
}


void loop()
{
  switch (currentPID)
  {
    case OBD_PID_State::RPM:
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
      }
      currentPID = OBD_PID_State::OIL_TEMPERATURE;
      break;
    }

    case OBD_PID_State::OIL_TEMPERATURE:
    {
      float tempOilTemp = myELM327.oilTemp();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        oil_temp = tempOilTemp;
        DEBUG_PORT.print("Öltemperatur: "); Serial.println(oil_temp);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
      }
      currentPID = OBD_PID_State::FUEL_LEVEL;
      break;
    }

    case OBD_PID_State::FUEL_LEVEL:
    {
      float tempFuelLevel = myELM327.fuelLevel();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        fuel_level = tempFuelLevel;
        DEBUG_PORT.print("Tank Füllstand: "); Serial.println(fuel_level);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
      }
      currentPID = OBD_PID_State::RUNTIME;
      break;
    }

    case OBD_PID_State::RUNTIME:
    {
      uint16_t tempRuntime = myELM327.runTime();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        engine_runtime = tempRuntime;
        DEBUG_PORT.print("Motorlaufzeit: "); Serial.println(engine_runtime);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
      }
      currentPID = OBD_PID_State::KMH;
      break;
    }

    case OBD_PID_State::KMH:
    {
      int32_t tempKMH = myELM327.kph();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        kmh = (uint32_t) tempKMH;
        kmh_determined = true;
        DEBUG_PORT.print("KMH: "); Serial.println(kmh);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
      }
      currentPID = OBD_PID_State::CONSUMPTION_HOUR;
      break;
    }

    case OBD_PID_State::CONSUMPTION_HOUR:
    {
      float tempLitersPerHour = myELM327.fuelRate();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        consumption_liters_per_hour = tempLitersPerHour;
        liters_per_hour_determined = true;
        DEBUG_PORT.print("l/h: "); Serial.println(consumption_liters_per_hour);
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
      }
      currentPID = OBD_PID_State::RPM;
      break;
    }
  
    default:
      break;
  }

  if (kmh_determined && liters_per_hour_determined)
  {
    // We've all data we need for the l/100km calculation and can do the math
    // Possible solutions:
    // https://stackoverflow.com/questions/17170646/what-is-the-best-way-to-get-fuel-consumption-mpg-using-obd2-parameters
    // https://stackoverflow.com/questions/72659339/fuel-consumtion-data-via-obd2-is-wrong-can-you-help-me-out
  
    // Since Toyota supports the PID 5E (Fuel Rate l/h), this is expected here.
    // In order to work generically, the formula of the first link should also be included if necessary
    // and the two results are compared.
    consumption_liters_per_100km = consumption_liters_per_hour / kmh;
  
    kmh_determined = false;
    liters_per_hour_determined = false;
  }

  // Show data on display
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
  //display.invertDisplay(true);
  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    epd_bitmap_toyota_logo, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(3000);
  //display.invertDisplay(false);
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