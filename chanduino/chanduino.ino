#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include "WiFiUdp.h"
#include "WiFiClientSecure.h"
#include "HTTPClient.h"
#include <Wire.h>
#include <Button2.h>
#include "esp_adc_cal.h"
#include "EEPROM.h"

#include <TJpg_Decoder.h>

/**
 * SETTINGS
 */
// Set ENABLED to 1 to turn backlight off after TIME seconds of inactivity (no new posts and no button presses)
#define CHANDUINO_SCREENSAVER_ENABLED 1
#define CHANDUINO_SCREENSAVER_TIME 45
// Set to 1 to enable threadwatcher
// Set ENABLED to 1 to watch thread every TIME seconds after last buttonpress (DO NOT set under 10, recommended values are 30-120)
#define CHANDUINO_THREADWATCHER_ENABLED 1
#define CHANDUINO_THREADWATCHER_TIME 60
// Change this to your favorite board to have it auto-selected
#define CHANDUINO_DEFAULTBOARD "/replaceme/"
// Set to 1 to hide progress bar
#define CHANDUINO_HIDE_PROGRESS_BAR 0

/**
 * THEME
 */
#define CHANDUINO_THEME_BOARDSELECT_BACKGROUND 0x2104
#define CHANDUINO_THEME_BOARDSELECT_SELECTION 0x0000
#define CHANDUINO_THEME_BOARDSELECT_P 0xF81F
#define CHANDUINO_THEME_BOARDSELECT_WS 0x554A
#define CHANDUINO_THEME_BOARDSELECT_NSFW 0xDAAA
#define CHANDUINO_THEME_BOARD_BACKGROUND_WS 0xD6DE
#define CHANDUINO_THEME_BOARD_BACKGROUND_NSFW 0xF71A
#define CHANDUINO_THEME_POST_TITLE 0x086B
#define CHANDUINO_THEME_POST_NAME 0x13A8
#define CHANDUINO_THEME_POST_FILE 0x31AB
#define CHANDUINO_THEME_POST_TEXT 0x0000
#define CHANDUINO_THEME_POST_YOU 0xD800
#define CHANDUINO_THEME_POST_GREEN 0x7CC4
#define CHANDUINO_THEME_POST_SPOILER_FG 0xFFFF
#define CHANDUINO_THEME_POST_SPOILER_BG 0x0000
#define CHANDUINO_THEME_THREADWATCHER_PRIMARY 0xDAAA
#define CHANDUINO_THEME_THREADWATCHER_SECONDARY 0x0000
#define CHANDUINO_THEME_LOADING 0xD800




#define CHANDUINO_VERSION "0.8"

#define ADC_EN          14
#define ADC_PIN         34
#define BUTTON_1        35
#define BUTTON_2        0

TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

// 4chan root certificate (Baltimore CyberTrust Root)
const char *root_ca = PSTR( \
                            "-----BEGIN CERTIFICATE-----\n" \
                            "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
                            "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
                            "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
                            "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
                            "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
                            "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
                            "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
                            "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
                            "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
                            "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
                            "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
                            "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
                            "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
                            "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
                            "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
                            "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
                            "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
                            "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
                            "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
                            "-----END CERTIFICATE-----\n");

// Web client settings
const int httpPort = 443;
const char *host = "a.4cdn.org";
const String useragent = "Chanduino/" + String(CHANDUINO_VERSION);

// Wifi config webserver variables
WiFiServer server(80);
String header;

// How long to hold down button for secondary action
const int buttonDelay = 350;

// Current board
String board = "";
// Current thread
int thread = 0;
// If a post is too long, split it into multiple "pages" (-1 = no split)
int multiPage = -1;

char buff[32768]; //32K

int replies[2001];
int currentreply = 0;
int maxreply = 0;

// Hack for keeping post location
int posts[2001];
int savedpost = 0;
int maxposts = 0;

// For loading progress bar
int boardPostCount = 0;
int postReplyCount = 0;

String tim = "";

// Current board settings
int bgcolor = CHANDUINO_THEME_BOARDSELECT_BACKGROUND;

/**
 * viewMode:
 * 1 - Browse replies in thread
 * 2 - Browse threads in board
 * 3 - Browse boards
 */
int viewMode = 3;
int wifiMode = 0;

// Threadwatcher stuff
// Timestamp of last buttonpress
int64_t lastBtnPress = 0;
// Timestamp of last threadrefresh
int64_t lastRefresh = 0;
int lastReadReply = 0;
int newPostCount = 0;
bool seenAllNewPosts = true;
int oldReplies[2001];

// Chunked encoding can suck my [4chan pass]

// Board list cache
std::vector<String> boards_nm;
std::vector<String> boards_ds;
std::vector<bool> boards_ws;
std::vector<int> boards_ct;

// For loading images
//uint8_t PicArray[15000] = {0};

WiFiClientSecure client;

/* Chan stuff */

/**
 * Handles buttonpresses and navigation.
 */
