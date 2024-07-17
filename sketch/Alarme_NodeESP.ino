#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi
const char* ssid = "nome_da_rede";
const char* password = "senha";

// NTP Server
const char* NTP_SERVER = "pool.ntp.org";
const int timeZone = -3;

WiFiUDP Udp;
unsigned int localPort = 8888;  // Porta local a ser usada para a conexao UDP

IPAddress ntpServerIP;

// inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------- icones
byte Sound[8] = {
  0b00001,
  0b00011,
  0b00101,
  0b01001,
  0b01001,
  0b01011,
  0b11011,
  0b11000
};

byte Bell[8] = {
  0b00100,
  0b01110,
  0b01110,
  0b01110,
  0b11111,
  0b00000,
  0b00100,
  0b00000
};

byte Speaker[8] = {
  0b00001,
  0b00011,
  0b01111,
  0b01111,
  0b01111,
  0b00011,
  0b00001,
  0b00000
};

byte Plus[8] = {
  0b00000,
  0b00100,
  0b00100,
  0b11111,
  0b11111,
  0b00100,
  0b00100,
  0b00000
};

byte Minus[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};

byte Relogio[8] = {
  0b01110,
  0b10001,
  0b10101,
  0b10101,
  0b10111,
  0b10001,
  0b10001,
  0b01110
};

byte Relogio1[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00111,
  0b01000,
  0b10001,
  0b10001,
  0b10001
};

byte Relogio2[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b11000,
  0b00100,
  0b00010,
  0b00010,
  0b11010
};

byte Relogio3[8] = {
  0b10000,
  0b10000,
  0b01000,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte Relogio4[8] = {
  0b00010,
  0b00010,
  0b00100,
  0b11000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
// ----------------- fim icones

void criaIcones() {
  lcd.createChar(0, Sound);
  lcd.createChar(1, Bell);
  lcd.createChar(2, Speaker);
  lcd.createChar(3, Plus);
  lcd.createChar(4, Minus);
  lcd.createChar(5, Relogio);
  lcd.createChar(6, Relogio1);
  lcd.createChar(7, Relogio2);
  lcd.createChar(8, Relogio3);
  lcd.createChar(9, Relogio4);
}

void ligaLCD() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
}

void conectaNaInternet() {
  int i = 10;

  WiFi.begin(ssid, password);

  Serial.print("Conectando a Rede: ");
  Serial.println(ssid);

  lcd.setCursor(0, 0);
  lcd.print(ssid);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print("Conectando");

    if(i < 15) {
      lcd.setCursor(i, 1);
      lcd.print(".");
      i++;
    } else {
      i = 10;
      lcd.setCursor(i, 1);
      lcd.print("      ");
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Finalmente");
  lcd.setCursor(0, 1);
  lcd.print("Conectado!");

  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void configNTP() {
  Udp.begin(localPort);
  Serial.println("Iniciando cliente UDP");

  Serial.print("Conectando a ");
  Serial.println(NTP_SERVER);

  IPAddress dns(8, 8, 8, 8);  // Exemplo de servidor DNS do Google
  WiFi.config(IPAddress(192, 168, 31, 100), IPAddress(192, 168, 31, 1), IPAddress(255, 255, 255, 0), dns);

  WiFi.hostByName(NTP_SERVER, ntpServerIP);

  Serial.print("Servidor NTP encontrado em: ");
  Serial.println(ntpServerIP);

  setSyncProvider(getNtpTime);
  setSyncInterval(300);  // Atualizar a hora a cada 300 segundos (5 minutos)
}

time_t getNtpTime() {
  const int NTP_PACKET_SIZE = 48;     // tamanho do pacote NTP
  byte packetBuffer[NTP_PACKET_SIZE]; // buffer para armazenar o pacote NTP

  // monta o pacote NTP
  memset(packetBuffer, 0, NTP_PACKET_SIZE); // limpa o buffer

  packetBuffer[0]  = 0b11100011; // Versao do NTP, modo cliente
  packetBuffer[1]  = 0;          // Numero de stratum
  packetBuffer[2]  = 6;          // Intervalo maximo entre atualizacoes
  packetBuffer[3]  = 0xEC;       // Clock estrato
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // enviar pacote NTP para o servidor
  Udp.beginPacket(ntpServerIP, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();

  // aguardar resposta do servidor NTP
  delay(1000);

  // verifica se ha dados recebidos
  if(Udp.parsePacket()) {
    // le a resposta do servidor NTP
    Udp.read(packetBuffer, NTP_PACKET_SIZE);

    // converte os 4 bytes da resposta do timestamp para um long
    unsigned long secsSince1900 = (unsigned long) packetBuffer[40] << 24 |
                                  (unsigned long) packetBuffer[41] << 16 |
                                  (unsigned long) packetBuffer[42] << 8  |
                                  (unsigned long) packetBuffer[43];

    // tempo unix comeca em 1 de janeiro de 1970
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;

    return epoch + timeZone * SECS_PER_HOUR;  // Ajuste para o fuso horário correto
  }

  return 0; // se nao conseguir a hora
}

void telaPrincipal(time_t time) {
  char buffer[20];

  lcd.clear();

  // icone relogio
  lcd.setCursor(0, 0);
  lcd.write(6);
  lcd.setCursor(1, 0);
  lcd.write(7); 
  lcd.setCursor(0, 1);
  lcd.write(8); 
  lcd.setCursor(1, 1);
  lcd.write(9); 

  // hora
  lcd.setCursor(3, 0);
  lcd.print("Hora: ");
  sprintf(buffer, "%02d:%02d", hour(time), minute(time));
  lcd.setCursor(9, 0);
  lcd.print(buffer);

  // data
  sprintf(buffer, "%02d/%02d/%02d", day(time), month(time), year(time));
  lcd.setCursor(3, 1);
  lcd.print(buffer);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  ligaLCD();
  criaIcones();
  conectaNaInternet();
  configNTP();
}

void loop() {
  // espera ate que o tempo esteja sincronizado
  if(timeStatus() != timeNotSet) {
    time_t horaAtual = now();
    telaPrincipal(horaAtual);
  }

  delay(1000);
}
