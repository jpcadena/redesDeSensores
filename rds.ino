#include "DHT.h"
#include "SoftwareSerial.h"
#define RX 10
#define TX 11
#define DHTPIN 4
#define DHTTYPE DHT22

/*Inicializamos el sensor DHT11*/
DHT dht(DHTPIN, DHTTYPE);

/*se generan las variables referentes a la red ad-hoc y su clave con el nombre del servidor*/
String ssid = ""; /*nombre de la red*/
String password = ""; /*contraseña de la red*/
String host = ""; /*direccion ip del servidor q aloja la base de datos dentro de la misma red*/
int countTrueCommand, countTimeCommand;
boolean found = false;
unsigned long previousMillis = 0;
const long interval = 16000;
int sct013 = 0;
float refVoltage = 1.1;
float bitResolution = 1023.0;
float vcRelation = 30.0;
int cycle = 500; //Duración 0.5 segundos(Aprox. 30 ciclos de 60Hz)
float acVoltage = 220.0;


/*se establece conexion mediante comandos AT del ESP8266*/
SoftwareSerial wifi(RX,TX); 
void sendCommand(String command, int maxTime, char readReplay[]);

void setup() {
  Serial.begin(9600);
  analogReference(INTERNAL);
  dht.begin();
  wifi.begin(9600);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ ssid +"\",\""+ password +"\"",20,"OK");
}

void loop() {
 /*se declaran las variables de interes para calculos posteriores*/
  unsigned long currentMillis = millis();
  int indice = 0;
  String url = "/redesDeSensores.php"; /*nombre del php alojado en el servidor q ingresa y muestra los datos  en la pagina web*/
  String dato1 = "?Temperatura=";
  String dato2 = "&Potencia=";
  float currentRMS=getCurrent(); //Corriente eficaz (A)
  float power=currentRMS*acVoltage;
  // Leemos la temperatura en grados centígrados
  float temp = dht.readTemperature();
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(temp)) {
    Serial.println("Error obteniendo los datos del sensor DHT22");
    return;
  }
  Serial.print("Temperatura: ");
  Serial.print(temp);
  Serial.print("°C, Irms: ");
  Serial.print(currentRMS);
  Serial.print("A, Potencia: ");
  Serial.print(power);  
  Serial.println("W");
  delay(10000);

  /* Subiendo datos cada 15 segundos*/
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    String getData = "GET " + url + dato1 + temp + dato2 + power + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n";
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ host +"\","+ 80,10,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+0),3,">");
    wifi.println(getData);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");
  }
}

float getCurrent()
{
  float voltage;
  float current=0;
  float sum=0;
  long tiempo=millis();
  int N=0;
  while(millis()-tiempo<cycle)
  { 
    voltage = analogRead(sct013) * (refVoltage / bitResolution); //voltaje del sensor
    current = voltage * vcRelation; //corriente=VoltajeSensor*(30A/1V)
    sum=sum+sq(current);//Sumatoria de Cuadrados
    N=N+1;
    delay(1);
  }
  sum=sum*2;//Para compensar los cuadrados de los semiciclos negativos.
  current=sqrt((sum)/N); //ecuación del RMS
  return(current);
}

// Envío de comandos al módulo WIFI
void sendCommand(String command, int maxTime, char readReplay[]){
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    wifi.println(command);//at+cipsend
    delay(1000);
    if(wifi.find(readReplay))
    {
      found = true;
      break;
    }  
    countTimeCommand++;
  }  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }  
  found = false;
}