void button_init() {
  // When button down, activate backlight
  btn1.setPressedHandler([](Button2 & b) { updateLastTimes(); });
  btn2.setPressedHandler([](Button2 & b) { updateLastTimes(); });

  // UP button
  btn1.setReleasedHandler([](Button2 & b) {
    if (wifiMode == 0) {
      if (connect_wifi()) {
        unsigned int time = b.wasPressedFor();
        if (time < buttonDelay) {
          if (viewMode == 1 || viewMode == 2 || viewMode == 3) {
            if (multiPage < 1 || viewMode != 1) {
              multiPage = -1;
              currentreply--;
            } else {
              multiPage--;
            }
            if (currentreply < 0) {
              currentreply = maxreply;
            }
            if (viewMode == 3) {
              show_boards();
            } else {
              load_reply();
            }
          }
        } else {
          multiPage = -1;
          if (viewMode == 1) {
            viewMode = 2;
            restorePosts();
            load_reply();
          } else if (viewMode == 2) {
            viewMode = 3;
            bgcolor = CHANDUINO_THEME_BOARDSELECT_BACKGROUND;
            currentreply = 0;
            maxreply = -2;
            show_boards();
          } else if (viewMode == 3) {
            currentreply += 12;
            if (currentreply > maxreply) {
              currentreply = 0;
            }
            show_boards();
          }
        }
      }
    } else {
      wifiMode = 0;
      WiFi.mode(WIFI_STA);
      if (connect_wifi()) {
        draw_connection_restored();
        return;
      }
    }
    updateLastTimes();
  });

  // DOWN button
  btn2.setReleasedHandler([](Button2 & b) {
    if (wifiMode == 0) {
      if (connect_wifi()) {
        unsigned int time = b.wasPressedFor();
        if (time < buttonDelay) {
          if (viewMode == 1 || viewMode == 2 || viewMode == 3) {
            if (multiPage == -1 || viewMode != 1) {
              currentreply++;
            } else {
              multiPage++;
            }
            if (currentreply > maxreply) {
              currentreply = 0;
            }
            if (viewMode == 3) {
              show_boards();
            } else {
              load_reply();
            }
          }
        } else {
          multiPage = -1;
          if (viewMode == 1) {
            draw_img(1);
          } else if (viewMode == 2) {
            newPostCount = 0;
            seenAllNewPosts = true;
            thread = replies[currentreply];
            savePosts();
            viewMode = 1;
            draw_loading_text();
            load_posts();
            load_reply();
          } else if (viewMode == 3) {
            load_board();
            viewMode = 2;
            Serial.println(board);
            draw_loading_text();
            load_posts();
            load_reply();
          }
        }
      }
    } else {
      wifiMode = 0;
      WiFi.mode(WIFI_STA);
      if (connect_wifi()) {
        draw_connection_restored();
        return;
      }
    }
    updateLastTimes();
  });
}

/**
 * Connects to 4chan's api.
 */
int connectToa4cdn() {
  // Flush everything already in
  while (client.available()) {
    // Strange hack because .flush() doesn't work as expected
    client.readStringUntil('\n');
    if(!client.available())
      delay(50);
  }
  if (!client.connected()) {
    if (!client.connect(host, httpPort)) {
      Serial.println("Connection failed");
      return 0;
    }
  }
  return 1;
}

/**
 * Enters a board.
 */
void load_board() {
  if (boards_ws[currentreply]) {
    bgcolor = CHANDUINO_THEME_BOARD_BACKGROUND_WS;
  } else {
    bgcolor = CHANDUINO_THEME_BOARD_BACKGROUND_NSFW;
  }
  board = boards_nm[currentreply];
  boardPostCount = boards_ct[currentreply];
}

/**
 * Loads list of boards.
 */
void load_boards() {
  DynamicJsonDocument jsonDoc(1024 * 16);
  HTTPClient http;

  Serial.print("\r\njsonDoc capacity: ");
  Serial.println(jsonDoc.capacity());

  Serial.print("\r\nConnecting to ");
  Serial.println(host);

  http.useHTTP10(true);
  http.begin("https://a.4cdn.org/boards.json", root_ca);
  int httpCode = http.GET();

  // https://arduinojson.org/v6/assistant/
  StaticJsonDocument<128> filter;
  filter["boards"][0]["ws_board"] = true;
  filter["boards"][0]["meta_description"] = true;
  filter["boards"][0]["board"] = true;
  filter["boards"][0]["per_page"] = true;
  filter["boards"][0]["pages"] = true;

  DeserializationError err = deserializeJson(jsonDoc, http.getStream(), DeserializationOption::Filter(filter));
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }

  int i = 0;
  String desc = "";
  for (i = 0; i < jsonDoc["boards"].size(); i++) {
    desc = jsonDoc["boards"][i]["meta_description"].as<String>();
    desc.replace("&quot;", "'");
    desc.replace("&amp;", "&");
    desc.replace("\\\/", "\/");
    desc.replace("is 4chan's board for", "-");
    desc.replace("is 4chan's imageboard dedicated to the discussion of My Little Pony: Friendship is Magic.", "Ponies!");
    desc.replace("is 4chan's imageboard for", "-");
    desc.replace("is 4chan's imageboard dedicated to the discussion of", "-");
    desc.replace("is 4chan's imageboard dedicated to", "-");
    desc.replace("is 4chan's imageboard", "-");
    boards_ds.push_back(desc);
    boards_nm.push_back(jsonDoc["boards"][i]["board"].as<String>());
    boards_ws.push_back(jsonDoc["boards"][i]["ws_board"].as<int>());
    int total_posts = jsonDoc["boards"][i]["per_page"].as<int>() * jsonDoc["boards"][i]["pages"].as<int>();
    boards_ct.push_back(total_posts);
    if (desc.indexOf(CHANDUINO_DEFAULTBOARD) > 0)
      currentreply = i;
  }
  Serial.println("END");
}

