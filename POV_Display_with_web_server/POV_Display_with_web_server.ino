/****************************************************************************************************************************
  For RP2040W with CYW43439 WiFi

  AsyncWebServer_RP2040W is a library for the RP2040W with CYW43439 WiFi

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncWebServer_RP2040W
  Licensed under GPLv3 license
 *****************************************************************************************************************************/

#if !(defined(ARDUINO_RASPBERRY_PI_PICO_W))
#error For RASPBERRY_PI_PICO_W only
#endif

#if defined __has_include
#if __has_include("credentials.h")
#include "credentials.h"
#else
// if you dont have a credentials.h file you may adjust the "secret Infos" inplace.
#define WIFI_SSID "secret Info"
#define WIFI_PASS "secret Info"
#endif
#endif

#define _RP2040W_AWS_LOGLEVEL_ 1

#include <Adafruit_NeoPixel.h>
#include <string.h>
#include <AsyncWebServer_RP2040W.h>
#include <AsyncFSEditor_RP2040W.h>
#include "pico/stdlib.h"

#define PIN_NEO_PIXEL 18             /* Raspberry Pi Pico W pin that connects to NeoPixel */
#define NUM_PIXELS 8                 /* The number of LEDs (pixels) on NeoPixel */
#define Hallsensor 16                /* Raspberry Pi Pico W pin to which the hallsensor is connected */
#define MAX_CONNECTIONS 2            /* Maximum number for connections allowed in the AP hotspot */
#define STATUS_CHECK_INTERVAL 10000L /* This constant is used in check_status method which is used for heartbeat print in serial monitor */

const char *ap_ssid = WIFI_SSID;           /* Access Point SSID */
const char *ap_password = WIFI_PASS;       /* Access Point Password */
uint8_t max_connections = MAX_CONNECTIONS; /* Maximum Connection Limit for AP */
unsigned long delayStart;                  /* Used to save the timer data used in loop() method */
bool delayRunning = false;                 /* Flag to be used in loop() method */

/* Configure IP Address */
IPAddress local_ip(192, 168, 4, 1); /* Stores the desired static IP address that needs to set to connect to the webserver */
IPAddress gateway(192, 168, 4, 1);  /* Stores the gateway IP that needs to be set to the webserver */
IPAddress subnet(255, 255, 255, 0); /* Stores the subnet mask that needs to be set to the webserver */

AsyncWebServer server(80); /* Create an aynchronous web server at port 80 */

/* Initialising the Neopixel library by providing details about the number of pixels, output pin for the Neopixels and type of LEDs used */
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

/* Stores the color values in rgb */
uint32_t e = NeoPixel.Color(0, 0, 0);                 /* no colour */
uint32_t colorToDisp = NeoPixel.Color(255, 255, 255); /* White color is set by default */

char receivedWords[14] = "POV Display";
char text_color[18];

/* String that contains the html code of the form that is being displayed on the webserver */
const String postForms =
    "<html>\
<head>\
<title>AsyncWebServer POST handling</title>\
<style>\
h1{}\
.inputForm{\
  background-color: #ffff66;\
  color: black;\
  border: 2px solid black;\
  margin: 20px;\
  padding: 20px;\
}\
.form-row{\
    padding: 10px 0;\
    display: flex;\
}\
.form-row label{\
    padding-right: 10px;\
}\
.form-row input{\
    flex:1;\
}\
fieldset input[type=\"submit\"] {\
    background-color: #d3dce0;\
    border: 1px solid #787878;\
    cursor: pointer;\
    font-size: 1.0em;\
    font-weight: 600;\
    padding: 7px 7px 7px 7px;\
    margin-top: 20px;\
    display : block;\
}\
</style>\
</head>\
<body style=\"background-color:#2a2f44; color:orange; font-family: Arial, Helvetica, sans-serif; align-items: center; justify-content: center; font-weight: normal; \">\
<h1><center>Persistence of Vision(PoV) web UI</center></h1><br>\
<center>\
<div class=\"inputForm\">\
<fieldset>\
<div class=\"form-row\">\
<form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
<label for=\"letters\">Enter the message to be displayed in the POV Display\(Minimum 3 and maximum 14 characters)</label><br>\
<input type=\"text\" name=\"letters\" required minlength=\"3\" maxlength=\"14\" size=\"30\" placeholder=\"Enter the message here.\(a-z or numbers)\" /><br/>\
<label for=\"color\">Choose a color</label><br>\
<select id=\"color\" name=\"color\">\
<option value=\"red\">Red</option>\
<option value=\"white\">White</option>\
<option value=\"green\">Green</option>\
<option value=\"blue\">Blue</option>\
<option value=\"purple\">Purple</option>\
<option value=\"yellow\">Yellow</option>\
<option value=\"cyan\">Cyan</option>\
<option value=\"blue_violet\">Blue Violet</option>\
<option value=\"sky_blue\">Sky Blue</option>\
<option value=\"aquamarine\">Aquamarine</option>\
<option value=\"navy\">Navy</option>\
<br/><input type=\"submit\" value=\"Submit\">\
</div>\
</fieldset>\
</form>\
</div>\
</center>\
</body>\
</html>";

