#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // ENDERECO MAC DO ARDUINO

unsigned int localPort = 123;         // PORTA LOCAL PARA RECEBER OS PACOTES NTP CLIENT
const int NTP_PACKET_SIZE = 48;       // TAMANHO DO PACOTE NTP (APENAS NA PARTE DA APLICACAO NTP, DESCONSIDERANDO AUTENTIFICADOR)
byte packetBuffer[NTP_PACKET_SIZE];   //ONDE SERA ARMAZENADO O PACOTE NTP
byte teste[4];                        // ONDE SERA ARMAZENADO TIMESTAMP EM BYTES
unsigned long intValue;               // ONDE SERA ARMAZENADO O TIMESTAMP EM INTEIRO
EthernetUDP Udp;                      // INSTANCIA OBJETO UDP PARA RECEBER E ENVIAR PACOTES EM CIMA DO PROTOCOLO UDP

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
  Udp.begin(localPort);
}


void loop() {
  if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // LE O PACOTE NTP RECEBIDO
      IPAddress remote = Udp.remoteIP();
      intValue = 3720124210;
      sendNTPpacket(remote, packetBuffer[40], packetBuffer[41], packetBuffer[42], packetBuffer[43], packetBuffer[44], packetBuffer[45], packetBuffer[46], packetBuffer[47], intValue);
  }

  Ethernet.maintain();
}

// ENVIA DE VOLTA O PACOTE NTP SERVER
void sendNTPpacket(IPAddress address, byte recebido0, byte recebido1, byte recebido2, byte recebido3, byte recebido4, byte recebido5, byte recebido6, byte recebido7, unsigned long valor) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  teste[0] = (byte)(valor >> 24);
  teste[1] = (byte)(valor >> 16);
  teste[2] = (byte)(valor >> 8);
  teste[3] = (byte)valor;
  /*Serial.println(teste[0]);
  Serial.println(teste[1]);
  Serial.println(teste[2]);
  Serial.println(teste[3]);*/
  
  packetBuffer[0] = 0b00011100;   // LI, Version, Mode <<<<< MUDEI AQUI PARA MODO SERVER (100, SERVER), (011, CLIENT), mudei li para no warning, mudei ntp para versao 3
  packetBuffer[1] = 6;            // Stratum, or type of clock <<<<<<<< REFERENTE AO TIPO (OU NIVEL) DE SERVIDOR NTP
  packetBuffer[2] = 0x0A;         // Polling Interval
  packetBuffer[3] = 0xE9;         // Peer Clock Precision troquei de ec para e9

  
  packetBuffer[4] = 0x00; /////////////ROOT DELAY
  packetBuffer[5] = 0x01;
  packetBuffer[6] = 0x08;
  packetBuffer[7] = 0x56;
  
  packetBuffer[8] = 0x00; //////////// ROOT DISPERSION
  packetBuffer[9] = 0x05;
  packetBuffer[10] = 0x0C;
  packetBuffer[11] = 0xD2;
  
  packetBuffer[12]  = 0xC8; ///
  packetBuffer[13]  = 0xBA; ///
  packetBuffer[14]  = 0x7D; /// reference id
  packetBuffer[15]  = 0xC3; ///


  packetBuffer[16] = teste[0];   /////////// REFERENCE TIMESTAMP <- TIMESTAMP REFERENTE AO HORARIO EM QUE O SERVER NTP TEVE SEU RELOGIO SINCRONIZADO
  packetBuffer[17] = teste[1];   /////////// COLOCO COM 2 SEG DE ATRASO SO PRA PASSAR A IDEIA QUE O SERVER TEM SEU RELOGIO ATUALIZADO SEMPRE
  packetBuffer[18] = teste[2];
  packetBuffer[19] = teste[3] - 2;
  
  packetBuffer[20] = 0xF3; /////////// REFERENCE TIMESTAMP (PARTE REFERENTE A FRACAO DE SEGUNDOS)
  packetBuffer[21] = 0x3E; /////////// ESTOU IGNORANDO E SO MANDANDO UNS NUMERO QUALQUER
  packetBuffer[22] = 0x18;
  packetBuffer[23] = 0xD6;

  
  packetBuffer[24] = recebido0;  /////////////// ORIGINATE TIMESTAMP <- TIMESTAMP REFERENTE AO HORARIO EM QUE A SOLICITACAO NTP FOI CRIADA PELO CLIENTE
  packetBuffer[25] = recebido1;  /////////////// ESTOU PEGANDO O PACOTE RECEBIDO E COPIANDO A PARTE DO TRANSMIT TIMESTAMP DO CLIENTE PARA SER O ORIGINATE TIMESTAMP DO SERVER
  packetBuffer[26] = recebido2;
  packetBuffer[27] = recebido3;
  packetBuffer[28] = recebido4;
  packetBuffer[29] = recebido5;
  packetBuffer[30] = recebido6;
  packetBuffer[31] = recebido7;


  packetBuffer[32] = teste[0]; //////////////// RECEIVE TIMESTAMP <- TIMESTAP REFERENTE AO HORARIO EM QUE A SOLICITACAO CHEGOU
  packetBuffer[33] = teste[1]; //////////////// ESTOU USANDO O MESMO DO TRANSMIT (PRA FINGIR QUE TRANSMITE QUASE NO MESMO TEMPO QUE RECEBE)
  packetBuffer[34] = teste[2];
  packetBuffer[35] = teste[3];
  
  packetBuffer[36] = 0xEF; //////////////// RECEIVE TIMESTAMP (PARTE REFERENTE A FRACAO DE SEGUNDOS)
  packetBuffer[37] = 0xCB; //////////////// ESTOU IGNORANDO E SO MANDANDO UNS NUMERO QUALQUER
  packetBuffer[38] = 0x8F;
  packetBuffer[39] = 0xF2;

  packetBuffer[40] = teste[0]; ///////////////// TRANSMIT TIMESTAMP <- AQUI É ONDE VAI O HORARIO CERTO
  packetBuffer[41] = teste[1]; ///////////////// TIMESTAMP EM SEGUNDOS DESDE DE 1 JAN 1900 CONVERTIDO DE DECIMAL EM HEX
  packetBuffer[42] = teste[2];
  packetBuffer[43] = teste[3];
  
  packetBuffer[44] = 0xEF; ///////////////// TRANSMIT TIMESTAMP (PARTE DA FRACAO DE SEGUNDOS)
  packetBuffer[45] = 0xCD; ///////////////// ESTOU IGNORANDO E SO MANDANDO UNS NUMERO QUALQUER
  packetBuffer[46] = 0xD8;
  packetBuffer[47] = 0x44;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}