/**
 * Shows list of boards.
 */
void show_boards() {
  tft.fillScreen(bgcolor);
  draw_reply_number();

  if (boards_ws.size() == 0)
    load_boards();

  if (board != ""){
    currentreply = std::find(boards_nm.begin(), boards_nm.end(), board) - boards_nm.begin();
    board = "";
  }

  int drawn = 0;
  int i = 0;
  String desc = "";


  for (i = 0; i < boards_ws.size(); i++) {
    if (i > currentreply - 6) {
      if (drawn < 12) {
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(boards_ws[i] ? ((boards_ds[i].indexOf("Ponies") > 0) ? CHANDUINO_THEME_BOARDSELECT_P : CHANDUINO_THEME_BOARDSELECT_WS) : CHANDUINO_THEME_BOARDSELECT_NSFW, (i == currentreply) ? CHANDUINO_THEME_BOARDSELECT_SELECTION : bgcolor);
        tft.drawString(boards_ds[i], 6, 6 + drawn * 10);
        drawn++;
      } else if(maxreply != -2) {
        break;
      }
    }
  }

  //currentreply = i-2;
  if (maxreply == -2) {
    maxreply = i - 1;
  }
  draw_reply_number();
}

/**
 * Loads a thread or reply and calls draw_reply on it.
 */
void load_reply() {
  tft.fillScreen(bgcolor);
  draw_reply_number();

  Serial.print("\r\nConnecting to ");
  Serial.println(host);
  if (!connectToa4cdn()) {
    load_reply();
    return;
  }

  if (viewMode == 2)
    thread = replies[currentreply];
  Serial.print("GET /" + board + "/thread/" + String(thread) + ".json HTTP/1.1\r\n");
  client.print("GET /" + board + "/thread/" + String(thread) + ".json HTTP/1.1\r\n");
  client.print("Host: " + (String)host + "\r\n");
  client.print("User-Agent: " + useragent + "\r\n");
  client.print("Connection: keep-alive\r\n");
  client.print("Keep-Alive: timeout=30, max=1000\r\n");
  client.print("Cache-Control: no-cache\r\n\r\n");

  // 0 - searching for quotes
  // 1 - searching for colon
  // 2 - searching for comma
  // 3 - searching for closing bracket
  // 4 - searching for the meaning of life
  int mode = 0;
  int buffloc = 0;
  int i = 0;

  bool foundPost = false;
  bool loadImage = false;

  while (client.readStringUntil('\n') != "\r") {}
  while (client.peek() != '0') {
    // Get current chunk len
    int chunklen = (int)strtol(client.readStringUntil('\r').c_str(), NULL, 16);
    client.readStringUntil('\n');
    Serial.println("CHUNK LEN: " + String(chunklen));
    if (chunklen == 0){
      Serial.println("Retrying...");
      load_reply();
      return;
    }
    while (chunklen > 0){
      while (!client.available()){}
      char currentByte = client.read();
      chunklen--;
      if (mode == 4) {
        while (chunklen > 0) {
          client.read();
          chunklen --;
        }
        continue;
      }
      if (buffloc >= 0)
        buff[buffloc] = currentByte;
      // Set string end byte to nullbyte just in case
      buff[buffloc + 1] = 0;

      if (mode == 0 && currentByte == '"'){
        if (strncmp(buff,"\"no\"",4) == 0){
          mode = 1;
          draw_progress_bar(i, currentreply);
          i++;
        }
        buffloc = 0;
      } else if (mode == 1 && currentByte == ':'){
        mode = 2;
        buffloc = -1;
      } else if (mode == 2 && currentByte == ','){
        if (buffloc >= 0)
          buff[buffloc] = 0;
        if (String(buff).toInt() == replies[currentreply]) {
          Serial.println("Found reply, loading...");
          mode = 3;
        } else {
          mode = 0;
        }
        buffloc = 0;
      } else if (mode == 3 && currentByte == '}'){
        buff[0] = '{';
        Serial.println("Reply loaded.");
        foundPost = true;
        loadImage = draw_reply(String(buff));
        draw_reply_number();
        draw_progress_bar(0, 1);
        mode = 4;
        continue;
      }

      if (buffloc == 0){
        buff[buffloc] = currentByte;
        buff[buffloc + 1] = 0;
      }

      if (buffloc > 32766)
        buffloc = 0;
      buffloc++;
    }
    client.readStringUntil('\n');
  }
  client.readStringUntil('\n');

  // Artifact from an earlier version of the code.
  // Right now it just displays "..." if loading a
  // post fails and retries loading the post.
  if (!foundPost) {
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(0x0000, bgcolor);
    tft.drawString("...", tft.width() / 2, tft.height() / 2);
    load_reply();
  } else if (loadImage) {
    draw_img(0);
    draw_reply_number();
    draw_progress_bar(0, -1);
    Serial.println("Done!");
  } else {
    Serial.println("Multipage reload");
    load_reply();
  }
}

