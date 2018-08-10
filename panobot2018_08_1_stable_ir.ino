
#include <AccelStepper.h> // external library with functions for acceleration of the stepper
#include <Wire.h>
#include <IRremote.h> // external library for the IR remote
#include "U8glib.h" // external library for the display
#if defined(ARDUINO) && ARDUINO >= 100
#define FOCUS_PIN 2  // camera focus pin 
#define SHUTTER_PIN 3 // camera shutter pin
#define SLEEP 10     // sleep pin puts steppers in sleep mode
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif
char val; // variable to receive data from the serial port

// in te stellen variabelen
int shuttertime =  2300;
int shutterdelay = 200; 
int hormove = 4; // factor movement microstepping
int vermove = 2; // factor movement microstepping
int IRpin = 5;  // pin for the IR sensor  
int prog = 12;
IRrecv irrecv(IRpin);
decode_results results;
boolean LEDon = true; // initializing LEDon as true
int stepdegree = 38;
int poshor = 4000; 
int posverstart = 3000; 
int posver = -34 * stepdegree; 
int posverhome = -1 * posverstart; 
int posverst = -67 * stepdegree; 
AccelStepper stepperver(1, 7, 8);
AccelStepper stepperhor(1, 12, 11); 
int program = 0;   
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

void panobot(uint8_t d)
{
    u8g.setRot180();
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,20+d,"panobot");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,35+d,"version");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,50+d,"2018_08_1_stable_IR");
}

void panobot1(uint8_t d)
{
    u8g.setRot180();
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,17+d," A:pano auto");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,32+d," B:pano M");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,47+d," C:TL hor, 3.5sec");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,62+d," D:timelapse M");
}

void panorama2(uint8_t d)
{
    u8g.setRot180();
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,17+d," panoramic");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,32+d," program");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,47+d," press 2,4,8 to set");
    u8g.setFont(u8g_font_helvB10);
    u8g.drawStr(0+d,62+d," shutter time");   
}
void panoramastart(uint8_t d)
{
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,25+d,"panoramic prog");
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,50+d,"started");
}

void timelapsestart(uint8_t d)
{
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,25+d,"timelapse prog");
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,50+d,"started");   
}

void timelapsestart3(uint8_t d)
{
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,15+d,"1 for slow");
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,30+d,"2 normal speed");
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,45+d,"3 fast speed.");
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,60+d,"4 not moving");
    
}

void timelapserichting(uint8_t d)
{
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,11+d,"1: horizontal");
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,22+d,"2: 30 degr");
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,33+d,"3: 45 degr");
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,44+d,"4: 60 degr");
    u8g.setFont(u8g_font_7x14B);
    u8g.drawStr(0+d,55+d,"5: vertical");
}


void sec2(uint8_t d)
{
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,25+d,"shuttertime");
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,50+d,"2 seconds");
}

void sec4(uint8_t d)
{
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,25+d,"shuttertime");
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,50+d,"4 seconds");
}
 
void sec8(uint8_t d)
{
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,25+d,"shuttertime");
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(0+d,50+d,"8 seconds");
}


void horrotate(){
  while (stepperhor.distanceToGo() != 0) // do not do anything other than the following as long as the desired position is not reached
         {
           stepperhor.run();  // turn motor until the above is fulfilled
         }
         delay(shutterdelay);
         stepperhor.setCurrentPosition(0);  // set the current horizontal position as the base position for the next rotation              
  }


void verrotate(){
  while (stepperver.distanceToGo() != 0) // do not do anything other than the following as long as the desired position is not reached
         {
           stepperver.run();  // turn motor until the above is fulfilled
         }
         delay(shutterdelay);
         digitalWrite(SHUTTER_PIN, HIGH); // take picture
         delay(shuttertime); 
         digitalWrite(SHUTTER_PIN, LOW); //         
         stepperver.setCurrentPosition(0);           
  }

void verrotatestart(){
  while (stepperver.distanceToGo() != 0) // do not do anything other than the following as long as the desired position is not reached
         {
           stepperver.run();  // turn motor until the above is fulfilled
         }
         stepperver.setCurrentPosition(0);           
  }


