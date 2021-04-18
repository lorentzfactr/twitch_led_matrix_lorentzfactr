#define PIN 2
#define colorSaturation 255       //This defines brightness for specific colors, off = 0, full on = 255


typedef ColumnMajorAlternatingLayout MyPanelLayout;
const uint8_t PanelWidth = 16; 
const uint8_t PanelHeight = 16;
const uint16_t PixelCount = PanelWidth * PanelHeight;  
const byte numChars = 100;         //Max size of incoming string from Twitch Chat

char * rchar;
char * gchar; 
char * bchar;
char user[numChars];              //List for incoming string
char receivedChars[numChars];     
char startMarker = '(';
char endMarker = ')';


boolean newData = false;          //Logic var for determining the end of the string in startEnd function

int animationSpeed = 25;           //Var for the delay speed in LF animation                
int newColor = 0;


uint8_t rval = 0;                 //Var for Twitch user defined red value
uint8_t gval = 0;                 //Var for Twitch user defined green value
uint8_t bval = 0;                 //Var for Twitch user defined blue value


unsigned int x = 0;               //Vars for x,y location for Twitch user defined LED to send color to
unsigned int y = 0;
unsigned int x_1 = 0;
unsigned int y_1 = 0;
unsigned int x_2 = 0;
unsigned int y_2 = 0;


//Called specifically for use with ESP8266, for other controllers please visit the wiki:
//https://github.com/Makuna/NeoPixelBus/wiki

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(PixelCount);

//Called specfically for my panel. Yours might be different. Depends on how you wired it up. ¯\_(ツ)_/¯
//See https://github.com/Makuna/NeoPixelBus/wiki/Matrix-Panels-Support for more info.
NeoTopology<MyPanelLayout> topo(PanelWidth, PanelHeight);