/**
 * Draws a thread or reply.
 */
bool draw_reply(String jsonsnippet) {
  DynamicJsonDocument reply(1024 * 32);  //32KB
  jsonsnippet.replace("\\u2019", "&#039;");
  jsonsnippet.replace("\\u201c", "&quot;");
  jsonsnippet.replace("\\u201d", "&quot;");
  jsonsnippet.replace("<s>", "<x>");
  jsonsnippet.replace("</s>", "</x>");
  jsonsnippet.replace("\\u", "$u");
  deserializeJson(reply, jsonsnippet);
  const char *nowb = reply["now"];
  const char *comb = reply["com"];
  const char *nameb = reply["name"];
  const char *subb = reply["sub"];
  const char *extb = reply["ext"];
  const char *fnameb = reply["filename"];
  const int imgw = reply["w"];
  const int imgh = reply["h"];
  int tnw = reply["tn_w"];
  int tnh = reply["tn_h"];

  if (viewMode == 2) {
    postReplyCount = reply["replies"];
  }
  
  String fulltext = "";
  if (String(subb).length() > 0) {
    fulltext.concat(String(subb) + " ");
  }
  fulltext.concat("<n>" + String(nameb) + "</n> " + String(nowb) + " No." + String(replies[currentreply]) + "<br>");
  tft.fillScreen(bgcolor); //moved this here or else pic won't be visible lol
  // Parse the tim element (timestamp) to load the image later on
  tim = "";
  if (String(extb).length() > 0) {
    fulltext.concat("File: <z>" + String(fnameb) + String(extb) + "</z> (" + String(imgw) + "x" + String(imgh) + ")<br>");
    bool readingtim = false;
    for (int x = 10; x < jsonsnippet.length(); x++) {
      if (readingtim) {
        if (String(jsonsnippet.charAt(x)) == String(",")) {
          break;
        }
        tim.concat(String(jsonsnippet.charAt(x)));
      }
      if (jsonsnippet.substring(x - 5, x + 1) == "\"tim\":") {
        readingtim = true;
      }
    }
    tnw /= 2;
    tnh /= 2;
  }
  fulltext.concat(comb);
  //FOR COLORS: http://www.rinkydinkelectronics.com/calc_rgb565.php (RGB565)
  //TEXT IS 6x8 CHARACTERS
  //4CHAN PREVIEW IMAGES ARE ???x125
  //SCREEN IS: 240x135
  int sx = 9;
  int sy = 6;
  int txtmode = 0;
  String entity = "";
  tft.setTextColor(CHANDUINO_THEME_POST_TITLE, bgcolor);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  int currentMultiPage = 0;
  // This part parses and draws the text.
  // I honestly don't remember anything from the
  // night I wrote this so you are on your own.
  for (int x = 0; x < fulltext.length(); x++) {
    if (sx < tnw + 12 && sy < tnh + 6 && currentMultiPage == 0) {
      sx = tnw + 12;
    }
    if (sy > 130 && viewMode == 1) {
      if (multiPage < 1) {
        multiPage = 0;
        break;
      } else if (currentMultiPage < multiPage) {
        currentMultiPage += 1;
        sx = 9;
        sy = 6;
      } else {
        break;
      }
    }
    String cchar = String(fulltext.charAt(x));
    if (cchar == "<") {
      txtmode = 1;
      cchar = String(fulltext.charAt(x + 1));
      if (cchar == "a") {
        tft.setTextColor(CHANDUINO_THEME_POST_YOU, bgcolor);
      } else if (cchar == "b") {
        sx = 9;
        sy += 10;
      } else if (cchar == "n") {
        tft.setTextColor(CHANDUINO_THEME_POST_NAME, bgcolor);
      } else if (cchar == "s") {
        tft.setTextColor(CHANDUINO_THEME_POST_GREEN, bgcolor);
      } else if (cchar == "z") {
        tft.setTextColor(CHANDUINO_THEME_POST_FILE, bgcolor);
      } else if (cchar == "x") {
        tft.setTextColor(CHANDUINO_THEME_POST_SPOILER_FG, CHANDUINO_THEME_POST_SPOILER_BG);
      } else if (cchar == "/") {
        tft.setTextColor(CHANDUINO_THEME_POST_TEXT, bgcolor);
      }

    }
    if (cchar == "&") {
      txtmode = 2;
      cchar = String(fulltext.charAt(x + 1));
      //should be replaced with full HTML entity parser
      if (currentMultiPage == multiPage || multiPage == -1 || viewMode != 1) {
        if (cchar == "g") {
          tft.drawString(">", sx, sy);
        } else if (cchar == "0") {
          tft.drawString("'", sx, sy);
        } else {
          tft.drawString("Õ", sx, sy);
        }
      }
      sx += 6;
      if (sx > 225) {
        sx = 9;
        sy += 10;

      }
    }
    if (txtmode == 0) {
      if (currentMultiPage == multiPage || multiPage == -1 || viewMode != 1) {
        tft.drawString(cchar, sx, sy);
      }
      sx += 6;
      if (sx > 225) {
        sx = 9;
        sy += 10;
      }
    } else if (txtmode == 1) {
      if (cchar == ">") {
        txtmode = 0;
      }
    } else if (txtmode == 2) {
      if (cchar == ";") {
        txtmode = 0;
      }
    }
  }
  if (seenAllNewPosts){
    lastReadReply = replies[maxreply];
  } else {
    tft.setTextColor(CHANDUINO_THEME_THREADWATCHER_PRIMARY, bgcolor);
    if (replies[currentreply] == lastReadReply)
      tft.setTextColor(CHANDUINO_THEME_THREADWATCHER_SECONDARY, CHANDUINO_THEME_THREADWATCHER_PRIMARY);
    tft.setTextDatum(BL_DATUM);
    if (currentreply == maxreply){
      seenAllNewPosts = true;
      newPostCount = 0;
      tft.drawString("No new posts.", 9, 125);
      lastReadReply = replies[maxreply];
    }else{
      tft.drawString(String(newPostCount) + (newPostCount == 1 ? " new post." : " new posts."), 9, 125);
    }
  }
  if (currentMultiPage < multiPage) {
    currentreply++;
    multiPage = -1;
    if (currentreply > maxreply) {
      currentreply = 0;
    }
    return 0;
  }
  return 1;
}

