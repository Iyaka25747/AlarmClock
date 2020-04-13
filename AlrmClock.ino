/***
 * 
 * INI 4.4.2020 : ALARM CLOCK project
 * 
 ***/

#define FASTLED_ESP8266_RAW_PIN_ORDER //Mandatory for NodeMCU ESP8266 correct PINOUT for LED_STRIPE_PIN below; you can remove the line for Board_ProMini
// #define FASTLED_ESP8266_NODEMCU_PIN_ORDER
// #define FASTLED_ESP8266_D1_PIN_ORDER

#define LED_BUILT_IN_MODE // Define what is tesed, built in blue LED or a connected stripe of leds,  possbile values: LED_STRIPE_MODE or LED_BUILT_IN_MODE

#include <DNSServer.h> // ESPUI dependency
#include <ESPUI.h>     // UI, see https://github.com/s00500/ESPUI
#include <ESP8266WiFi.h>

// Load Library for LED Stripe
#include <FastLED.h>

//  Network Config, do not publish this file
#include "config.h"

// Network variables
const byte DNS_PORT = 53;
// IPAddress apIP(172, 22, 22, 1);
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

/**
 * 
 * Web Server settings
 * 
 ***/

// Replace with your network credentials
const char *ssid = SSID;    //
const char *password = PWD; //
const char *hostname = "espui";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// ESPUI config
// int statusLabelId; // Label
int millisLabelId; // Label
int testSwitchId;
int heureLabelId;

/***
 * LED Stripe settings
 ***/

#define LED_STRIPE_PIN 2 // = GPIO2 = D4 PIN on the board
#define NUM_LEDS 8       // LED stripe with 8 LEDs
#define BRIGHTNESS 32    // initial value: 64
#define LED_TYPE WS2813  // LED stripe type connected to board
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// #define UPDATES_PER_SECOND 100
#define DELAY_BETWEEN_LED_SECOND 250 // org 250
#define DELAY_BUILTIN_LED_Millis 500

/***
 * This is only used for testing
 * BuiltinLED settings
 ***/
unsigned long bilCurrentTime = millis(); // builtIn LED (bil) current time
unsigned long bilLasttime = 0;
int bilInterval = 500; // interval between blinking in milliseconds
bool bilState = HIGH;  // LED is HIGH or LOW
int bilLoop = 0;

/*
FUNCTIONS ESPUI
*/
void test_textCall1(Control *sender, int type)
{
    Serial.print("Text: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void textSetAlarmTime(Control *sender, int type)
{
    String alarmTime;
    alarmTime = sender->value;
    Serial.print("Text: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(alarmTime);
}

/**
 * 
 * FUNCTIONS LED STRIPE
 * 
 **/

#ifdef LED_BUILT_IN_MODE

void switchBuiltinLED()
{

    bilCurrentTime = millis();
    if (bilCurrentTime - bilLasttime > bilInterval) // if elapsed time more than interval let's tooggle the LED
    {
        bilLasttime = bilCurrentTime;
        // Serial.println("Toggling the BuiltIn LED");
        if (bilState)
        {
            // Serial.println("Turn Builtin LED Blue OFF");
            bilState = LOW;
            digitalWrite(LED_BUILTIN, bilState);
        }
        else
        {
            // Serial.println("Turn Builtin LED Blue ON");
            bilState = HIGH;
            digitalWrite(LED_BUILTIN, bilState); // turn the LED on (HIGH is the voltage level)
        }
    }
    // bilLoop += 1;
    // Serial.print("Loop: ");
    // Serial.println(bilLoop);
}
#endif

/**
 * 
 * SETUP
 * 
 **/

void setup()
{
    Serial.println("***");
    Serial.println("Beginning the setup");
    // pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    delay(1000); // power-up safety delay

    // ESPUI.setVerbosity(Verbosity::VerboseJSON);
    WiFi.hostname(hostname);

#ifdef LED_BUILT_IN_MODE
    Serial.print("LED BUILTIN MODE is executed, PIN LED Builtin value: ");
    Serial.println(LED_BUILTIN);
    pinMode(LED_BUILTIN, OUTPUT);
#endif

#ifdef LED_STRIPE_MODE
    Serial.print("LED Stripe MODE is executed. PIN LED Stripe Data LED_STRIPE_PIN value: ");
    Serial.println(LED_STRIPE_PIN);
    Serial.print("Brightness: ");
    Serial.println(BRIGHTNESS);
    FastLED.addLeds<LED_TYPE, LED_STRIPE_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    // currentPalette = RainbowColors_p;
    // currentBlending = LINEARBLEND;
#endif

    // try to connect to existing network
    WiFi.begin(ssid, password);
    Serial.print("\n\nTry to connect to existing network");

    {
        uint8_t timeout = 10;

        // Wait for connection, 5s timeout
        do
        {
            delay(500);
            Serial.print(".");
            timeout--;
        } while (timeout && WiFi.status() != WL_CONNECTED);
        Serial.print("Wifi status: ");
        Serial.println(WiFi.status());

        // not connected -> create hotspot
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("\n\nCreating hotspot");

            WiFi.mode(WIFI_AP);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
            WiFi.softAP(ssid);

            timeout = 5;

            do
            {
                delay(500);
                Serial.print(".");
                timeout--;
            } while (timeout);
        }
    }

    dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("\n\nWiFi parameters:");
    Serial.print("Mode: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
    Serial.print("IP address: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());

    // ESPUI Setup
    // statusLabelId = ESPUI.label("Status:", ControlColor::Turquoise, "Stop");
    millisLabelId = ESPUI.label("Millis:", ControlColor::Emerald, "0");
    heureLabelId = ESPUI.label("Heure", ControlColor::Emerald, "0");

    ESPUI.text("Text test call 1:", &test_textCall1, ControlColor::Alizarin, "a Text Field");
    ESPUI.text("Heure : ", &textSetAlarmTime, ControlColor::Alizarin, "another text");

    ESPUI.begin("ESPUI Alarm CLock des NICOs");
}
void loop()
{

    // Serial.println("Beginning the loop");
#ifdef LED_STRIPE_MODE

    for (int i = 0; i < NUM_LEDS; i++) // Turn on the LED one by one
    {
        leds[i] = CRGB::Blue;
        Serial.print("Turn i Red ON: ");
        Serial.println(i);
        FastLED.show();
        delay(DELAY_BETWEEN_LED_SECOND);
    }

    for (int i; i <= NUM_LEDS + 1; ++i) // Turn off the LED one by one
    {
        leds[i] = CRGB::Black;
        Serial.print("Turn i Red OFF: ");
        Serial.println(i);
        FastLED.show();
        delay(DELAY_BETWEEN_LED_SECOND);
    }
#endif

    dnsServer.processNextRequest();

    static long oldTime = 0;
    static bool testSwitchState = false;
    int heure = 0;
    if (millis() - oldTime > 5000)
    {
        ESPUI.print(millisLabelId, String(millis())); // show changing time for each loop.
        // std::string txt = std::to_string(42);
        ESPUI.print(heureLabelId, "22:00:01");

        // ESPUI.addGraphPoint(graphId, random(1, 50));

        testSwitchState = !testSwitchState;
        // ESPUI.updateSwitcher(testSwitchId, testSwitchState);

        oldTime = millis();
    }

    switchBuiltinLED();
    // delay(100);
}