void setup() { 
 
  pinMode(SHUTTER_PIN, OUTPUT); 
  pinMode(SLEEP, OUTPUT);
  digitalWrite(FOCUS_PIN, LOW); // unlock exposure
  irrecv.enableIRIn(); // Start the receiver
  stepperhor.setMaxSpeed(50000);  // snelheid horizontale stepper
  stepperhor.setAcceleration(15000);  // speed acceleration horizontal motor
  stepperver.setMaxSpeed(50000);  // speed vertical stepper
  stepperver.setAcceleration(15000); // speed acceleration vertical stepper
  Serial.begin(9600);       // start serial communication at 9600bps
 // program = 10;
 // prog = 30;
  u8g.firstPage();  
  do {
  panobot(0);
  u8g.setColorIndex(1);
  }while( u8g.nextPage() );
  u8g.firstPage();  
  delay(2000);
  do {
  panobot1(0);
  u8g.setColorIndex(1);
  }while( u8g.nextPage() );
  digitalWrite(SLEEP, LOW);  // disable easydrive
  Wire.begin();
}

void loop()  {  

if (program == 0) 
{ 
if (irrecv.decode(&results)) 
  {irrecv.resume();   // Receive the next value
  }
   if (results.value == 0xFFA25D)  // Automatic panorama programm incl. exposure
   {
     program = 40;
   }
   if (results.value == 0xFF629D)  // standard program with suttertime chose
   {
     program = 10;
   }
   if (results.value == 0xFFE21D)  // timelapse automatic programm
   {
     hormove = 4;
     vermove = 0;
     shuttertime =  2000;
     program = 24;
   }
      if (results.value == 0xFF22DD)  // timelapse with movement custom settings
   {
     program = 11;
     prog = 21;
   }
}
 
if (program == 10) 
  {  digitalWrite(SLEEP, HIGH);  //enable easydriver
    {    
    delay (200);  
    // turn the camera vertically and take a picture
    stepperver.moveTo(posverstart);   //turn the camera to the entered vertical position 
    verrotatestart();
    u8g.firstPage();  
    do {
    panorama2(0);
    u8g.setColorIndex(1);
    }while( u8g.nextPage() );
    program = 11;
    } 
  }  
  
if (program == 11)
   { 
   u8g.firstPage();  
   do {
   panorama2(0);
   u8g.setColorIndex(1);
   }while( u8g.nextPage() );
   if (irrecv.decode(&results)) 
   {irrecv.resume();   // Receive the next value
   }
   if (results.value == 0xFF18E7)  // change zero to your IR remote button number
   {
     shuttertime =  2000;
     program = prog;
     u8g.firstPage();  
      do {
      sec2(0);
      u8g.setColorIndex(1);
      }while( u8g.nextPage() );
      delay(500); 
      results.value = 0;
   }   
   if (results.value == 0xFF10EF)  // change zero to your IR remote button number
   {
     shuttertime =  4000;
     program = prog;
     u8g.firstPage();  
      do {
      sec4(0);
      u8g.setColorIndex(1);
      }while( u8g.nextPage() );
      delay(500); 
      results.value = 0;
   }

   if (results.value == 0xFF4AB5)  // change zero to your IR remote button number
   {
     shuttertime =  8000;
     program = prog;
     u8g.firstPage();  
      do {
      sec8(0);
      u8g.setColorIndex(1);
      }while( u8g.nextPage() );
      delay(500); 
      results.value = 0;
   }   
 }

 if (program == 12) 
 {   u8g.firstPage();  
    do {
    panoramastart(0);
    u8g.setColorIndex(1);
    }while( u8g.nextPage() );
    {        
    digitalWrite(FOCUS_PIN, HIGH); // fix exposure   
    delay(shutterdelay);
    digitalWrite(SHUTTER_PIN, HIGH); // take picture
    delay(shuttertime); 
    digitalWrite(SHUTTER_PIN, LOW); //         
    // turn the camera horizontally and take a picture
    stepperhor.moveTo(poshor+500);  //rotate camera to horizontal position     
    horrotate();            
    }      
    { // turn the camera vertically and take a picture
    stepperver.moveTo(posver);   //turn the camera to the entered vertical position 
    verrotate();         
    }       
    { // turn the camera horizontally and take a picture
    stepperhor.moveTo(poshor-500);  //turn the camera to the entered horizontal position      
    horrotate();
    }
    { // turn the camera vertically and take a picture
    stepperver.moveTo(-posver);   //turn the camera to the entered vertical position 
    verrotate();         
    }    
    { // turn the camera horizontally and take a picture
    stepperhor.moveTo(poshor);  //turn the camera to the entered horizontal position      
    horrotate();             
    }              
    { // turn the camera vertically and take a picture
    stepperver.moveTo(posver);   //turn the camera to the entered vertical position 
    verrotate();         
    }       
    { // turn the camera horizontally and take a picture
    stepperhor.moveTo(poshor);  //turn the camera to the entered horizontal position      
    horrotate();
    }
    { // turn the camera vertically and take a picture
    stepperver.moveTo(-posver);   //turn the camera to the entered vertical position 
    verrotate();         
    } 
    { // turn the camera horizontally and take a picture
    stepperhor.moveTo(poshor);  //turn the camera to the entered horizontal position      
    horrotate();
    }      
    { // turn the camera vertically to start position
    stepperver.moveTo(posverhome);   //turn the camera to the entered vertical position 
    verrotatestart();    
    program = 0;
    }
      digitalWrite(FOCUS_PIN, LOW); // set exposure free
      digitalWrite(SLEEP, LOW);  //turn easydriver off
      u8g.firstPage();  
      do {
      panobot1(0);
      u8g.setColorIndex(1);
      }while( u8g.nextPage() );
  }
  


if (program == 21) 
{    
    digitalWrite(SLEEP, HIGH);
    u8g.firstPage();  
    do {
    timelapserichting(0);
    u8g.setColorIndex(1);
    }while( u8g.nextPage() );
   { 
   if (irrecv.decode(&results)) 
   {irrecv.resume();   // Receive the next value
   }

   if (results.value == 0xFF30CF)  // change code to your IR remote button number
   {
    // horizontal movement
     hormove = -4;
     vermove = 0;
     program = 22;
     delay(500); 
     results.value = 0;
   }
   if (results.value == 0xFF18E7)  //30 degree movement
   {
     hormove = -4;
     vermove = 2;
     program = 22;
     delay(500); 
     results.value = 0; 
   }   
   if (results.value == 0xFF7A85)  //45 degree movement
   {
     hormove = -4;
     vermove = 4;
     program = 22;
     delay(500); 
     results.value = 0;
   }   
   if (results.value == 0xFF10EF)  //60 degree movement
   {
     hormove = -2;
     vermove = 4;
     program = 22;
     delay(500); 
     results.value = 0;
   }
   if (results.value == 0xFF38C7)  // vertical movement
   {
     hormove = 0;
     vermove = 4;
     program = 22;
     delay(500); 
     results.value = 0;
   }
 }
} 

if (program == 22) {
   u8g.firstPage();  
   do {
   timelapsestart3(0);
   u8g.setColorIndex(1);
   }while( u8g.nextPage() );
   if (irrecv.decode(&results)) 
   {irrecv.resume();   // Receive the next value
   }
   if (results.value == 0xFF30CF)  //half speed
   {
     hormove = hormove/2;
     vermove = vermove/2;
     program = 24;
     delay(500); 
     results.value = 0;
   }
   if (results.value == 0xFF18E7)  //normal speed
   {
     hormove = hormove*1;
     vermove = vermove*1;
     program = 24;
     delay(500); 
     results.value = 0;
   }   
   if (results.value == 0xFF7A85)  //dubble speed
   {
     hormove = hormove*2;
     vermove = vermove*2;
     program = 24;
     delay(500); 
     results.value = 0;
   }   
   if (results.value == 0xFF10EF)  // no movement
   {
     hormove = 0;
     vermove = 0;
     program = 24;
     delay(500); 
     results.value = 0;
   }
 } 


if (program == 24) {
       u8g.firstPage();  
       do {
       timelapsestart(0);
       u8g.setColorIndex(1);
       }while( u8g.nextPage() );
       digitalWrite(FOCUS_PIN, HIGH); // fix exposure
        delay(1000);
        digitalWrite(SHUTTER_PIN, HIGH); // take picture
        delay(500); 
        digitalWrite(SHUTTER_PIN, LOW); //
        delay(shuttertime - 1650);
       stepperhor.moveTo(hormove);  
       while (stepperhor.distanceToGo() != 0) 
           {
             stepperhor.run(); 
           }
       stepperhor.setCurrentPosition(0);                
       stepperver.moveTo(vermove);     
       while (stepperver.distanceToGo() != 0) 
           {
             stepperver.run();  
           }
       stepperver.setCurrentPosition(0);              
   } 
       
 
 if (program == 40) 
 {digitalWrite(SLEEP, HIGH);  //schakel easydriver in
    {    
    delay (200);  
    stepperver.moveTo(posverstart);   //turn the camera to the entered vertical position 
    verrotate();
    program = 12;
  
 }  
 } 

}