/**
 * Draws current thread/reply number in the corner.
 */
void draw_reply_number() {
  tft.setTextColor(CHANDUINO_THEME_POST_TEXT, bgcolor);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(String(currentreply + 1) + "/" + String(maxreply + 1), 231, 125);
}

/**
 * Draws connection restored message or show boards for when WiFi is restored.
 */
void draw_connection_restored() {
  if (viewMode == 3) {
    show_boards();
  } else {
    tft.setTextColor(0x0000, bgcolor);
    tft.drawString("Connection restored!", tft.width() / 2, tft.height() / 2);
  }
}

/**
 * Draws loading text.
 */
void draw_loading_text() {
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(CHANDUINO_THEME_LOADING, bgcolor);
  tft.drawString("Loading...", tft.width() / 2, tft.height() / 2);
}

/**
 * Draws progress bar.
 */
void draw_progress_bar(int progress, int total) {
  if (CHANDUINO_HIDE_PROGRESS_BAR) return;
  if (total == -1) {
    tft.drawFastHLine(0, tft.height() - 1, tft.width(), bgcolor);
  }
  if (total == 0) return;
  int startPoint = (tft.width() * progress)/total;
  int endPoint = (tft.width() * (progress + 1))/total;
  if (startPoint > tft.width()) return;
  if (startPoint == endPoint) return;
  // tft.fillRect(startPoint, 0, endPoint - startPoint, 16, CHANDUINO_THEME_LOADING);
  tft.drawFastHLine(startPoint, tft.height() - 1, endPoint - startPoint, CHANDUINO_THEME_LOADING);
}

/**
 * Loads either all threads on a board or all replies on a thread.
 */
void load_posts() {
  // Wipe replies to avoid bugs
  for (int i = 0; i < 2001; i++) {
    replies[i] = 0;
  }

  Serial.print("\r\nConnecting to ");
  Serial.println(host);

  if (!connectToa4cdn()) {
    load_posts();
    return;
  }

  if (viewMode == 1) {
    Serial.print("GET /" + board + "/thread/" + String(thread) + ".json HTTP/1.1\r\n");
    client.print("GET /" + board + "/thread/" + String(thread) + ".json HTTP/1.1\r\n");
  } else {
    Serial.print("GET /" + board + "/catalog.json HTTP/1.1\r\n");
    client.print("GET /" + board + "/catalog.json HTTP/1.1\r\n");
  }
  client.print("Host: " + (String)host + "\r\n");
  client.print("User-Agent: " + useragent + "\r\n");
  client.print("Connection: keep-alive\r\n");
  client.print("Keep-Alive: timeout=30, max=1000\r\n");
  client.print("Cache-Control: no-cache\r\n\r\n");

  int i = 0;
  int currentpost = 0; //only used for viewMode 2

  // 0 - searching for quotes
  // 1 - searching for colon
  // 2 - searching for comma
  int mode = 0;
  int buffloc = 0;
  // Chunked encoding hack
  // This part has some horrible buffer code
  // because I'm not familiar with C
  while (client.readStringUntil('\n') != "\r") {}
  while (client.peek() != '0') {
    // Get current chunk len
    int chunklen = (int)strtol(client.readStringUntil('\r').c_str(), NULL, 16);
    client.readStringUntil('\n');
    Serial.println("CHUNK LEN: " + String(chunklen));
    if (chunklen == 0){
      Serial.println("Retrying...");
      load_posts();
      return;
    }
    while (chunklen > 0){
      while (!client.available()){}
      char currentByte = client.read();
      //Serial.print(currentByte);
      if (buffloc >= 0)
        buff[buffloc] = currentByte;
      // Set string end byte to nullbyte just in case
      buff[buffloc + 1] = 0;

      if (mode == 0 && currentByte == '"'){
        //Serial.println(buff);
        if (viewMode == 2 && strncmp(buff,"\"replies\"",9) == 0){
          replies[i] = currentpost;
          draw_progress_bar(i, boardPostCount);
          i++;
        }
        if (strncmp(buff,"\"no\"",4) == 0){
          mode = 1;
        }
        buffloc = 0;
      } else if (mode == 1 && currentByte == ':'){
        mode = 2;
        buffloc = -1;
      } else if (mode == 2 && currentByte == ','){
        if (buffloc >= 0)
          buff[buffloc] = 0;
        if (viewMode == 1) {
          replies[i] = String(buff).toInt();
          draw_progress_bar(i, postReplyCount);
          i++;
        } else {
          currentpost = String(buff).toInt();
        }
        mode = 0;
        buffloc = 0;
      }

      if (buffloc == 0){
        buff[buffloc] = currentByte;
        buff[buffloc + 1] = 0;
      }

      if (buffloc > 32766)
        buffloc = 0;
      buffloc++;
      chunklen--;
    }
    client.readStringUntil('\n');
  }
  client.readStringUntil('\n');
  Serial.print("Posts loaded!\r\n");
  currentreply = 0;
  maxreply = i - 1;

  if (maxreply == -1) {
    load_posts();
  }
}

