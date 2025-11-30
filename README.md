# POV-Display-Arduino-RPi-PicoW
A Persistence of Vision (POV) display using Arduino code, Raspberry Pi Pico W, and Neopixel LEDs.

# POV Display (Persistence of Vision)

This project demonstrates a Persistence of Vision (POV) display using Arduino and C++ with a Raspberry Pi Pico W and Neopixel LEDs, rapidly flashing in sequence to create the illusion of continuous images. The system also hosts a webserver on the Raspberry Pi in Access Point mode, serving an HTML webpage when the IP is accessed, where users can input custom text to be displayed. The repository contains all the necessary code and documentation to recreate the project.

## Features
- Arduino code for controlling the POV display
- Integration with Raspberry Pi Pico W
- Neopixel LED control
- Detailed setup instructions and schematics

## Getting Started

### Prerequisites
- Arduino IDE
- Raspberry Pi Pico W
- Neopixel LEDs
- Adafruit Neopixel library
- Raspberry Pi Pico SDK

### Installation
1. **Clone the repository:**
   \```sh
   git clone https://github.com/gauravshetty90/POV-Display-Arduino-RPi-PicoW.git
   cd POV-Display-Arduino-RPi-PicoW
2. Install Arduino IDE and the board library:
    - Open Arduino IDE and go to File > Preferences.
    - In the Additional Board Manager URLs field, provide the link: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    - Go to Tools > Board > Boards Manager.
    - Scroll down until you find Raspberry Pi Pico/RP2040 and click on Install.
    - Wait for the installation to complete.
    - Change the board type by going to Tools > Board > Boards Manager and selecting Raspberry Pi Pico W.

3. Set up the required libraries and webserver SSID and password:
    - Open Arduino IDE and go to Tools > Manage Libraries.
    - Search for and install the following libraries:
      - Adafruit_NeoPixel
      - AsyncWebServer_RP2040W
    - Add the Access Point SSID and Password of your choice in the placeholders present in credentials.h.
    - Click on Sketch > Include Library to include the libraries (if you need to add extra libraries).
4. Compile the code:
    - Select the correct board and port.
    - Upload the code to the Raspberry Pi Pico W.
5. Connect to the webserver:
    - The Raspberry Pi Pico W should create an Access Point network.
    - Set the serial monitor port to 115200 to see the connection status and details once the code is uploaded.
    - Use the SSID and password to connect to the webserver through the URL: http://192.168.4.1.
  
# Working of characters in the POV Display
The LED lights up from right to left as the motor rotates it in the anti-clockwise direction:
There are 8 LEDs numbered from '0' to '7' according to the Neopixel library.
As per the code the LEDs have been programmed starting with the index '3'.
Explanation of how 'R' is displayed according to the code.
'R' is split into 4 segments
'letterColor' is a paramter which specifies the colour that needs to be displayed as Neopixels support RGB.
The colours are initialised in the beginning of the program. I have passed green colour for the display message.

uint32_t g = NeoPixel.Color(0, 255, 0); //Grün

writeChars(g,receivedWords);

Setting the LEDs:
//To Light up a single LED.
NeoPixel.setPixelColor(LED index, Setting a colour for the LED);
//To light up multiple LEDs which are adjacent to each other.
NeoPixel.fill( Setting a colour for the LED, Index of the starting LED which needs to be lit, Number of LEDs that need to be lit from the starting index);

	

    
   NeoPixel.setPixelColor(3, letterColor);
    NeoPixel.setPixelColor(6, letterColor);
    NeoPixel.show();
    NeoPixel.clear();
	The 3rd and the 6th index LEDs are set to light up.
	
NeoPixel.fill(letterColor, 4, 2);
	NeoPixel.setPixelColor(7, letterColor);
	NeoPixel.show();
	NeoPixel.clear();
	
The 4th and the 5th LEDs are set to light up in the first line and 7th LED is set to light up in the 2nd line.
	
NeoPixel.setPixelColor(5, letterColor);
	NeoPixel.setPixelColor(7, letterColor);
	NeoPixel.show();
	NeoPixel.clear();
	
The 5th and 7th LEDs are set to light up.
	
NeoPixel.fill(letterColor, 3, 5);
	NeoPixel.show();
	NeoPixel.clear();
	NeoPixel.show();
	
The 3rd, 4th, 5th, 6th, 7th LEDs are set to light up.
	
Here is how the LEDs actually light up from right to left
Every column is a segment as numbered below in the order of lighting up.
	
7    * * *       
6    *     *       
5    * * *       
4    *   *       
3    *     *
2                          
1                         
0                          
	  4 3 2 1
<img width="712" height="1046" alt="image" src="https://github.com/user-attachments/assets/4c71f1df-e78d-4b41-a62e-ff478262c8af" />

  
# Contributing
Contributions are welcome! Please fork this repository and submit pull requests.

# License
This project is licensed under the MIT License - see the LICENSE file for details.

# Acknowledgments
Thanks to the open-source community for the libraries and tools used in this project.

# Working Images
![The default landing page](https://github.com/user-attachments/assets/22165740-62c9-4a14-a32b-65dc928cfb88)

![Text and color selection](https://github.com/user-attachments/assets/046c34c9-8f90-4566-833f-df70439f77c0)


