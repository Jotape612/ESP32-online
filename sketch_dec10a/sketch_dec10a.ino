#include <LiquidCrystal.h>
#include <WiFi.h>
#include "HX711.h"

#include <ArduinoJson.h>
#include <IOXhop_FirebaseESP32.h>


#define FIREBASE_HOST "https://esp32-6fe25-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH  
// Pinos do LCD (RS, E, D4, D5, D6, D7) - ESP32
LiquidCrystal lcd(26, 25, 33, 32, 35, 34);

// Pinos dos LEDs e Load Cell
#define LEDR 13  // LED Vermelho
#define LEDG 12  // LED Verde
#define DT_LC 27 // Data do HX711
#define SCK_LC 14 // Clock do HX711

#define PESO_MAXIMO 50.0 // Peso máximo da balança em kg

// Credenciais Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

WiFiServer server(80); // Servidor HTTP na porta 80
HX711 scale;

void setup() {
  // Configuração dos LEDs como saída
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);

  // Inicialização do LCD
  lcd.begin(16, 2);

  // Configuração da comunicação serial
  Serial.begin(115200);

  // Inicialização do HX711
  scale.begin(DT_LC, SCK_LC);
  scale.set_scale(470); // Ajustar o valor da escala conforme necessário
  scale.tare(); // Zerando a balança

  // Conexão ao Wi-Fi
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  server.begin(); // Inicializa o servidor
}

void loop() {
  // Leitura do peso na balança
  float valor_balanca = scale.get_units();
  if (valor_balanca < 0) valor_balanca = 0; // Evitar valores negativos

  // Cálculo da porcentagem do estoque
  int porcentagem_estoque = (valor_balanca * 1.125 / PESO_MAXIMO) * 100;
  porcentagem_estoque = constrain(porcentagem_estoque, 0, 100); 

  // Controle dos LEDs com base na porcentagem
  if (porcentagem_estoque <= 25) {
    digitalWrite(LEDR, HIGH); // Acende o LED vermelho
    digitalWrite(LEDG, LOW);  // Apaga o LED verde
  } else {
    digitalWrite(LEDR, LOW);  // Apaga o LED vermelho
    digitalWrite(LEDG, HIGH); // Acende o LED verde
  }

  // Atualização do LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Peso: ");
  lcd.print(valor_balanca * 1.119, 2);  
  lcd.print(" kg");
  lcd.setCursor(0, 1);
  lcd.print("Estoque: ");
  lcd.print(porcentagem_estoque);
  lcd.print("%");

  // Envio para o Serial Monitor
  Serial.print("Peso: ");
  Serial.print(valor_balanca * 1.119, 2);  
  Serial.print(" kg | Estoque: ");
  Serial.print(porcentagem_estoque);
  Serial.println("%");

  // Verifica conexões HTTP e responde
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Cliente conectado");
    String request = client.readStringUntil('\r');
    client.flush();

    // Resposta HTTP
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    client.print("<!DOCTYPE HTML>");
    client.print("<html>");
    client.print("<h1>Dados da Balança</h1>");
    client.print("<p>Peso: ");
    client.print(valor_balanca * 1.119, 2);
    client.print(" kg</p>");
    client.print("<p>Estoque: ");
    client.print(porcentagem_estoque);
    client.print("%</p>");
    client.print("</html>");
    client.stop();
    Serial.println("Cliente desconectado");
  }

  delay(2000); // Intervalo entre leituras
}