/**
 * Draw the current image. If `full` is enabled
 * show in fullscreen, otherwise show thumbnail.
 */
void draw_img(bool full) {
  if (!full && multiPage > 0)
    return;
  if (tim.length() == 0)
    return;

  connectToa4cdn();
 
  uint32_t cacheBuster = esp_random();
  Serial.print("GET /" + board + "/" + tim + "s.jpg?" + cacheBuster + " HTTP/1.1\r\n");
  client.print("GET /" + board + "/" + tim + "s.jpg?" + cacheBuster + " HTTP/1.1\r\n");
  client.print("Host: i.4cdn.org\r\n");
  client.print("User-Agent: " + useragent + "\r\n");
  client.print("Connection: keep-alive\r\n");
  client.print("Keep-Alive: timeout=30, max=1000\r\n");
  client.print("Cache-Control: no-cache\r\n\r\n");

  int chunklen = 0;
  int buffloc = 0;

  while (1){
    client.readStringUntil('\n');
    if (client.peek() == '\r') {
       if (client.readStringUntil('\n') == "\r"){
         Serial.println("Something wrong while processing image header, returning dangerously.");
         Serial.println(client.peek());
         connectToa4cdn();
         return;
       }
    }
    if (client.readStringUntil(':') == "Content-Length"){
      client.readStringUntil(' ');
      chunklen = String(client.readStringUntil('\r')).toInt();
      client.readStringUntil('\n');
      break;
    }
  }

  while (client.readStringUntil('\n') != "\r") {}
  while (chunklen > 0){
    while (!client.available()){}
    chunklen--;
    char currentByte = client.read();
    if (buffloc >= 0)
      buff[buffloc] = currentByte;
    // Set string end byte to nullbyte just in case
    buff[buffloc + 1] = 0;

    if (buffloc > 32766)
      buffloc = 0;

    buffloc++;
  }

  /*
  Chunked HTTP TROLL CODE
  int buffloc = 0;
  while (client.readStringUntil('\n') != "\r") {}
  while (client.peek() != '0') {
    // Get current chunk len
    int chunklen = (int)strtol(client.readStringUntil('\r').c_str(), NULL, 16);
    client.readStringUntil('\n');
    Serial.println("CHUNK LEN: " + String(chunklen));
    if (chunklen == 0){
      Serial.println("Retrying...");
      draw_img(full);
      return;
    }
    while (chunklen > 0){
      while (!client.available()){}
      chunklen--;
      char currentByte = client.read();
      if (buffloc >= 0)
        buff[buffloc] = currentByte;
      // Set string end byte to nullbyte just in case
      buff[buffloc + 1] = 0;

      if (buffloc > 32766)
        buffloc = 0;

      buffloc++;
    }
    client.readStringUntil('\n');
  }
  client.readStringUntil('\n');
  */

  if (buff[0] == '<') {
    Serial.println("Got HTML while rendering image, skipping.");
    return;
  }

  // Just logging
  Serial.println("Image size:" + String(buffloc));

  Serial.print("First 10 Bytes: ");
  for (int ipt = 0; ipt < 11; ipt++) {
    Serial.print(buff[ipt], HEX);
    Serial.print(",");
  }
  Serial.print("\nLast 10 Bytes : ");
  for (int ipt = 10; ipt >= 0; ipt--) {
    Serial.print(buff[buffloc - ipt], HEX);
    Serial.print(",");
  }
  Serial.println("");

  // if we are displaying as a thumbnail then show it 2x smaller
  TJpgDec.setJpgScale(full ? 1 : 2); //1, 2, 4 or 8
  uint16_t w = 0, h = 0;
  TJpgDec.getJpgSize(&w, &h, (const uint8_t*)buff, sizeof(buff));
  // show the pic in the middle or the edge depending on if it is a thumbnail
  TJpgDec.drawJpg(full ? 120 - (w / 2) : 6, full ? 68 - (h / 2) : 6, (const uint8_t*)buff, sizeof(buff));
}

