#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // ENDERECO MAC DO ARDUINO
unsigned int localPort = 123;         // PORTA LOCAL PARA RECEBER OS PACOTES NTP CLIENT
const int NTP_PACKET_SIZE = 48;       // TAMANHO DO PACOTE NTP (APENAS NA PARTE DA APLICACAO NTP, DESCONSIDERANDO AUTENTIFICADOR)
byte packetBuffer[NTP_PACKET_SIZE];   // ONDE SERA ARMAZENADO O PACOTE NTP
byte teste[4];                        // ONDE SERA ARMAZENADO TIMESTAMP EM BYTES
unsigned long intValue;               // ONDE SERA ARMAZENADO O TIMESTAMP EM INTEIRO
EthernetUDP Udp;                      // INSTANCIA OBJETO UDP PARA RECEBER E ENVIAR PACOTES EM CIMA DO PROTOCOLO UDP
unsigned long horacerta;              // ONDE SERA ARMAZENADO O TIMESTAMP DO GPS
unsigned long lastUpdate;
byte ip[] = { 192, 168, 0, 235 }; 

double Datatransfer(char *data_buf,char num)//convert the data to the float type
{                                           //*data_buf：the data array                                       
  double temp=0.0;                           //the number of the right of a decimal point
  unsigned char i,j;
 
  if(data_buf[0]=='-')
  {
    i=1;
    //process the data array
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    //convert the int type to the float type
    for(j=0;j<num;j++)
      temp=temp/10;
    //convert to the negative numbe
    temp=0-temp;
  }
  else//for the positive number
  {
    i=0;
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    for(j=0;j<num;j++)
      temp=temp/10 ;
  }
  return temp;
}

 
char ID(){ 
  char i=0;
  char value[6]={'$','G','P','G','G','A'};//match the gps protocol
  char val[6]={'0','0','0','0','0','0'};
 
  while(1){
    if(Serial.available()){
      val[i] = Serial.read();//get the data from the serial interface
      if(val[i]==value[i]){    
        i++;
        if(i==6){
          i=0;
          return 1;//break out after get the command
        }
      }
      else
        i=0;
    }
  } 
}
 
void comma(char num){   
  char val;
  char count=0;//count the number of ','
 
  while(1){
    if(Serial.available()){
      val = Serial.read();
      if(val==',')
        count++;
    }
    if(count==num)//if the command is right, run return
      return;
  }
}

 

int UTC()//get the UTC data -- the time
{
  char i;
  char time[9]={
    '0','0','0','0','0','0','0','0','0'
  };
  double t=0.0;
 
  if( ID())//check ID
  {
    comma(1);//remove 1 ','
    //get the datas after headers
    while(1)
    {
      if(Serial.available())
      {
        time[i] = Serial.read();
        i++;
      }
      if(i==9)
      {
        i=0;
        t=Datatransfer(time,2);//convert data
        t=t-1800.00;//convert to the chinese time GMT+8 Time zone
        long time=t;
        int h=time/10000;
        int m=(time/100)%100;
        int s=time%100;
        
//        if(h>=24)               //UTC+
//        {
//        h-=24;
//        }
 
/*         if (h<0)               //UTC-
        {
          h+=24;
        }*/
        Serial.print(h);
        Serial.print("h");
        Serial.print(m);
        Serial.print("m");
        Serial.print(s);
        Serial.println("s");
 
        //Serial.println(t);//Print data 
        horacerta = (h * 3600) + (m * 60) + s;
        //return (h * 3600) + (m * 60) + s;
        Serial.println("utc:");
        Serial.println(horacerta);
        return horacerta;
      }  
    }
  }
}


void quality(){ //pega a qualidade
  char i;
  char lat[10]={'0','0','0','0','0','0','0','0','0','0'};
  
  if( ID()){
    comma(6);
    while(1){
      if(Serial.available()){
        lat[i] = Serial.read();
        i++;
      }
      if(i==10){
        i=0;
        Serial.println(Datatransfer(lat,5)/100.0,7);//print latitude 
        return;
      }  
    }
  }
}



void setup() {
  pinMode(3,OUTPUT);//The default digital driver pins for the GSM and GPS mode
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  delay(1500);
  digitalWrite(5,LOW);
  digitalWrite(3,LOW);//Enable GSM mode
  digitalWrite(4,HIGH);//Disable GPS mode
  delay(2000);
  Serial.begin(9600); 
  /*delay(5000);//GPS ready
  Serial.println("AT");   
  delay(2000);
  //turn on GPS power supply
  Serial.println("AT+CGPSPWR=1");
  delay(1000);
  //reset GPS in autonomy mode
  Serial.println("AT+CGPSRST=1");
  delay(1000);
  digitalWrite(4,LOW);//Enable GPS mode
  digitalWrite(3,HIGH);//Disable GSM mode
  delay(2000);
  Serial.println("$GPGGA statement information: ");*/
  // Open serial communications and wait for port to open:
  //Serial.begin(9600);
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
  
  //UTC();//////////////////////////////////////////////////////chamo a funcao pra atualizar a hora, preciso colocar em loop, de forma assicrona
}


void loop() {
  /*if((millis() - lastUpdate) > 10000){
    UTC();
    lastUpdate = millis();
  }*/
  
  
  if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // LE O PACOTE NTP RECEBIDO
      IPAddress remote = Udp.remoteIP();
      //intValue = 3720124210;
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      //Serial.print("Seconds since Jan 1 1900 = ");
      //Serial.println(secsSince1900);
      secsSince1900 = secsSince1900 / 86400; ///aqui eh uma gambiarra para pegar a data do cliente
      secsSince1900 = secsSince1900 * 86400;
      //Serial.println(secsSince1900);
      //intValue = secsSince1900 + horacerta + (lastUpdate - millis()); //// depois adicionou apenas a hora certa e mando de volta
      //intValue = secsSince1900 + UTC(); //// depois adicionou apenas a hora certa e mando de volta
      intValue = secsSince1900 + 1500; //// depois adicionou apenas a hora certa e mando de volta
      Serial.println(intValue);
      sendNTPpacket(remote, packetBuffer[40], packetBuffer[41], packetBuffer[42], packetBuffer[43], packetBuffer[44], packetBuffer[45], packetBuffer[46], packetBuffer[47], intValue);
      /*if((millis() - lastUpdate) > 10000){
        UTC();
        lastUpdate = millis();
      }*/
  }
  /*while(1){ 
    UTC();
    quality();
    Serial.println(' ');
    delay(2000);
  }*/

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









