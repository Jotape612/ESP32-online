#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <ArduinoJson.h>
#include <HX711.h>

#define WIFI_SSID "Bel infoneT_Apolonio"
#define WIFI_PASSWORD "marcador123@"
#define FIREBASE_HOST "https://esp32-6fe25-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "bI68m0iE1rOM0zij1b6VSwtQ14r6YZ3K747dfdkN"

#define LEDR 13
#define LEDG 12
#define LEDY 14
#define DT_LC 27
#define SCK_LC 14
#define PESO_MAXIMO 50.0

HX711 scale;

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Conexão WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi conectado!");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  scale.begin(DT_LC, SCK_LC);
  scale.set_scale(470); 
  scale.tare();

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDY, OUTPUT);
}

void loop() {
  float pesoAtual = scale.get_units(10); 
  if (pesoAtual < 0) pesoAtual = 0; 

  int porcentagem = (pesoAtual / PESO_MAXIMO) * 100;
  porcentagem = constrain(porcentagem, 0, 100);

  Firebase.setFloat("/Prateleira/pesoAtual", pesoAtual);
  Firebase.setInt("/Prateleira/porcentagem", porcentagem);


  if (porcentagem <= 30) {
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDG, LOW);
  } else if (porcentagem <= 70) {
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDY, HIGH);
    digitalWrite(LEDG, LOW);
  } else {
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDG, HIGH);
  }

  delay(3000);
}