// Saves posts so we can restore the position later
void savePosts(){
  for (int i = 0; i < 2001; i++) {
      posts[i] = replies[i];
  }
  savedpost = currentreply;
  maxposts  = maxreply;
}

// Restores posts after savePosts()
void restorePosts(){
  for (int i = 0; i < 2001; i++) {
      replies[i] = posts[i];
  }
  currentreply = savedpost;
  maxreply  = maxposts;
}

void updateLastTimes(){
  lastBtnPress = esp_timer_get_time();
  lastRefresh = esp_timer_get_time();
}

/**
 * Check for new replies in a thread and notify.
 * Code is overengineered and ugly because we
 * want to handle cases where replies are deleted.
 */
void threadwatcherUpdate(){
  Serial.println("Checking for new replies...");
  // Save old variables and reload the posts
  for (int i = 0; i < 2001; i++) {
    oldReplies[i] = replies[i];
  }
  int oldMax = maxreply;
  int oldcurrentreply = currentreply;
  load_posts();
  currentreply = oldcurrentreply;

  // Check how many new posts
  int newPostCountTemp = 0;
  for (int i = 0; i <= maxreply; i++) {
    bool exists = false;
    for (int j = 0; j <= oldMax; j++) {
      if (replies[i] == oldReplies[j]){
        exists = true;
        break;
      }
    }
    if (!exists) {
      newPostCountTemp++;
      Serial.println("Detected new post: " + String(replies[i]) + " at ID " + String(i));
    }
  }

  // Check if our old selected reply still exists and is the same ID
  if (oldReplies[oldcurrentreply] != replies[oldcurrentreply]){
    // Oh no, we couldn't find it
    // Let's search for it and fallback to previous posts if we can't find it
    Serial.println("Current reply has moved, investigating");
    currentreply = 0;
    for (int i = oldcurrentreply; i > 0; i--) {
      for (int j = 0; j <= maxreply; j++) {
        if (oldReplies[i] == replies[j]){
          // We found a match, set it as our current reply
          Serial.println("We found a match, set it as our current reply");
          currentreply = j;
          // If we are loading a different post than the original, disable multipage
          if (replies[currentreply] != oldReplies[oldcurrentreply]){
            Serial.println("Current reply deleted, loading the closest older one.");
            multiPage = -1;
          }
          break;
        }
      }
      if (currentreply > 0)
        break;
    }
  }

  if (newPostCountTemp > 0){
    Serial.println("Found " + String(newPostCountTemp) + " new replies.");
    newPostCount += newPostCountTemp;
    seenAllNewPosts = false;
    load_reply();
    updateLastTimes();
  } else {
    Serial.println("Found no new replies.");
  }
}

void time_loop(){
  // Screensaver
  if (CHANDUINO_SCREENSAVER_ENABLED && (esp_timer_get_time() - lastBtnPress) > CHANDUINO_SCREENSAVER_TIME*1000000){
    digitalWrite(TFT_BL, LOW);
  } else {
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  }
  // Threadwatcher
  if (CHANDUINO_THREADWATCHER_ENABLED && viewMode == 1 && (esp_timer_get_time() - lastRefresh) > CHANDUINO_THREADWATCHER_TIME*1000000){
    threadwatcherUpdate();
    lastRefresh = esp_timer_get_time();
  }
}

/* Other stuff */

void setup() {
  Serial.begin(115200);
  // This part used to have an ascii art of Cadence but I removed it
  Serial.println("HI ANON!");
  if (!EEPROM.begin(512)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  } else {
    Serial.println("EEPROM initialized successfully!");
  }
  TJpgDec.setSwapBytes(false);
  TJpgDec.setCallback(tft_output);
  tft.init();
  tft.setRotation(1);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.fillScreen(CHANDUINO_THEME_BOARD_BACKGROUND_WS);
  tft.setTextDatum(MC_DATUM);

  if (TFT_BL > 0) { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
  }

  tft.setSwapBytes(true);
  tft.setTextColor(CHANDUINO_THEME_POST_TEXT, CHANDUINO_THEME_BOARD_BACKGROUND_WS);

  int vref = 1100;
  uint16_t v = analogRead(ADC_PIN);
  float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);

  tft.drawString("Chanduino v" + String(CHANDUINO_VERSION), tft.width() / 2, 8 * 7);
  tft.drawString("Connecting...", tft.width() / 2, 8 * 8);
  tft.drawString(String(battery_voltage) + "V", tft.width() / 2, 8 * 15);

  //Initialize buttons
  button_init();

  viewMode = 3;
  currentreply = 0;
  maxreply = -2;

  client.setCACert(root_ca);

  WiFi.mode(WIFI_STA);
  if (connect_wifi()) {
    show_boards();
  }
}

void loop() {
  if (wifiMode == 1) {
    wifiLoop();
  }
  button_loop();
  time_loop();
}

/**
 * Hosts the hotspot website.
 */
