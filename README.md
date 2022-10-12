# Chanduino
Chanduino is a standalone unofficial 4chan browser for the ESP32 (TTGO T-Display). It started off as a project to practice embedded development and was made as a present for the /mlp/ Secret Santa, but 9 months later I decided to fix it up (added HTTPS support since 4chan is HTTPS-only now) and release it.

## [Demo video](https://www.youtube.com/watch?v=RlFtYx4oX9U)

[![banner](banner.jpg?raw=true)](https://www.youtube.com/watch?v=RlFtYx4oX9U)

# Features
- Browse any board
- Browse both threads and replies
- Zoom into images (fullscreen)
- Compact UI styled after Yotsuba/Yotsuba B
- Multi-screen posts (if there is too much text for one screen)
- Screensaver (turn off screen after inactivity)
- Threadwatcher (turn on screen on new posts)
- Customizable themes
- HTTPS and keep-alive support
- Progress bar
- Doesn't let you respond to bait
- Doesn't crash most of the time

![UI demo](yotsuba.jpg?raw=true)

# Usage
On first launch or if no WiFi is found, Chanduino will create a WiFi hotspot which you can use to set up a WiFi connection for it. The SSID/password will be saved into flash and Chanduino will boot straight into the main menu next time.

Just pressing up/down buttons is self-explanatory. Holding down the up button lets you go upwards (from thread to board to boards selection). It also lets you navigate the boards selection faster. Holding the down button does the opposite (goes from boards selectin to board to thread) and also lets you view images in threads in fullscreen.

If you don't press any buttons for some time, Chanduino will turn off the screen (this can be disabled in the configuration section). If you load a thread and don't touch Chanduino for a while, it'll check for new posts from time to time and wake up the screen if new posts are found.

# Flashing and Setup
Please see [SETUP.md](SETUP.md) for instructions on how to flash the code and reset the board.

# Configuration
You can change some of the settings through editing `#define`s in `chanduino.ino`:  
 - CHANDUINO_SCREENSAVER_ENABLED 0/1
   - Turn screensaver on/off
 - CHANDUINO_SCREENSAVER_TIME 45
   - Change how long until screen turns off (seconds)
 - CHANDUINO_THREADWATCHER_ENABLED 0/1
   - Turn threadwatcher on/off
 - CHANDUINO_THREADWATCHER_TIME 60
   - Change how often threadwatcher should check for new posts (seconds)
 - CHANDUINO_DEFAULTBOARD "/replaceme/"
   - Auto-select a board of your choice on startup
 - CHANDUINO_HIDE_PROGRESS_BAR 0/1
   - Turn progrss bar on/off

# Themes
You can edit the colors used in the `THEME` section of `chanduino.ino`, similar to how you change the other settings. 

Premade themes are available in the `themes` folder and a theme generator is available in `extra/theme_generator.html`.

# Dependencies
- [ArduinoJson 6.16.1](https://arduinojson.org/)
- [TFT_eSPI 1.4.20](https://github.com/Bodmer/TFT_eSPI)
- [TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder)
- [Button2](https://github.com/LennartHennigs/Button2)

You can flash this project with the Arudino IDE.

# Disclaimer
I started this project with pretty much no experience in both C++ and embedded development. Thus, the code quality and memory management is rather poor and this project should not be used as a reference for learning. You can look at it for keks though.
