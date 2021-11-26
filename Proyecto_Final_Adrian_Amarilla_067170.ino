/*************************Librerias a utilizar***********************************/
#include <WiFi.h> 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
/*************************Variables del dht11***********************************/
#include <DHT.h> //Cargamos la librería DHT
#define DHTTYPE DHT11 //Definimos el modelo del sensor DHT11
#define DHTPIN 4 // Se define el pin 4 del ESP32 para conectar el pin de datos del DHT11
DHT dht(DHTPIN, DHTTYPE, 11); 
/********************Parametros de conexion Wifi*******************************/
#define WLAN_SSID "WLAN_SSID"  // Ingresa el nombre de tu red         
#define WLAN_PASS "WLAN_PASS"  // Ingresa la contraseña de tu red 
/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // Use 8883 para SSL
#define AIO_USERNAME "AIO_USERNAME" // Reemplace con su nombre de usuario
#define AIO_KEY "AIO_KEY" // Reemplace con su Clave de autenticación
/************ LED **************/
#define LED 2 //Se define el led 2 azul del ESP32
/************ Estado global (¡no necesita cambiar esto!) *********************/
WiFiClient client; // Crea una clase WiFiClient para conectarse al servidor MQTT
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY); // Configuración cliente MQTT pasando, cliente WiFi y servidor MQTT y los detalles de inicio de sesión.
/***************** Feeds que se definieron en Adafruit IO *******************/
// Configuracion para publicar los feeds
Adafruit_MQTT_Publish temperatura = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME"/feeds/TempRefri");
Adafruit_MQTT_Publish humedad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME"/feeds/HumRefri");
//Configuracion para recibir de los feeds
Adafruit_MQTT_Subscribe LED_ONOFF = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/led");
/***************************************************************************/

void MQTT_connect();

void setup() {
  pinMode(DHTPIN, INPUT);
  Serial.begin(115200);
  delay(100);
/*************************Conexión a la red wifi**************************/
Serial.println();
Serial.print("Conectando a ");                                            //Mensaje en la consola serial de la conexion WiFi (Titulo)
Serial.println(WLAN_SSID);                                                //Mensaje en la consola serial de la conexion WiFi-->SSID Conectado

WiFi.mode(WIFI_STA);
WiFi.begin(WLAN_SSID, WLAN_PASS);                                         //Se introducen las credenciales para la conexion WiFi

//Espera a que el WiFi se conecte correctamente
uint32_t notConnectedCounter = 0;
while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando al WiFi...");
    notConnectedCounter++;
    if(notConnectedCounter > 15) {                                        // Hacemos un reset del ESP32 si es que no se conecta a la red WiFi luego de 15 segundos 
        Serial.println("Reseteando ESP32 por conexion fallida del WiFi");
        ESP.restart();
    }
}
Serial.println("WiFi conectado");
Serial.println("IP address: "); 
Serial.println(WiFi.localIP());

mqtt.subscribe(&LED_ONOFF);
}
/****************************************************************************/

void loop(){
  
MQTT_connect();
/*********************** LED ON % OFF ***************************/
pinMode(LED,OUTPUT);                                  //Definimos el modo del pin, que sea de salida
Adafruit_MQTT_Subscribe *subscription;
subscription = mqtt.readSubscription();               //Leemos el ultimo estado del Feed LED_ONOFF

Serial.print(F("\nDatos del Refrigerador: ")); 
Serial.print(F("\nIluminación: "));                   //Mensaje en la consola serial del estado (Titulo)
Serial.println((char *)LED_ONOFF.lastread);           //Mensaje en la consola serial de la ultima lectura del feed LED_ONOFF

//Comprobaciones del estado del Feed LED_ONOFF
if (strcmp((char *)LED_ONOFF.lastread, "ON") == 0) {  //Comparacion si la ultima lectura del feed LED_ONOFF es ON, si lo es, enciende el led del ESP32
  digitalWrite(LED, HIGH); }
if (strcmp((char *)LED_ONOFF.lastread, "OFF") == 0) { //Comparacion si la ultima lectura del feed LED_ONOFF es OFF, si lo es, apaga el led del ESP32
  digitalWrite(LED, LOW); }
/********************** Sensor Temperatura % Humedad ************************/
float tem = dht.readTemperature();                   //Se lee la temperatura
float h = (dht.readHumidity())/10;                   //Se lee la humedad y la divimos entre 10 para tenerlo en porcentaje

/*float hic = dht.computeHeatIndex(tem, h, false);   //Opcion de lectura de Temperatura y Humedad a la vez*/

Serial.print(F("\n Temperatura: "));                  //Mensaje en la consola serial del estado de la lectura (Titulo-Temperatura)
Serial.print(tem);                                    //Mensaje en la consola serial de la ultima lectura del sensor de Temperatura en Celsius

if (! temperatura.publish(tem)) {                     //Verificamos si se publico la lectura del sensor de temperatura
Serial.println(F("Lectura de Temperatura Fallida"));  //Si no se publico, lanzamos un mensaje del error.
} else {
Serial.print(F("°C"));                                //Adjutamos al mensaje de la consola serial especificando que la lectura de Temperatura es en Celsius
}

Serial.print(F("\n Humedad: "));                      //Mensaje en la consola serial del estado de la lectura (Titulo-Humedad)
Serial.print(h);                                      //Mensaje en la consola serial de la ultima lectura del sensor de Humedad

if (! humedad.publish(tem)) {                         //Verificamos si se publico la lectura del sensor de humedad
Serial.println(F("Lectura de Humedad Fallida"));      //Si no se publico, lanzamos un mensaje del error.
} else {
Serial.print(F("%"));                                 //Adjutamos al mensaje de la consola serial especificando que la lectura de Humedad es un porcentaje
Serial.print(F("\n"));                                //Adjuntamos un salto para la siguiente impresion
}

 delay(4000);                                         //Van a transcurrir 4 segundos para que se vuelva a ejecutar el LOOP()
}

void MQTT_connect() {
int8_t ret;
// Detener si ya está conectado
if (mqtt.connected()) {
return;
}

Serial.print("Conectando a MQTT... ");

uint8_t retries = 3;

while ((ret = mqtt.connect()) != 0) { // connect devolverá 0 para conectado
Serial.println(mqtt.connectErrorString(ret));
Serial.println("Re-intentando la conexión MQTT en 5 segundos...");
mqtt.disconnect();
delay(5000); // espera 5 segundos
retries--;
if (retries == 0) {
// La conexión falla y espera a que se reinicie
while (1);
}
}
Serial.println("MQTT Conectado!"); 
}