void wifiLoop() {
  WiFiClient client = server.available();/* I don't remember writing this comment and I do not like it: */
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;                        // and add it to the header.
        if (c == '\n') {                    // If the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            String requestBody;

            while (client.available()) {
              requestBody += (char)client.read();
            }

            if (requestBody.length()) {
              Serial.println("Got credentials!");
              DynamicJsonDocument credentials(1024 * 1); //1KB
              deserializeJson(credentials, requestBody);
              //set EEPROM creds
              const char *jsonssid = credentials["ssid"];
              const char *jsonpwd = credentials["pwd"];
              EEPROM.writeString(0, jsonssid);
              EEPROM.writeString(256, jsonpwd);
              EEPROM.commit();
              wifiMode = 0;
              WiFi.mode(WIFI_STA);
              if (connect_wifi()) {
                draw_connection_restored();
                return;
              }
            }

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<title>Chanduino</title>");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center }");
            client.println("body { background-color: #1D1F21; color: #B294BB }");
            client.println(".button { background-color: #B5BD68; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("</style></head>");

            // Web Page Heading
            client.println("<body><h1>Chanduino</h1>");
            if (requestBody.length()) {
              client.println("<p>Chanduino is attempting to connect to WiFi...</p>");
            } else {
              int n = WiFi.scanNetworks();
              Serial.println("scan done");
              if (n == 0) {
                client.println("<p>No networks detected, please refresh</p>");
              } else {
                client.println("<h3>Pick a network!</h3>");
                for (int i = 0; i < n; ++i) {
                  String iSSID = WiFi.SSID(i);
                  String esc_iSSID = iSSID;
                  iSSID.replace('"', '&quot;');
                  iSSID.replace("'", "\'");
                  esc_iSSID.replace('&', '&amp;');
                  esc_iSSID.replace('<', '&lt;');
                  esc_iSSID.replace('>', '&gt;');
                  esc_iSSID.replace('"', '&quot;');
                  client.print(String("<button class=\"button\" onclick=\"connectWifi(") + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "true" : "false") + ",'" + iSSID + "')\">" + esc_iSSID + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : "*") + "</button><br>");
                  client.print("</p>");
                }
              }
            }
            client.println("<script>\nfunction connectWifi(open,ssid){\n  let pwd = '';\n  if (!open){\n    pwd = prompt('Please enter the password for ' + ssid, '');\n    if (pwd == null) { \n      return;\n    }\n  }\n  fetch('/', { headers: { 'Accept': 'application/json', 'Content-Type': 'application/json' }, method: 'POST', body: JSON.stringify({ssid,pwd})});\n  document.body.innerHTML = '<h1>Connecting...</h1>';\n}\n</script>");
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void button_loop() {
  btn1.loop();
  btn2.loop();
}

/**
 * Connects to the saved WiFI.
 * If no WiFi is found, launches a hotspot (website) and asks the user to connect to a network through it.
 */
bool connect_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("WiFi not connected, attempting to re-connect...");
    WiFi.begin(EEPROM.readString(0).c_str(), EEPROM.readString(256).c_str());
    Serial.println(EEPROM.readString(0));
    // Prints WiFI password, commented out for obvious reasons
    //Serial.println(EEPROM.readString(256));
    // Try to connect for 5 seconds
    for (int i = 100; i > 0; i--) {
      delay(50);
      Serial.print(".");
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("No WiFi, showing error screen");
      WiFi.mode(WIFI_OFF);
      WiFi.mode(WIFI_AP);
      // You can specify a password for the AP:
      // WiFi.softAP("Chanduino", "password");
      WiFi.softAP("Chanduino");
      IPAddress IP = WiFi.softAPIP();
      server.begin();
      Serial.print("AP IP address: ");
      Serial.println(IP);
      tft.setTextColor(CHANDUINO_THEME_POST_TEXT, CHANDUINO_THEME_BOARD_BACKGROUND_WS);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("No wifi detected!", tft.width() / 2, 8 * 2);
      tft.drawString("Press any button to retry", tft.width() / 2, 8 * 3);
      tft.drawString("or connect to the 'Chanduino'", tft.width() / 2, 8 * 4);
      tft.drawString("Wifi and visit the following", tft.width() / 2, 8 * 5);
      tft.drawString("URL on your device for setup:", tft.width() / 2, 8 * 6);
      tft.drawString("http://" + ipToString(IP), tft.width() / 2, 8 * 7);

      tft.drawString("Note: you might have to disable", tft.width() / 2, 8 * 14);
      tft.drawString("mobile data on phones", tft.width() / 2, 8 * 15);
      wifiMode = 1;
      return false;
    }
  }
  return true;
}

String ipToString(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void disconnect_wifi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  if ( y >= tft.height() ) return 0;
  //w,h = 16x16
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void heap() {
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
}

void cb_connection_ok(void *pvParameter) {
  ESP_LOGI(TAG, "I have a connection!");
}


void test_all_boards() {
  currentreply = 0;
  while (currentreply != maxreply) {
    // Load Board
    load_board();
    viewMode = 2;
    Serial.println(board);
    draw_loading_text();
    load_posts();
    load_reply();
    delay(50);
    // Unload Board
    viewMode = 3;
    bgcolor = CHANDUINO_THEME_BOARDSELECT_BACKGROUND;
    currentreply = 0;
    maxreply = -2;
    show_boards();
    currentreply += 1;
    show_boards();
  }
}

// Shitty unit tests
void tests() {
  test_all_boards();
}
