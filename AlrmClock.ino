/***
 * 
 * INI 4.4.2020 : ALARM CLOCK project wwww
 * 
 ***/

#define FASTLED_ESP8266_RAW_PIN_ORDER //Mandatory for NodeMCU ESP8266 correct PINOUT for LED_STRIPE_PIN below; you can remove the line for Board_ProMini
// #define FASTLED_ESP8266_NODEMCU_PIN_ORDER
// #define FASTLED_ESP8266_D1_PIN_ORDER

#define LED_BUILT_IN_MODE // Define what is tesed, built in blue LED or a connected stripe of leds,  possbile values: LED_STRIPE_MODE or LED_BUILT_IN_MODE

// Load library for Web server UI for better UX
// #include <ESPUI.h>
// Load Library for LED Stripe
#include <FastLED.h>
// Load Wi-Fi library
#include <ESP8266WiFi.h>
// 
#include "config.h"


/***
 * 
 * This is only used for testing
 * BuiltinLED settings
 * 
 ***/
unsigned long bilCurrentTime = millis(); // builtIn LED (bil) current time
unsigned long bilLasttime = 0;
int bilInterval = 500; // interval between blinking in milliseconds
bool bilState = HIGH;  // LED is HIGH or LOW
int bilLoop = 0;

/***
 * 
 * LED Stripe settings
 * 
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

/**
 * 
 * Web Server settings
 * 
 ***/

// Replace with your network credentials
const char *ssid = SSID; // 
const char *password = PWD; //

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

/**
 * 
 * FUNCTIONS
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

    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
    Serial.println("Exiting the setup");
    Serial.println("***");
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

    // Serial.println("Beginning the WiFi loop ");
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    {                                  // If a new client connects,
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        currentTime = millis();
        previousTime = currentTime;
        while (client.connected() && currentTime - previousTime <= timeoutTime)
        { // loop while the client's connected
            currentTime = millis();
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                header += c;
                if (c == '\n')
                { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        // turns the GPIOs on and off
                        if (header.indexOf("GET /5/on") >= 0)
                        {
                            Serial.println("GPIO 5 on");
                            output5State = "on";
                            // digitalWrite(output5, HIGH);
                        }
                        else if (header.indexOf("GET /5/off") >= 0)
                        {
                            Serial.println("GPIO 5 off");
                            output5State = "off";
                            // digitalWrite(output5, LOW);
                        }
                        else if (header.indexOf("GET /4/on") >= 0)
                        {
                            Serial.println("GPIO 4 on");
                            output4State = "on";
                            // digitalWrite(output4, HIGH);
                        }
                        else if (header.indexOf("GET /4/off") >= 0)
                        {
                            Serial.println("GPIO 4 off");
                            output4State = "off";
                            // digitalWrite(output4, LOW);
                        }

                        // Display the HTML web page
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS to style the on/off buttons
                        // Feel free to change the background-color and font-size attributes to fit your preferences
                        client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                        client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
                        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                        client.println(".button2 {background-color: #77878A;}</style></head>");

                        // Web Page Heading
                        client.println("<body><h1>ESP8266 Web Server</h1>");

                        // Display current state, and ON/OFF buttons for GPIO 5
                        client.println("<p>GPIO 5 - State " + output5State + "</p>");
                        // If the output5State is off, it displays the ON button
                        if (output5State == "off")
                        {
                            client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
                        }
                        else
                        {
                            client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
                        }

                        // Display current state, and ON/OFF buttons for GPIO 4
                        client.println("<p>GPIO 4 - State " + output4State + "</p>");
                        // If the output4State is off, it displays the ON button
                        if (output4State == "off")
                        {
                            client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
                        }
                        else
                        {
                            client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
                        }
                        client.println("</body></html>");

                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }

    switchBuiltinLED();
    // delay(100);
}
