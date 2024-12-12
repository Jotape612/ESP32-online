#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <LiquidCrystal.h>
#include "HX711.h"

// Configurações do Firebase
#define FIREBASE_HOST "https://esp32-6fe25-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "YZ1HtCti4AYGSNsOgiGFhzpqZxrcPm7hEthFDJUQ"
#define WIFI_SSID "teste1"
#define WIFI_PASSWORD "123456"

// Pinos do LCD e Load Cell
LiquidCrystal lcd(26, 25, 33, 32, 35, 34);
#define LEDR 13
#define LEDG 12
#define DT_LC 27
#define SCK_LC 14

#define PESO_MAXIMO 50.0

HX711 scale;
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("WiFi Conectado!");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  scale.begin(DT_LC, SCK_LC);
  scale.set_scale(470);
  scale.tare();

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  lcd.begin(16, 2);
}

void atualizarFirebase() {
  float peso = scale.get_units(10);
  if (peso < 0) peso = 0;

  int porcentagem = (peso * 1.125 / PESO_MAXIMO) * 100;
  porcentagem = constrain(porcentagem, 0, 100);

  Firebase.setFloat(firebaseData, "/Prateleira/pesoAtual", peso);
  Firebase.setInt(firebaseData, "/Prateleira/porcentagem", porcentagem);

  if (firebaseData.error()) {
    Serial.println("Erro ao enviar para o Firebase: " + firebaseData.errorReason());
  } else {
    Serial.println("Dados enviados para o Firebase!");
  }

  //O ? representa if-else, isto é, ele está basicamente me dizendo "IF porcentagem <= 25 (%) HIGH, ELSE (então) Low"
  digitalWrite(LEDR, porcentagem <= 25 ? HIGH : LOW);
  digitalWrite(LEDG, porcentagem > 25 ? HIGH : LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Peso: ");
  lcd.print(peso, 2);
  lcd.print(" kg");
  lcd.setCursor(0, 1);
  lcd.print("Estoque: ");
  lcd.print(porcentagem);
  lcd.print("%");
}

// Função Get_Firebase
void obterDadosFirebase() {
  String nome = Firebase.getString(firebaseData, "/Prateleira/nome");
  int capacidade = Firebase.getInt(firebaseData, "/Prateleira/capacidade");
  float pesoAtual = Firebase.getFloat(firebaseData, "/Prateleira/pesoAtual");
  int porcentagem = Firebase.getInt(firebaseData, "/Prateleira/porcentagem");

  if (firebaseData.error()) {
    Serial.println("Erro ao obter dados do Firebase: " + firebaseData.errorReason());
  } else {
    Serial.println("Prateleira: " + nome);
    Serial.println("Capacidade Limite: " + String(capacidade));
    Serial.println("Peso Atual: " + String(pesoAtual, 2) + " kg");
    Serial.println("Porcentagem: " + String(porcentagem) + "%");
  }
}

void loop() {
  atualizarFirebase();

  obterDadosFirebase();

  delay(5000); 
}