void setup()
{
  /* Start the serial monitor output */
  Serial.begin(115200);

  /* It waits for 5000ms if the serial monitor output has not been started */
  while (!Serial && millis() < 5000)
    ;

  /* Set a delay of 200 milliseconds before moving ahead with the execution */
  delay(200);

  /* Check for the WiFi module */
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");

    /* Wait until the WiFi module is available */
    while (true)
      ;
  }

  /* Provide the configurations to start the Server in Access Point mode with the provided IP, Gateway and Subnet */
  WiFi.softAPConfig(local_ip, gateway, subnet);

  /* Delay for 0.5 seconds before checking if the AP server has started */
  delay(500);

  /* Setting the AP Mode with SSID, Password, and Max Connection Limit. Instantiate the starting of the server with the provided credentials and configurations */
  if (WiFi.softAP(ap_ssid, ap_password, 1, false, max_connections) == true)
  {
    /* Print the SSID of the network in the Serial Monitor */
    Serial.print("Access Point is Created with SSID: ");
    Serial.println(ap_ssid);

    /* Print the maximum connections that are allowed to join the wifi network. You can change this count by altering the macro MAX_CONNECTIONS */
    Serial.print("Max Connections Allowed: ");
    Serial.println(max_connections);
  }
  else
  {
    /* If the access point connection fails, the failure message is printed in the serial monitor */
    Serial.println("Unable to create Access Point");
  }

  /* It sets up a route for HTTP GET requests to the root URL ("/") */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    /* The lambda function "{...}" calls handleRoot(request) when a GET request is received at the root URL */
    handleRoot(request); });

  /* Sets up a route for HTTP POST requests to the URL "/postform/" (URI, web request method and request handler function is sent as parameters) */
  server.on("/postform/", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    /* The lambda function calls handlePostForm(request) when a POST request is received at this URL */
    handleForm(request); });

  server.onNotFound(handleNotFound);

  server.begin();
  /* IP address of the Async server is displayed in the serial monitor */
  Serial.print(F("HTTP Async_PostServer started @ IP : "));
  Serial.println(WiFi.localIP());

  /* Initialize NeoPixel strip object */
  NeoPixel.begin();
  NeoPixel.setBrightness(255);
  NeoPixel.clear();
  NeoPixel.show();
  /* Configures the specified pin to behave either as an input or an output. Here we configure the hall sensor which is connected to PIN 16 to work as an input */
  pinMode(Hallsensor, INPUT_PULLUP);
}

/* Send the post request to the webserver with the html form which is embedded inside the string 'postForms' */
void handleRoot(AsyncWebServerRequest *request)
{
  /* send method in class AsyncWebServerRequest is invoked which takes code(status code), contentType and content(html code) as inputs */
  request->send(200, "text/html", postForms);
}

