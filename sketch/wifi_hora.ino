#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA_DA_REDE";

const long utcOffsetInSeconds = -10800;

char diasDaSemana[7][8] = {"DOMINGO", "SEGUNDA", "TERCA", "QUARTA", "QUINTA", "SEXTA", "SABADO"};

// define NTP Client para pegar o tempo
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "south-america.pool.ntp.org", utcOffsetInSeconds);

// cria o objeto do LCD (HEX address, characters, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  delay(500);

  // lcd
  lcd.init();
  lcd.backlight();
  
  // wifi
  conectaNoWiFi();

  // time
  timeClient.begin();
}

void loop() {
  timeClient.update();

  escreveHora();
  
  delay(1000);
}

void conectaNoWiFi() {
  WiFi.begin(ssid, password);

  lcd.setCursor(0, 0);
  lcd.print("Conectando ao");
  lcd.setCursor(0, 1);
  lcd.print("Wi-Fi");
  
  int i = 0;

  while(WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectado!");
  delay(1000);
}

void escreveHora() {
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  int dia = ptm->tm_mday;
  int mes = ptm->tm_mon+1;
  int ano = ptm->tm_year+1900;

  String dataAtual = String(dia) + "/" + String(mes) + "/" + String(ano);

  Serial.print("DIA: ");
  Serial.println(dia);
  Serial.print("MES: ");
  Serial.println(mes);
  Serial.print("ANO: ");
  Serial.println(ano);
  
  lcd.clear();

  // dia/mes/ano
  lcd.setCursor(0, 0);
  lcd.print(dataAtual);  

  // HH:MM:SS dia da semana
  lcd.setCursor(0, 1);
  lcd.print(timeClient.getFormattedTime());
  lcd.setCursor(9, 1);
  lcd.print(diasDaSemana[timeClient.getDay()]);
}