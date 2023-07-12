# Setup and Flashing

In order to flash chanduino to your ESP32 TTGO T-Display board, you will need a USB-C cable and a computer with the [Arduino IDE](https://www.arduino.cc/en/software) installed.

## Flashing

1. Open the Arduino IDE and plug in your device via a USB-C cable
1. Go to the menu and select Tools->Board->Boards Manager...
1. Install the esp32 module, which provides support for the "ESP32 Dev Module". Use the latest available version
1. Now go to the menu and select Tools->Manage Libraries...
1. Install the libraries listed under the [Dependencies section of the README](https://github.com/rebane2001/chanduino/blob/master/README.md#dependencies)
1. While not always required, you may need to select the correct port under the Tools->Port menu
1. Now for the one slightly involved bit. We need to tell the TFT_eSPI library which display we have
   - Open your Arduino sketchbook folder (you can find it by going to the menu File->Preferences. It will be under "Sketchbook location")
   - Open this folder and edit the file at libraries/TFT_eSPI/User_Setup_Select.h
   - Change the line:
 
   ```
   #include <User_Setup.h>
   ```
 
   to:
 
   ```
   //#include <User_Setup.h>
   ```
 
   - (either) Change the other line (for original T-Display):
 
   ```
   //#include <User_Setups/Setup25_TTGO_T_Display.h>
   ```
 
   to:
 
   ```
   #include <User_Setups/Setup25_TTGO_T_Display.h>
   ```

   - (or) Change the other line (for T-Display-S3):
 
   ```
   //#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>
   ```
 
   to:
 
   ```
   #include <User_Setups/Setup206_LilyGo_T_Display_S3.h>
   ```

1. Great! Now we just need to open, compile and upload the chanduino code to the board.

   Download the code and open the [chanduino/chanduino.ino](chanduino/chanduino.ino) file in Arduino IDE
1. Click the little right-pointing arrow at the top left to Upload the code

Once that is finished, your board should reboot and Chanduino should be installed. Follow the instructions on the screen to use!

For some customization, be sure to check the [README](README.md) for themes and configuration options that can be changed. These options are located in the chanduino.ino file, and the code must be re-uploaded to apply changes.

## Resetting the board

If you want to reset the board (necessary to reset chanduino WiFi preferences) you'll need to erase the flash and re-Upload the program.

1. Download and install [esptool](https://github.com/espressif/esptool)
1. Plug in your board and run `esptool.py erase_flash` in a terminal window
1. Follow the instructions under "Flashing" to flash your board again
