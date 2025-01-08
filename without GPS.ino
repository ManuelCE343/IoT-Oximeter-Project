#include <Adafruit_GFX.h>// Biblioteca principal para todas las pantallas adrafruit
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h" 
#include "heartRate.h"

///**************************
#include <Adafruit_MLX90614.h>

// sensor de temperatura corporal y ambiental
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//Display
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
// SENSOR de ritmo cardiaco
MAX30105 particleSensor;
//************************************


//Constante BEAT 
const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 
float beatsPerMinute;
int beatAvg;
//Constante SPO
double avered = 0; double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 100;
//Hay que ir configurando Estos Valores 
double ESpO2 = 93.0; // Valor inicial de la SpO2 estimada
double FSpO2 = 0.7; // Factor de filtro para la SpO2 estimada
double frate = 0.95; // Filtro de paso bajo para valor de LED IR / Rojo para eliminar el componente de CA


static const unsigned char PROGMEM logo2_bmp[] =
{ 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E, 0x02, 0x10, 0x0C, 0x03, 0x10,              //Logo2 and Logo3 are two bmp pictures that display on the OLED if called
0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00,  };

static const unsigned char PROGMEM logo3_bmp[] =
{ 0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08, 0x03,
0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01, 0x80, 0x00, 0x14, 0x00,
0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C, 0x00,
0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00  };

void setup() {
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  //************************************
mlx.begin(); //Se inicia el sensor
//************************************
//particleSensor.enableDIETEMPRDY(); //Enable the temp ready interrupt. This is required.
  // BPM
  particleSensor.begin(Wire, I2C_SPEED_FAST); 
  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  
  display.clearDisplay();
  display.setCursor(35,10);
  display.print("ITTG");
  display.setTextSize(1);
  display.setCursor(10,40);
  display.print("Balamsemiconductor");
  display.display();
  delay(2000); 

   display.clearDisplay();
  display.setCursor(10,20);
  display.print("Inicializando el ");
  display.setCursor(20,40);
  display.print("Sistema....");
  display.display();
  delay(2000);

   
  }

 
void loop() {

  
  uint32_t ir, red, green;
  double fred, fir;
  double SpO2 = 0;
  float temp_corp = mlx.readObjectTempC();
  long irValue = particleSensor.getIR();
  
  //***********************************************************************
  particleSensor.check();
  while(particleSensor.available()) { 
    
    //Lecturas SPO
    ir = particleSensor.getFIFORed(); 
    red = particleSensor.getFIFOIR(); 
    i++;
    fred = (double)red;
    fir = (double)ir;
    avered = avered*frate+(double)red*(1.0 - frate); 
    aveir = aveir*frate+(double)ir*(1.0 - frate);
    sumredrms += (fred - avered)*(fred - avered);
    sumirrms += (fir - aveir)*(fir - aveir);
    
    //Calculo SPO2
    if((i % Num) == 0)
    { double R = (sqrt(sumredrms)/avered) / (sqrt(sumirrms) / aveir);
      SpO2 = -23.3*(R - 0.4) + 100;
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;
      sumredrms = 0.0; sumirrms = 0.0; i = 0;
      break;  }
    particleSensor.nextSample();
   
    
  if(irValue > 7000){                                           //If a finger is detected
     //Pantalla
     //float temperature = particleSensor.readTemperature();
     display.clearDisplay();
       display.setTextSize(2); //Clear the display
       display.drawBitmap(75, 35, logo2_bmp, 24, 21, WHITE);
       display.setTextSize(1);                                   //Near it display the average BPM you can display the BPM if you want
       display.setTextColor(WHITE);
       display.setCursor(45,2);  
       display.setRotation(3); 
       display.println("BPM"); 
       display.setRotation(0); 
       display.setTextSize(2);
       display.setCursor(20,5);
       display.println(beatAvg); 
       display.setCursor(60,0);
       display.setTextSize(1);
       display.println("Temp:");
       display.setCursor(50,10);
       display.setTextSize(2);
       display.println(temp_corp);
       display.setCursor(110,10);
       display.print("C");

       display.setTextSize(1);                                   //Near it display the average BPM you can display the BPM if you want
       display.setTextColor(WHITE);
       display.setRotation(3);
       display.setCursor(2,2); 
       display.println("SpO2%");
       display.setTextSize(2);
       display.setRotation(0);
       display.setCursor(20,45); 
       display.println(ESpO2,1);
       display.display();
    
  if (checkForBeat(irValue) == true)                        //If a heart beat is detected
  {
   
    display.clearDisplay();
    display.setTextSize(2); 
    display.drawBitmap(70, 30, logo3_bmp, 32, 32, WHITE);
       display.setTextSize(1);                                   //Near it display the average BPM you can display the BPM if you want
       display.setTextColor(WHITE);
       display.setRotation(3);
       display.setCursor(45,2);  
       display.println("BPM"); 
       display.setRotation(0); 
       display.setTextSize(2);
       display.setCursor(20,5);
       display.println(beatAvg); 
       display.setCursor(60,0);
       display.setTextSize(1);
       display.println("Temp:");
       display.setCursor(50,10);
       display.setTextSize(2);
       display.println(temp_corp);
       display.setCursor(110,10);
       display.print("C");

       display.setTextSize(1);                                   //Near it display the average BPM you can display the BPM if you want
       display.setTextColor(WHITE);
       display.setCursor(2,2);
       display.setRotation(3);   
       display.println("SpO2%");
       display.setTextSize(2);
       display.setRotation(0);
       display.setCursor(20,45); 
       display.println(ESpO2,1);

       display.display();
       delay(100);


    
                                             //Deactivate the buzzer to have the effect of a "bip"
    //We sensed a beat!
    long delta = millis() - lastBeat;                   //Measure duration between two beats
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);           //Calculating the BPM

    if (beatsPerMinute < 255 && beatsPerMinute > 20)               //To calculate the average we strore some values (4) then do some math to calculate the average
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

}

  if (irValue < 7000){       //If no finger is detected it inform the user and put the average BPM to 0 or it will be stored for the next measure
     beatAvg=0;
     display.clearDisplay();
     display.setTextSize(2);                    
     display.setTextColor(WHITE);             
     display.setCursor(15,5);                
     display.println("Por favor "); 
     display.setCursor(25,25);
      display.println("coloque "); 
     display.setCursor(20,45);
     display.println("EL DEDAL ");  
     display.display();
     noTone(3);
     }
 
}
 //***********************************************************************

}
