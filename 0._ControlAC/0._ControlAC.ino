//Librerías para SD
#include <SPI.h>
#include <SD.h>
const int chipSelect = 10; //Pin de selección de la SD

//Librerías para RTC
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;    //Se declara un objeto RTC

//Librerías Sensores
#include "DHT.h"
#include "EmonLib.h"
DHT senInt(2, DHT22); //Sensor de temperatura interna (Pin digital, Tipo de DHT)
DHT senExt(3, DHT11); //Sensor de temperatura externa
EnergyMonitor velectric;        // Se declara un objeto Energy Monitor
double ip = 30, is = 0.015, rburden = 120; //Datos para la calibración del sensor de corriente
float humInt = 0.0;       //Variable Humedad interna
float tempInt = 0.0;      //variable Temperatura interna
float humExt = 0.0;       //Variable Humedad externa
float tempExt = 0.0;      //Variable Temperatura externa
float Pact = 0.0;         //Variable Potencia activa o real
float Papa = 0.0;         //Variable Potencia aparente
float FP = 0.0;           //Variable Factor de Potencia
float Vrms = 0.0;         //Variable Voltaje RMS
float Irms = 0.0;         //Variable Corriente RMS
unsigned long lastTime = 0.0;     //Tiempo inicial
unsigned long timeNow = 0.0;      //Tiempo actual
unsigned long storageTime = 1.0;  //Tiempo de espera para guardar en minutos (máximo 60 minutos)

void setup() {

  SD.begin(chipSelect, 11, 12, 13);

  rtc.begin();

  //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Ajusta la fecha y la hora del RTC


  //Calibración del sensor de corriente y voltaje
  double calibration = ((ip / is) / rburden);
  velectric.current(0, calibration);                 //Corriente: (Pin de lectura, Calibración)
  velectric.voltage(1, 112, 1.7);                    //Voltaje: (Pin de lectura, calibración, Desfase)
  //Fin calibración del sensor de corriente y voltaje
}

void loop() {

  energy();
  climate();
  timer();
}

//Función para el alamacenamiento de los datos del monitoreo:
void storage() {

  DateTime now = rtc.now();

  File dataFile = SD.open("Mon1.txt", FILE_WRITE);

  // Escribiendo en la SD:

  if (dataFile) {

    //1. Imprimir la fecha y la hora:

    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print("  ");
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    dataFile.print(now.second(), DEC);
    dataFile.print("  ");

    //2. Imprimir los valores del monitoreo:

    dataFile.print(humInt);
    dataFile.print("  ");
    dataFile.print(tempInt);
    dataFile.print("  ");
    dataFile.print(humExt);
    dataFile.print("  ");
    dataFile.print(tempExt);
    dataFile.print("  ");
    dataFile.print(Vrms);
    dataFile.print("  ");
    dataFile.println(Irms);
    
    //3. Cerrar el archivo en la SD para evitar pérdida de información
    
    dataFile.close();
  }
}

//Función para el monitoreo de la corriente y el voltaje:
void energy() {
  for (int i = 0; i >= 5; i++) {
    velectric.calcVI(50000000, 1000);        // Calcula corriente y voltaje
  }
  velectric.calcVI(50000000, 2000);
  Vrms = velectric.Vrms;
  Irms = velectric.Irms;
}

//Función para el monitoreo de la temperatura interna y externa:
void climate() {

  humInt = senInt.readHumidity();
  tempInt = senInt.readTemperature();

  humExt = senExt.readHumidity();
  tempExt = senExt.readTemperature();

}

//Función para guardar datos cada cierto tiempo:
void timer() {

  DateTime now = rtc.now();
  timeNow = now.minute();

  if ((timeNow - lastTime) >= storageTime) {
    storage();
    if (timeNow >= 59) {
      lastTime = -1;
    }
    else {
      lastTime = timeNow;
    }
  }

}



