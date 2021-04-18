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
#include "dontlook.h"
#include "LorentzFunctions.h"
 



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
      int expected = 0;
      //prints chat to serial
      Serial.println(message);   


//LorentzFactr's stream has a SLOBS command !LEDcommands that tells users in chat how to use the display.
//The && statement ensures that any user except my BOT can use this command. Otherwise, everytime someone calls 
//!LEDcommands, the code will pick up one of the commands in the BOT message and display it. A real bummer for the 
//chat who has likely spent the last hour trying to draw something only to be erased by someone just trying to learn how to play.
   if (ircMessage.nick == TWITCH_BOT_NAME || ircMessage.nick == MY_BOT){
    
    // This is for the usage of bots only...
    
    if(ircMessage.text.indexOf("Thank you for following") > -1)
          {
          craftyGremlin();
          }

          //ADD more fun stuff if people follow, sub, etc...
   }


   else{

    // This is for the usage of users only...
    
    if (ircMessage.text.indexOf("!LEDcolor") > -1 && LEDxyIsValid(message,2))
      {
        delay(10);
        StartEnd(message);                                //Find the appropriate information in the string and put it into a list
        setColor();                                       //Turn the string into integers and set the color value   
      } 
  
          

    if (ircMessage.text.indexOf("!LEDline") > -1 && LEDxyIsValid(message,3))
      {
        delay(10);
        StartEnd(message);                               //Find the appropriate information in the string and put it into a list
        get2XY();                                         //Turn the string into integers and set the start location and length of the line
        drawSlope(); 
      }

         
    if (ircMessage.text.indexOf("!LEDxy") > -1 && LEDxyIsValid(message,1))
      {
        delay(10);
        StartEnd(message);                              //Find the appropriate information in the string and put it into a list
        get1XY();                                       //pass the char str of x,y coordinates and define the location by LED index.
        drawOnePix();                                   //display the user defined color at the defined LED index.
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
    
          
          
    if (ircMessage.text == "!LEDroots"){
          delay(100);
          for (int i = 0 ; i <= 10; i++)  {
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
            strip.SetPixelColor(i, red);
            strip.Show();
            delay(1);
            }
    
           for (int i = 11 ; i <= 20; i++)  {
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
            strip.SetPixelColor(i, yellow);
            strip.Show();
            delay(1);
            }
          for (int i = 21 ; i <= PixelCount; i++)  {
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
            strip.SetPixelColor(i, green);
            strip.Show();
            delay(1);
            }
          }
    
    if (ircMessage.text == "!LEDmurica"){
          delay(100);
          for (int i = 0 ; i <= 10; i++)  {
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
            strip.SetPixelColor(i, red);
            strip.Show();
            delay(1);
            }
    
           for (int i = 11 ; i <= 20; i++)  {
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
            strip.SetPixelColor(i, white);
            strip.Show();
            delay(1);
            }
          for (int i = 21 ; i <= PixelCount; i++)  {
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
            strip.Show();strip.SetPixelColor(i, blue);
            strip.Show();
            delay(1);
            }
          }
  
     if (ircMessage.text == "!LEDKITT"){
          delay(50);
          for (int i = 0 ; i <= PixelCount; i++)  {
              strip.SetPixelColor(i, black);
              strip.Show();
          }
          for (int i = 0 ; i <= 5; i++) {
            for (int i = 0 ; i <= PixelCount; i++)  {
              strip.SetPixelColor(i, black);
              strip.Show();
              delay(1);
              }
            for (int i = 0 ; i <= PixelCount; i++)  {
              strip.SetPixelColor(i, red);
              strip.Show();
              delay(1);
              }
            for (int i = PixelCount ; i >= 0; i--)  {
              strip.SetPixelColor(i, black);
              strip.Show();
              delay(1);
              }
            for (int i = PixelCount ; i >= 0; i--)  {
              strip.SetPixelColor(i, red);
              strip.Show();
              delay(1);
              }
              for (int i = 0 ; i <= PixelCount; i++)  {
              strip.SetPixelColor(i, black);
              strip.Show();
              delay(1);
              }
            for (int i = 0 ; i <= PixelCount; i++)  {
              strip.SetPixelColor(i, red);
              strip.Show();
              delay(1);
              }
             for (int i = PixelCount ; i >= 0; i--)  {
              strip.SetPixelColor(i, black);
              strip.Show();
              delay(1);
              }
            for (int i = PixelCount ; i >= 0; i--)  {
              strip.SetPixelColor(i, red);
              strip.Show();
              delay(1);
              }
            } 
           }
    
    if (ircMessage.text == "!LEDLF"){
        delay(100);
        for (int i = PixelCount ; i >= 0; i--)  {
            strip.SetPixelColor(i, blue);
            //strip.Show();
            //delay(10);
        }
            
        for (int i = 50; i <= 59; i++){ 
         strip.SetPixelColor(i, green);
        }
        for (int i = 68; i <= 77; i++){ 
         strip.SetPixelColor(i, green);
        }
        strip.SetPixelColor(90, green);
        strip.SetPixelColor(91, green);
        strip.SetPixelColor(101, green);
        strip.SetPixelColor(100, green);
        
        for (int i = 118; i <= 126; i++){ 
         strip.SetPixelColor(i, yellow);
        }
    
        for (int i = 129; i <= 137; i++){ 
         strip.SetPixelColor(i, yellow);
        }
        strip.SetPixelColor(150, yellow);
        strip.SetPixelColor(151, yellow);
        strip.SetPixelColor(169, yellow);
        strip.SetPixelColor(168, yellow);
        strip.SetPixelColor(182, yellow);
        strip.SetPixelColor(183, yellow);
        strip.SetPixelColor(201, yellow);
        strip.SetPixelColor(200, yellow);
        strip.SetPixelColor(154, red);
        strip.SetPixelColor(155, red);
        strip.SetPixelColor(164, red);
        strip.SetPixelColor(165, red);
    
        strip.Show();
        delay(10);
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

    if (ircMessage.text == "!LEDoff"){
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
 
