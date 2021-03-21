/*******************************************************************
   Connect to Twitch Chat with a Bot
   
   Created with code from TheOtherLoneStar (https://www.twitch.tv/theotherlonestar)
   Hackaday IO: https://hackaday.io/otherlonestar
   By Brian Lough (https://www.twitch.tv/brianlough)
   YouTube: https://www.youtube.com/channel/UCezJOfu7OtqGzd5xrP3q6WA
   Created with code from noycebru www.twitch.tv/noycebru
   NeoPixelBus from Makuna: https://github.com/Makuna/NeoPixelBus
   LED Matrix w/ NeoPixeBus from LorentzFactr: (https://www.twitch.tv/lorentzfactr)
 *******************************************************************/
 
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <IRCClient.h>
#include <NeoPixelBus.h>          
#include "dontlook.h"             //Define your Wifi credentials and Twitch Oauth here
#include "LorentzFunctions.h"     //Some functions I added to animate and parse incoming strings from Twitch chat
 



//The following code receives and sends info to Twitch:
//define your default values here, if there are different values in config.json, they are overwritten.
#define secret_ssid "my ssid" 
#define IRC_SERVER   "irc.chat.twitch.tv"
#define IRC_PORT     6667


String ircChannel = "";
 
WiFiClient wiFiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wiFiClient);
 
// put your setup code here, to run once:
void setup() {
  
  delay(2000);
  Serial.begin(115200);
  Serial.println();
  
  while (!Serial); // wait for serial attach
  
    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();


    Serial.println();
    Serial.println("Running...");
 
  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
 
  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
 
  ircChannel = "#" + twitchChannelName;
  LEDlogo();
  rval = 0;
  gval = 100;
  bval = 0;
  client.setCallback(callback);
}
 
void loop() {
 
  // Try to connect to chat. If it loses connection try again
  if (!client.connected()) {
    Serial.println("Attempting to connect to " + ircChannel );
    // Attempt to connect
    // Second param is not needed by Twitch
    if (client.connect(TWITCH_BOT_NAME, "", TWITCH_OAUTH_TOKEN)) {
      client.sendRaw("JOIN " + ircChannel);
      Serial.println("connected and ready to rock");
      sendTwitchMessage("Chat lighting intitiated. Proceed with caution.");
    } 
    else {
      Serial.println("failed... try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    return;
  }
  client.loop();
}
 
void sendTwitchMessage(String message) {
  client.sendMessage(ircChannel, message);
}
 
 
void callback(IRCMessage ircMessage) {
  Serial.println("In CallBack");
 
  if (ircMessage.command == "PRIVMSG" && ircMessage.text[0] != '\001') {
      Serial.println("Passed private message.");
     
      ircMessage.nick.toUpperCase();
    
      String message("<" + ircMessage.nick + "> " + ircMessage.text);
   
      //prints chat to serial
      Serial.println(message);   


//LorentzFactr's stream has a SLOBS command !LEDcommands that tells users in chat how to use the display.
//The && statement ensures that any user except my BOT can use this command. Otherwise, everytime someone calls 
//!LEDcommands, the code will pick up one of the commands in the BOT message and display it. A real bummer for the 
//chat who has likely spent the last hour trying to draw something only to be erased by someone just trying to learn how to play.

   if (ircMessage.nick != TWITCH_BOT_NAME){
    
    if (ircMessage.text.indexOf("!LEDcolor") > -1 && ircMessage.text.indexOf("[") > -1 && ircMessage.text.indexOf("]") > -1)
          {
            delay(10);
            chatMSG = ircMessage.text;
            StartEnd(chatMSG);                                //Find the appropriate information in the string and put it into a list
            setColor();                                       //Turn the string into integers and set the color value   
            sendTwitchMessage(ircMessage.nick + " updated the next color.");
             
          }

    if (ircMessage.text.indexOf("!LEDline") > -1 && ircMessage.text.indexOf("[") > -1 && ircMessage.text.indexOf("]") > -1)
          {
            delay(10);
            chatMSG = ircMessage.text;
            StartEnd(chatMSG);                               //Find the appropriate information in the string and put it into a list
            get2XY();                                         //Turn the string into integers and set the start location and length of the line
            drawSlope(); 
   
          }
         
    if (ircMessage.text.indexOf("!LEDxy") > -1 && ircMessage.text.indexOf("[") > -1 && ircMessage.text.indexOf("]") > -1)
          {
            delay(10);
            chatMSG = ircMessage.text;
            StartEnd(chatMSG);                             //Find the appropriate information in the string and put it into a list
            get1XY();                                       //pass the char str of x,y coordinates and define the location by LED index.
            drawOnePix();                                  //display the user defined color at the defined LED index.
          }
   
    if (ircMessage.text.indexOf("!LEDfill") > -1)
          {
            delay(10);
            
              LFanimation();                                  //overwrite all pixels with current last known RGB values   
          }
             
    if (ircMessage.text == "!LEDred"){
        delay(10);
        rval = 200;
        gval = 0;
        bval = 0;
        LFanimation();
    }
    
    if (ircMessage.text == "!LEDyellow"){
        delay(10);
        rval = 200;
        gval = 200;
        bval = 0;
        LFanimation();
    }
   
    if (ircMessage.text == "!LEDgreen"){
        delay(10);
        rval = 0;
        gval = 200;
        bval = 0;
        LFanimation();
    }
           
    if (ircMessage.text == "!LEDblue"){
        delay(10);
        rval = 0;
        gval = 0;
        bval = 200;
        LFanimation();
    }
       
    if (ircMessage.text == "!LEDwhite"){
        delay(10);
        rval = 200;
        gval = 200;
        bval = 200;
        LFanimation();
    }    
    
    if (ircMessage.text == "!LEDcrafty_gremlin"){
        rval = 255;
        gval = 0;
        bval = 0;
        craftyGremlin();
        }
        
    if (ircMessage.text == "!LEDLOGO"){
        
        LEDlogo();   
    }
   
    if (ircMessage.nick != TWITCH_BOT_NAME && ircMessage.text == "!LEDoff"){
        delay(10);
        rval = 0;
        gval = 0;
        bval = 0;
        LFanimation();
    }
   }
    
    return;
  }
}
 