/* This function will receive the data obtained from the form through HTTP POST method and pass it to the variables. The variables will store the text to be displayed in the POV display and the color of the text */
void handleForm(AsyncWebServerRequest *request)
{
  if (request->method() != HTTP_POST)
  {
    /* If the received request is not a HTTP POST request, method not supported error is thrown */
    request->send(405, "text/plain", "Method Not Allowed");
  }
  else
  {
    /* Message to be displayed after the form is submitted */
    String message = "The message that will be displayed in the POV Display is: ";

    for (uint8_t i = 0; i < request->args(); i++)
    {
      /* Parse the obtained argument from the submitted form and check the argument name "letters" which contains the text to be displayed on the POV display */
      if (strcmp(request->argName(i).c_str(), "letters") == 0)
      {
        /* Store the text which is obtained from the Text Box in the post form into the variable 'receivedWords' and also add it to the string 'message' which will be dispayed in the new page */
        message += " " + request->arg(i) + "\n";
        strcpy(receivedWords, request->arg(i).c_str());
      }
      /* Parse the obtained argument from the submitted form and check the argument name "color" which contains the color of the text to be displayed */
      else if (strcmp(request->argName(i).c_str(), "color") == 0)
      {
        /* Store the color value obtained in variable 'text_color' and then update the color value based on the input obtained from the form to a variable of type uint32_t named 'colorToDisp' */
        message += "The Color of the message is: " + request->arg(i) + "\n";
        strcpy(text_color, request->arg(i).c_str());
        if (strcmp(text_color, "white") == 0)
        {
          colorToDisp = NeoPixel.Color(255, 255, 255);
        }
        else if (strcmp(text_color, "red") == 0)
        {
          colorToDisp = NeoPixel.Color(255, 0, 0);
        }
        else if (strcmp(text_color, "green") == 0)
        {
          colorToDisp = NeoPixel.Color(0, 255, 0);
        }
        else if (strcmp(text_color, "blue") == 0)
        {
          colorToDisp = NeoPixel.Color(0, 0, 255);
        }
        else if (strcmp(text_color, "purple") == 0)
        {
          colorToDisp = NeoPixel.Color(255, 0, 255);
        }
        else if (strcmp(text_color, "yellow") == 0)
        {
          colorToDisp = NeoPixel.Color(255, 255, 0);
        }
        else if (strcmp(text_color, "cyan") == 0)
        {
          colorToDisp = NeoPixel.Color(0, 255, 255);
        }
        else if (strcmp(text_color, "blue_violet") == 0)
        {
          colorToDisp = NeoPixel.Color(138, 43, 226);
        }
        else if (strcmp(text_color, "sky_blue") == 0)
        {
          colorToDisp = NeoPixel.Color(0, 191, 255);
        }
        else if (strcmp(text_color, "aquamarine") == 0)
        {
          colorToDisp = NeoPixel.Color(127, 255, 212);
        }
        else if (strcmp(text_color, "navy") == 0)
        {
          colorToDisp = NeoPixel.Color(0, 0, 128);
        }
      }
    }
    /* Send the message to the landing page after form submission */
    request->send(200, "text/plain", message);
  }
}

/* This function is executed if the AsyncWebServer cannot find a matching route for the request */
void handleNotFound(AsyncWebServerRequest *request)
{
  /* Initialize a message string to store the error details */
  String message = "File Not Found\n\n";

  /* Append the requested URL to the message */
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";

  /* Append the HTTP method (GET or POST) to the message */
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";

  /* Append the number of arguments in the request to the message */
  message += request->args();
  message += "\n";

  /* Loop through all arguments and append their names and values to the message */
  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  /* Send a 404 response with the constructed message as plain text */
  request->send(404, "text/plain", message);
}

void loop()
{
  check_status();
  /* If else statement is used to check if the hallsensor has been detected once */
  /* It is to avoid the output being trigerred twice because of hall sensor being activated twice(Once when it enters the magnetic field and the second time when it leaves the magnetic field). */
  if (digitalRead(Hallsensor) == LOW && delayRunning == false)
  {
    /* This function call triggers the lighting of the LEDs for a single rotation */
    writeChars(colorToDisp, receivedWords);

    /* Flag to ignore the input from the hall sensor until a rotation of the POV display is complete */
    delayRunning = true;

    /* This variable is used to store the time when the hallsensor detects a change in voltage due to a magnetic field */
    delayStart = millis();
  }
  /* This condition detects if the hallsensor detects a change in voltage due to a magnetic field and the minimum time delay is more than 19 milliseconds since the last detection. This check is provided to verify if one rotation is completed by the POV display */
  /* It also checks if delayRunning flag is set as true */
  else if (digitalRead(Hallsensor) == LOW && delayRunning == true && ((millis() - delayStart) >= 19))
  {
    /* Update the delayRunning flag to false so that the if condition can be evaluated to true */
    delayRunning = false;
  }
}