//Format: RGB
//Just setup for some common colors 
RgbColor red(colorSaturation, 0, 0);
RgbColor salmon(250, 128, 114);  
RgbColor pink(255, 105, 180);
RgbColor orange(255, 105, 180);     
RgbColor yellow(colorSaturation, 150, 0);
RgbColor yellowgreen(154, 205, 50);
RgbColor green(0, colorSaturation, 0);
RgbColor olive(85, 107, 47);
RgbColor blue(0, 0 , colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

//This functions defines the start "[" and end marker "]" and puts the string information in between the markers into a list.
//Borrowed from Serial Input Basics: https://forum.arduino.cc/index.php?topic=396450

int colorMixer()
  {
    rval = rval + 10;
    gval = gval + 20;
    bval = bval + 5;
    RgbColor user(rval,gval,bval);
  }

 bool correctNumberOfCommas(String message, int expected){
  int numFound=0;
     
  int indexFound = message.indexOf(",");
  while(indexFound > -1){
    numFound++; 
    indexFound = message.indexOf(",",indexFound+1);            
  }

  return(expected == numFound);
}

bool LEDxyIsValid(String message,int expected){

  return (message.indexOf(startMarker) > -1 && message.indexOf(endMarker) && correctNumberOfCommas(message,expected));

}

char StartEnd(String chatMSG){
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char rc;
    
    for (int buflen = 0 ; buflen <= 32; buflen++) {
      user[buflen] = chatMSG[buflen];
      rc = user[buflen];
      if (recvInProgress == true){
        if (rc != endMarker){
          receivedChars[ndx] = rc;
          ndx++;
          
          if (ndx >= numChars){
            ndx = numChars-1;
          }
        }
        else {
          receivedChars[ndx] = '\0';
          recvInProgress = false;
          ndx = 0;
        }
      }

      else if (rc == startMarker){
        recvInProgress = true;
        
      }
      
    }
}

//This next set of functions tokenize the string characters and converts the tokens into integer values.
//Borrowed from Serial Input Basics: https://forum.arduino.cc/index.php?topic=396450

//A Twitch user must define their inputs as !LEDcolor[r,g,b]. 
char setColor(){
    char * strtokIndx;                           // this is used by strtok() as an index
    
    strtokIndx = strtok(receivedChars,",");      // get the first part - red value
    rchar = strtokIndx;
    rval = atoi(strtokIndx);                     // convert this part to an integer
    
    strtokIndx = strtok(NULL, ",");              // get the second part - blue value
    gchar = strtokIndx;
    gval = atoi(strtokIndx);                     // convert this part to an integer

    strtokIndx = strtok(NULL, ",");              // get the third part - green value
    bchar = strtokIndx;
    bval = atoi(strtokIndx);                     // convert this part to an integer
 }

//A Twitch user must define their inputs as !LEDline[x_1,y_1,x_2,y_2]. 
char get2XY(){
    char * strtokIndx;                           // this is used by strtok() as an index
    
    strtokIndx = strtok(receivedChars, ",");      // get the X location
    x_1 = atoi(strtokIndx);                    // convert this part to an integer

    strtokIndx = strtok(NULL, ",");               // get the Y location
    y_1 = atoi(strtokIndx);                    // convert this part to an integer

    strtokIndx = strtok(NULL, ",");      // get the X location
    x_2 = atoi(strtokIndx);                    // convert this part to an integer

    strtokIndx = strtok(NULL, ",");               // get the Y location
    y_2 = atoi(strtokIndx);                    // convert this part to an integer

 }

//A Twitch user must define their inputs as !LEDxy[x,y]
char get1XY(){
    char * strtokIndx;                           // this is used by strtok() as an index
    
    strtokIndx = strtok(receivedChars, ",");      // get the X location
    x = atoi(strtokIndx);                         // convert this part to an integer

    strtokIndx = strtok(NULL, ",");               // get the Y location
    y = atoi(strtokIndx);                         // convert this part to an integer

 }



//LorentzFactr's animation to fill the whole screen with the last set color.
int LFanimation(){
  RgbColor user(rval, gval, bval);                  //set the latest color from a Twitch user
  for (int i = 0 ; i <= 15; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);
   for (int i = 16 ; i <= 31; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);
    for (int i = 32 ; i <= 47; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);
    
    for (int i = 48 ; i <= 63; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 64 ; i <= 79; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 80 ; i <= 95; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 96 ; i <= 111; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);
      
      for (int i = 112 ; i <= 127; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 128 ; i <= 143; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 144 ; i <= 159; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 160 ; i <= 175; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 176 ; i <= 191; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 192 ; i <= 207; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);
   
      for (int i = 208 ; i <= 223; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);

      for (int i = 224 ; i <= 239; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);


      for (int i = 240 ; i <= 255; i++)  {         //simple for loop that displays each pixel in series 0 to Pixel Count
      //strip.SetPixelColor(i, black);                //wipe the previous color
      //strip.Show();
      //delay(animationSpeed);
      strip.SetPixelColor(i, user);                 //replace with the new color
      }
      strip.Show();
      delay(animationSpeed);
   
}

int LEDlogo(){
  delay(100);
        for (int i = PixelCount ; i >= 0; i--)  {
            strip.SetPixelColor(i, black);
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
         strip.SetPixelColor(i, green);
        }
    
        for (int i = 129; i <= 137; i++){ 
         strip.SetPixelColor(i, green);
        }
        strip.SetPixelColor(150, green);
        strip.SetPixelColor(151, green);
        strip.SetPixelColor(169, green);
        strip.SetPixelColor(168, green);
        strip.SetPixelColor(182, green);
        strip.SetPixelColor(183, green);
        strip.SetPixelColor(201, green);
        strip.SetPixelColor(200, green);
        strip.SetPixelColor(154, green);
        strip.SetPixelColor(155, green);
        strip.SetPixelColor(164, green);
        strip.SetPixelColor(165, green);
    
        
        strip.Show();
        delay(10);
  }


int craftyGremlin(){
  rval = 255;
  gval = 0;
  bval = 0;
  RgbColor user(rval, gval, bval); 
  for (int i = PixelCount ; i >= 0; i--)  {
            
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
        }
        for (int i = 0 ; i <= 10; i++){
          
        
        
          for (int i = PixelCount ; i >= 0; i--)  {
              strip.SetPixelColor(i, black);
              }
          delay(75);    
          strip.Show();
          delay(35); 
          strip.SetPixelColor(77, user);
          strip.SetPixelColor(82, user);
          strip.SetPixelColor(146, user);
          strip.SetPixelColor(173, user);
          strip.Show();
          delay(35);
          
          strip.SetPixelColor(51, user);
          strip.SetPixelColor(76, user);
          strip.SetPixelColor(83, user);
          strip.SetPixelColor(108, user);
          strip.SetPixelColor(140, user);
          strip.SetPixelColor(147, user);
          strip.SetPixelColor(172, user);
          strip.SetPixelColor(179, user);
          strip.Show();
          delay(35);
          
          strip.SetPixelColor(43, user);
          strip.SetPixelColor(52, user);
          strip.SetPixelColor(107, user);
          strip.SetPixelColor(116, user);
          strip.SetPixelColor(139, user);
          strip.SetPixelColor(180, user);
          strip.SetPixelColor(203, user);
          strip.Show();
          delay(35);
          
          strip.SetPixelColor(42, user);
          strip.SetPixelColor(117, user);
          strip.SetPixelColor(202, user);
          strip.Show();
          delay(35);
          
          strip.SetPixelColor(41, user);
          strip.SetPixelColor(201, user);
          strip.Show();
          delay(35);
      
          strip.SetPixelColor(40, user);
          strip.SetPixelColor(55, user);
          strip.SetPixelColor(183, user);
          strip.SetPixelColor(200, user);    
          strip.Show();
          delay(35);
          
          strip.SetPixelColor(56, user);
          strip.SetPixelColor(71, user);
          strip.SetPixelColor(167, user);
          strip.SetPixelColor(184, user);    
          strip.Show();
          delay(35);
      
          strip.SetPixelColor(70, user);
          strip.SetPixelColor(89, user);
          strip.SetPixelColor(153, user);
          strip.SetPixelColor(166, user);    
          strip.Show();
          delay(35);
      
          strip.SetPixelColor(90, user);
          strip.SetPixelColor(101, user);
          strip.SetPixelColor(133, user);
          strip.SetPixelColor(154, user);    
          strip.Show();
          delay(35);
      
          strip.SetPixelColor(100, user);
          strip.SetPixelColor(123, user);
          strip.SetPixelColor(132, user);
          strip.Show();
          delay(35);
      
          strip.SetPixelColor(124, user);   
          strip.Show();
          delay(35);
        }
        for (int i = PixelCount ; i >= 0; i--)  {
            strip.SetPixelColor(i, black);
            strip.Show();
            delay(1);
        } 
}

//This function displays the last set color to a specific pixel
int drawOnePix(){
      RgbColor user(rval, gval, bval);
      strip.SetPixelColor(topo.Map(x, y), black);                //wipe the previous color
      strip.Show();
      delay(animationSpeed);
      strip.SetPixelColor(topo.Map(x, y), user);                 //replace with the new color
      strip.Show();
      delay(animationSpeed);
}


//This function calculates and draws the slope of a line given two x,y coordinates. Albeit, shittily.

int drawSlope(){

  int dT = 1;
  float d = 0;
  float t = 0;
  int xt = 0;
  int yt = 0;
  int XX = 0;
  int YY = 0;

  XX = sq(x_2 - x_1);
  YY = sq(y_2 - y_1);
  d = sqrt(XX+YY);
  x = x_1;
  y = y_1;
  drawOnePix();
  
  for (int i = 0 ; i <= d; i++){
        dT++; 
        t = dT/d;
        xt = 1-t;
        x = xt*x_1+t*x_2;
        yt = 1-t;
        y = yt*y_1+t*y_2; 
        drawOnePix();                              
      }
}
