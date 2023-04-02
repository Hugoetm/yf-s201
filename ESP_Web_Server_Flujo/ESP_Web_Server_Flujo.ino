//-----Librerias------------
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
//--------------------------


//-----------Red a utlizar--------------
const char* ssid = "red";
const char* password = "Contraseña";
//--------------------------------------

// Creamos el servidor 
AsyncWebServer server(80);

//-----------------variables de flujo-------------
#define SENSOR_PIN 4             // Pin digital utilizado para la lectura del sensor
#define INTERRUPT_PIN 16         // Pin digital utilizado para la interrupción del sensor
volatile byte pulses;            // Variable para almacenar el número de pulsos
float FlowRate;                  // Variable para almacenar el caudal
unsigned int flowMilliLitres;    // Variable para almacenar el volumen
unsigned long totalMilliLitres;  // Variable para almacenar el volumen total
unsigned long oldTime;           // Variable para almacenar el tiempo anterior
//------------------------------------------------

void setup() {
  // iniciamos SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
    
 // Configuración del pin de lectura como entrada con resistencia pull-up
  pinMode(SENSOR_PIN, INPUT_PULLUP);                                            
  // Configuración del pin de interrupción como entrada con resistencia pull-up
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);   
  // Habilitación de la interrupción en el pin correspondiente
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pulseCounter, FALLING); 
  pulses = 0;
  FlowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  oldTime = 0;

  // activamos el puerto serial
  Serial.begin(115200);

  // Conexion a internet
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // print esp32 local IP address
  Serial.println(WiFi.localIP());


    
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html");
  });

  // Route to get the flow rate
  server.on("/flowrate", HTTP_GET, [](AsyncWebServerRequest* request) {
    float flowRate = FlowRate;
    request->send_P(200, "text/plain", String(flowRate).c_str());
  });

  // Start server
  server.begin();
}

void loop() {
  Flujo();
}


void Flujo(){
  unsigned long currentTime = millis();  // Obtención del tiempo actual
  if (currentTime - oldTime > 1000)      // Si han pasado más de 1 segundo desde la última medición
  {
    detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));                         // Deshabilitación de la interrupción
    FlowRate = ((1000.0 / (currentTime - oldTime)) * pulses) / 7.5;                // Cálculo del caudal en L/min
    oldTime = currentTime;                                                         // Actualización del tiempo anterior
    flowMilliLitres = (FlowRate / 60) * 1000;                                      // Cálculo del volumen en mL
    totalMilliLitres += flowMilliLitres;                                           // Actualización del volumen total
    pulses = 0;                                                                    // Reinicio del contador de pulsos
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pulseCounter, FALLING);  // Habilitación de la interrupción
  }
  Serial.print("Caudal: ");
  Serial.print(FlowRate);
  Serial.print(" L/min\tVolumen: ");
  Serial.print(totalMilliLitres);
  Serial.println(" mL");
  delay(500);  // Retardo para evitar saturar el puerto serie
}
void pulseCounter() {
  pulses++;  // Incremento del contador de pulsos
}