void check_status()
{
  static unsigned long checkstatus_timeout = 0;

  /* Send status report every 10 seconds(As macro STATUS_CHECK_INTERVAL is set to 10000ms in the beginning of this file). We don't need to send updates frequently if there is no status change */
  if ((millis() >= checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    /* The value of checkstatus_timeout is icremented by 10 seconds so that heartBeatPrint() is called every 10 seconds */
    checkstatus_timeout = millis() + STATUS_CHECK_INTERVAL;
  }
}

/* Function to display the heartbeat print or status indicator in Serial Monitor in Arduino IDE in the form of dots(.......... ..........) */
void heartBeatPrint()
{
  static int num = 1;
  /* Dot is printed every 10 seconds and after every 100 seconds, space is printed. After every 800 seconds(800000 milliseconds), the value of num is reset to 1 and the cursor is moved to next line in the serial monitor */
  Serial.print(F("."));
  /* The code waits until the number is incremented in else condition */
  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  /* Increment using postfix increment and print space after 10 dots */
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

/* This function takes the text and color provided in the webserver form and uses the switch statement to display the characters of the input text on the POV display */
void writeChars(uint32_t wordColor, char receivedWords[])
{
  /* Run the for loop based on the length of char array receivedWords */
  for (int i = strlen(receivedWords) - 1; i >= 0; i--)
  {
    switch (receivedWords[i])
    {
    /* Each character is compared with the a character in switch case and the function to display that character in the POV display is called  */
    case 'A':
    case 'a':
      drawA(wordColor);
      break;
    case 'B':
    case 'b':
      drawB(wordColor);
      break;
    case 'C':
    case 'c':
      drawC(wordColor);
      break;
    case 'D':
    case 'd':
      drawD(wordColor);
      break;
    case 'E':
    case 'e':
      drawE(wordColor);
      break;
    case 'F':
    case 'f':
      drawF(wordColor);
      break;
    case 'G':
    case 'g':
      drawG(wordColor);
      break;
    case 'H':
    case 'h':
      drawH(wordColor);
      break;
    case 'I':
    case 'i':
      drawI(wordColor);
      break;
    case 'J':
    case 'j':
      drawJ(wordColor);
      break;
    case 'K':
    case 'k':
      drawK(wordColor);
      break;
    case 'L':
    case 'l':
      drawL(wordColor);
      break;
    case 'M':
    case 'm':
      drawM(wordColor);
      break;
    case 'N':
    case 'n':
      drawN(wordColor);
      break;
    case 'O':
    case 'o':
      drawO(wordColor);
      break;
    case 'P':
    case 'p':
      drawP(wordColor);
      break;
    case 'Q':
    case 'q':
      drawQ(wordColor);
      break;
    case 'R':
    case 'r':
      drawR(wordColor);
      break;
    case 'S':
    case 's':
      drawS(wordColor);
      break;
    case 'T':
    case 't':
      drawT(wordColor);
      break;
    case 'U':
    case 'u':
      drawU(wordColor);
      break;
    case 'V':
    case 'v':
      drawV(wordColor);
      break;
    case 'W':
    case 'w':
      drawW(wordColor);
      break;
    case 'X':
    case 'x':
      drawX(wordColor);
      break;
    case 'Y':
    case 'y':
      drawY(wordColor);
      break;
    case 'Z':
    case 'z':
      drawZ(wordColor);
      break;
    case '0':
      draw0(wordColor);
      break;
    case '1':
      draw1(wordColor);
      break;
    case '2':
      draw2(wordColor);
      break;
    case '3':
      draw3(wordColor);
      break;
    case '4':
      draw4(wordColor);
      break;
    case '5':
      draw5(wordColor);
      break;
    case '6':
      draw6(wordColor);
      break;
    case '7':
      draw7(wordColor);
      break;
    case '8':
      draw8(wordColor);
      break;
    case '9':
      draw9(wordColor);
      break;
    case ' ':
      drawEmpty(e);
      drawEmpty(e);
      break;
    case '!':
      drawExclamation(wordColor);
      break;
    case '?':
      drawQuestionMark(wordColor);
      break;
    case ':':
      drawColon(wordColor);
      break;
    case ')':
      drawRightParentheses(wordColor);
      break;
    case '(':
      drawLeftParentheses(wordColor);
      break;
    }
  }
}

/* The below functions are used to control the Neopixel LEDs to display the different characters */
/* Here we need to provide inputs to light up the LEDs so that they display the different characters */
/* The functionality of how the LEDs are programmed and turned ON/OFF in explained for character 'Z' in the function drawZ(uint32_t letterColor) */
void drawA(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 1, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawB(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawC(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawD(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawE(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawF(uint32_t letterColor)
{
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawG(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 3);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawH(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawI(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawJ(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 2);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawK(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawL(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawM(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawN(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawO(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawP(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 4, 3);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawQ(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 1, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 2);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawR(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawS(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawT(uint32_t letterColor)
{
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawU(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawV(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 3, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 3, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawW(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 3);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 6);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawX(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 1, 3);
  NeoPixel.fill(letterColor, 5, 3);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 3);
  NeoPixel.fill(letterColor, 5, 3);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawY(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 5, 3);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 4);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 5, 3);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}

void drawZ(uint32_t letterColor)
{
  /* Set a pixel's color using a 32-bit 'packed' RGB or RGBW value. n Pixel index, starting from 0(the first parameter) */
  /* 8 NeoPixel LEDs are present */
  /*     12345 */
  /* 7 - ..... */
  /* 6 -     . */
  /* 5 -    .  */
  /* 4 -   .   */
  /* 3 -  .    */
  /* 2 - .     */
  /* 1 - ..... */
  /* 0 -       */
  /* The lighting of the LEDs is programmed from right to left as the POV display spins in the anti-clockwise direction */
  /* The first set of commands below is to display the PIXELs at column 5 as shown above */
  /* LED at index 1 should be set to ON */
  NeoPixel.setPixelColor(1, letterColor);
  /* LEDs at index 6 and 7 should be set to ON */
  NeoPixel.fill(letterColor, 6, 2);
  /* Transmit pixel data in RAM to NeoPixels. The LEDs at index 1, 6 and 7 light up together when show() function is called */
  NeoPixel.show();
  /* Once the LEDs are turned ON/OFF as required, the data from RAM is cleared to display the next column of the character(Column 5) */
  /* Fill the whole NeoPixel strip with 0 / black / off */
  NeoPixel.clear();
  /* LED at index 1 should be set to ON */
  NeoPixel.setPixelColor(1, letterColor);
  /* LED at index 5 should be set to ON */
  NeoPixel.setPixelColor(5, letterColor);
  /* LED at index 7 should be set to ON */
  NeoPixel.setPixelColor(7, letterColor);
  /* The LEDs at index 1, 5 and 7 light up together when show() function is called */
  NeoPixel.show();
  /* Once the LEDs are turned ON/OFF as required, the data from RAM is cleared to display the next column of the character(Column 4) */
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 2);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw0(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw1(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw2(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw3(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw4(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 1, 7);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw5(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.fill(letterColor, 4, 4);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw6(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw7(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 5, 3);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw8(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void draw9(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(5, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void drawExclamation(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.fill(letterColor, 3, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void drawQuestionMark(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 5, 2);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(4, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(3, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 5, 2);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void drawColon(uint32_t letterColor)
{
  NeoPixel.setPixelColor(2, letterColor);
  NeoPixel.setPixelColor(6, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void drawRightParentheses(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void drawLeftParentheses(uint32_t letterColor)
{
  NeoPixel.setPixelColor(1, letterColor);
  NeoPixel.setPixelColor(7, letterColor);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.fill(letterColor, 2, 5);
  NeoPixel.show();
  NeoPixel.clear();
  NeoPixel.show();
}
void drawEmpty(uint32_t letterColor)
{
  NeoPixel.fill(letterColor, 0, 7);
  NeoPixel.show();
}
