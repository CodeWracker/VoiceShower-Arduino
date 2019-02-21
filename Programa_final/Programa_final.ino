
//Autor: Rodrigo Ferraz Souza
#include <SD.h>
#include <TMRpcm.h>
#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"
#include <OneWire.h>
#include <DallasTemperature.h>
/*  Conexão modulo sd
 *  MISO 12
 *  MOSI 11
 *  SCK 13
 *  CS 4
 *    
 *  Conexão VoiceRecognitionModule
 *  7   ------->     TX
 *  8   ------->     RX
*/
VR myVR(7,8);                                                             // 7:TX 8:RX, ta certo
SoftwareSerial VR3(7,8);

TMRpcm audioCartaoSD;                                                     //Cria a saida de audio

uint8_t records[7];                                                       //Salva a Gravação
uint8_t buf[64];

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf     --> command length
           len     --> number of parameters
*/

//define para os comandos
#define chamada           1
#define iniciarB          2
#define aumentarTemp      3
#define diminuirTemp      4
#define PotTotal          5
#define PotMin            6
#define terminarB         7


//define para os pinos (7,8,4,9,11,12,13 estão ocupados com o sd e a voz)
#define pinoChipSelectSD  4                                                 //Pino CS do Modulo SD
#define botao_treinar     5
#define contMOSFET        6
#define Valvula           10
#define SensorTemp        A1
//o sensor de fluxo está na interrupção do pino 2
boolean aux =false;
boolean cham = false; //para saber se houve chamada
boolean banho = false;//para saber se ta tomando o banho
boolean automatico = true;
int treino=0;
int pwm = 0;
//define uma instancia do oneWire para a comunicacao com o sensor
OneWire oneWire(SensorTemp);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

int ContadordePulso;//var global para contar os pulsos da vazao

void printSignature(uint8_t *buf, int len)
{
  int i;
  for(i=0; i<len; i++){
    if(buf[i]>0x19 && buf[i]<0x7F){
      Serial.write(buf[i]);
    }
    else{
      Serial.print(F("["));
      Serial.print(buf[i], HEX);
      Serial.print(F("]"));
    }
  }
}

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf  -->  VR module return value when voice is recognized.
             buf[0]  -->  Group mode(FF: None Group, 0x8n: User, 0x0n:System
             buf[1]  -->  number of record which is recognized. 
             buf[2]  -->  Recognizer index(position) value of the recognized record.
             buf[3]  -->  Signature length
             buf[4]~buf[n] --> Signature
*/
void printVR(uint8_t *buf)
{
  Serial.println(F("VR Index\tGroup\tRecordNum\tSignature"));

  Serial.print(buf[2], DEC);
  Serial.print(F("\t\t"));

  if(buf[0] == 0xFF){
    Serial.print(F("NONE"));
  }
  else if(buf[0]&0x80){
    Serial.print(F("UG "));
    Serial.print(buf[0]&(~0x80), DEC);
  }
  else{
    Serial.print(F("SG "));
    Serial.print(buf[0], DEC);
  }
  Serial.print(F("\t"));

  Serial.print(buf[1], DEC);
  Serial.print(F("\t\t"));
  if(buf[3]>0){
    printSignature(buf+4, buf[3]);
  }
  else{
    Serial.print(F("NONE"));
  }
  Serial.println(F("\r\n"));
}

void setup()
{
  // configurações
  //iniciando o modulo vr e a comunicação serial
  myVR.begin(9600);
  // VR.begin(9600);
  Serial.begin(9600);
  
  //inicia o modulo sd e verifica sua conexao
  if (!SD.begin(pinoChipSelectSD)) {
        Serial.println(F("Falha no cartao SD"));                               //caso tenha algum erro no modulo sd
        while(1); //trava 
  }
  audioCartaoSD.speakerPin = 9;                                             //pino pra saida de audio
 
  //define os pinos
  pinMode(contMOSFET,     OUTPUT);
  pinMode(botao_treinar,  INPUT ); 
  pinMode(Valvula,        OUTPUT);
  digitalWrite(Valvula, LOW);
  
  //inicia o medidor de vazao
  attachInterrupt(0, MedVazao, RISING); //Configura o pino 2(Interrupção 0) para trabalhar como interrupção
  //o pino 2 ja ta em uso, tem que mudar isso

  //inicia o sensor de temperatura
  sensors.begin();
  if (!sensors.getAddress(sensor1, 0)) 
     Serial.println(F("Sensores nao encontrados !")); 
  //verifica a conexao do mdulo de voz
  if(myVR.clear() == 0){
        Serial.println(F("Recognizer cleared."));
  }else{
        Serial.println(F("Not find VoiceRecognitionModule."));
        Serial.println(F("Please check connection and restart Arduino."));
        while(1);
  }
 
//acredito que eu tenhha que fazer um if desses parta carregar cada um dos comandos que eu colocar
   if(myVR.load((uint8_t)chamada) >= 0)
     Serial.println(F("CHAMADA CARREGADO")); //acho que é só p falar q carregou
   if(myVR.load((uint8_t)iniciarB) >= 0)
     Serial.println(F("iniciarB CARREGADO"));
   if(myVR.load((uint8_t)aumentarTemp) >= 0)
     Serial.println(F("aumentarTemp"));
   if(myVR.load((uint8_t)diminuirTemp) >= 0)
     Serial.println(F("diminuirTemp CARREGADO"));
   if(myVR.load((uint8_t)PotTotal) >= 0)
     Serial.println(F("PotTotal CARREGADO"));
   if(myVR.load((uint8_t)PotMin) >= 0)
     Serial.println(F("PotMin CARREGADO"));
   if(myVR.load((uint8_t)terminarB) >= 0)
     Serial.println(F("terminarB CARREGADO"));
}

void loop()
{
  int ret;
  float tempC;
  float vazao=0;
  
      if(!banho)
    {
      digitalWrite(Valvula, LOW); 
    }

    //treina os comandos
   if(digitalRead(botao_treinar)) 
   {
      if(digitalRead(botao_treinar))
      {
          pwm=0;
          digitalWrite(Valvula, LOW);
          delay(250);                                                             //em vez desse delay fazer a sequencia de botão la
          audioCartaoSD.play("SOM-22.wav");                                     //Fala dizendo que a sequencia foi iniciaada
          while(audioCartaoSD.isPlaying())
          {
            digitalWrite(Valvula, LOW);                                        //enquanto estiver reproduzindo audio ele trava
          }
          for(int comando_sequencia_treino = 1; comando_sequencia_treino<8; comando_sequencia_treino++)
          {
                
              Serial.print(F("Comando_sequencia_treino:"));
          
              switch(comando_sequencia_treino)
              {
                  case 1://chamada
                      audioCartaoSD.play("SOM-13.wav"); 
                      while(audioCartaoSD.isPlaying())
                      {
                          digitalWrite(Valvula, LOW); 
                      }
                      myVR.write(0xAA);
                      myVR.write(0x03);
                      myVR.write(0x20);
                      myVR.write(0x01); //numero do comando que eu quero treinar
                      myVR.write(0x0A);
                  break;

                  case 2://ligar
                      audioCartaoSD.play("SOM-14.wav"); 
                      while(audioCartaoSD.isPlaying())
                      {
                          digitalWrite(Valvula, LOW); 
                      }
                      myVR.write(0xAA);
                      myVR.write(0x03);
                      myVR.write(0x20);
                      myVR.write(0x02);
                      myVR.write(0x0A);     
                  break;

                  case 3://aumentar
                      audioCartaoSD.play("SOM-15.wav"); 
                      while(audioCartaoSD.isPlaying())
                      {
                          digitalWrite(Valvula, LOW); 
                      }
                      myVR.write(0xAA);
                      myVR.write(0x03);
                      myVR.write(0x20);
                      myVR.write(0x03);
                      myVR.write(0x0A);
                  break;

                  case 4://diminuir
                      audioCartaoSD.play("SOM-16.wav"); 
                      while(audioCartaoSD.isPlaying())
                      {
                          digitalWrite(Valvula, LOW); 
                      }
                      myVR.write(0xAA);
                      myVR.write(0x03);
                      myVR.write(0x20);
                      myVR.write(0x04);
                      myVR.write(0x0A);
                  break;

                  case 5://total
                      audioCartaoSD.play("SOM-17.wav"); 
                      while(audioCartaoSD.isPlaying())
                      {
                          digitalWrite(Valvula, LOW); 
                      }
                      myVR.write(0xAA);
                      myVR.write(0x03);
                      myVR.write(0x20);
                      myVR.write(0x05);
                      myVR.write(0x0A);
                  break;

                  case 6://minimo
                      audioCartaoSD.play("SOM-18.wav"); 
                      while(audioCartaoSD.isPlaying())
                      {
                          digitalWrite(Valvula, LOW); 
                      }
                      myVR.write(0xAA);
                      myVR.write(0x03);
                      myVR.write(0x20);
                      myVR.write(0x06);
                      myVR.write(0x0A);
                  break;

                  case 7://desligar
                      audioCartaoSD.play("SOM-20.wav"); 
                      while(audioCartaoSD.isPlaying())
                      {
                          digitalWrite(Valvula, LOW); 
                      }
                      myVR.write(0xAA);
                      myVR.write(0x03);
                      myVR.write(0x20);
                      myVR.write(0x07);
                      myVR.write(0x0A);
                  break;

                  default:
                      Serial.println(F("NADA"));
                  break;
            }
            // AGUARDA 10 SEGUNDOS ATÉ OS COMANDOS SEREM TREINADOS  
            delay(15000);
            Serial.println(F("Comando treinado"));
            audioCartaoSD.play("SOM-25.wav"); //Audio dizendo q gravou
            while(audioCartaoSD.isPlaying())
            {
                digitalWrite(Valvula, LOW); 
            }
        }
        Serial.println(F("fim do treino"));
      }
   }  

  //executa os comandos
  ret = myVR.recognize(buf, 50);
  if(ret>0){
    //execução dos comandos
    switch(buf[1]){
      case chamada:
 
          cham = true;//diz que houve uma chamada
          Serial.println(F("cham"));
          audioCartaoSD.play("SOM-1.wav"); //Reconhece a chamada
          while(audioCartaoSD.isPlaying())
          {
                if(!banho)
                {
                      digitalWrite(Valvula, LOW); 
                } 
          }
         

      break;

      case iniciarB:
         if(cham)//caso tenha havido uma chamada anteriormente
         {
            delay(100);
            audioCartaoSD.play("SOM-6.wav"); //Som de ligar
            while(audioCartaoSD.isPlaying());
            digitalWrite(Valvula, HIGH); //abre a válvula
            banho = true;//diz que começou o banho
            cham = false;//diz que ja usou a chamada
            automatico=true;
        }
          
      break;

      case aumentarTemp:
          if(cham&&banho)
          {
            audioCartaoSD.play("SOM-2.wav"); 
            while(audioCartaoSD.isPlaying());
            if(pwm<242)
              pwm=pwm+13;
            else
              pwm=255;
            cham = false;//diz que ja usou a chamada
            automatico = false;
          }
      break;

      case diminuirTemp:
          if(cham&&banho)
          {
            audioCartaoSD.play("SOM-3.wav"); 
            while(audioCartaoSD.isPlaying());
            if(pwm>13)
              pwm=pwm-13;//diminui 5%
            else
              pwm=0;
            cham = false;//diz que ja usou a chamada
            automatico = false;
          }
      break;

      case PotTotal:
          if(cham&&banho)
          {
            audioCartaoSD.play("SOM-5.wav"); 
            while(audioCartaoSD.isPlaying());
            pwm = 255;
            cham = false;//diz que ja usou a chamada
            automatico = false;
          }
      break;

      case PotMin:
      if(cham&&banho)
          {
            audioCartaoSD.play("SOM-4.wav"); 
            while(audioCartaoSD.isPlaying());
            pwm = 0;
            cham = false;//diz que ja usou a chamada
            automatico = false;
          }
      break;

      case terminarB:
          if(cham&&banho)
          {
            digitalWrite(Valvula, LOW);
            audioCartaoSD.play("SOM-7.wav"); 
            while(audioCartaoSD.isPlaying())
            {
                 digitalWrite(Valvula, LOW); 
            }
            digitalWrite(Valvula, LOW);
            pwm = 0;
            banho = false;//diz que terminou o banho
            cham = false;//diz que ja usou a chamada
          }
      break;
      
      default:                                                                //Caso nenhuma dos casos acima seja reconhecido isso acontece, ou seja aqui que eu vou falar p maluco repetir
        Serial.println(F("Record function undefined"));
      break;
    }
    /** voice recognized */
    printVR(buf);
  }

  //salva os comandos
  if(treino>=50)
  {
      if(myVR.clear() == 0){
        Serial.println(F("Recognizer cleared."));
      }else{
        Serial.println(F("Not find VoiceRecognitionModule."));
        Serial.println(F("Please check connection and restart Arduino."));
        while(1);
      }
 
    //acredito que eu tenhha que fazer um if desses parta carregar cada um dos comandos que eu colocar
      if(myVR.load((uint8_t)chamada) >= 0)
        Serial.println(F("CHAMADA CARREGADO")); //acho que é só p falar q carregou
      if(myVR.load((uint8_t)iniciarB) >= 0)
        Serial.println(F("iniciarB CARREGADO"));
      if(myVR.load((uint8_t)aumentarTemp) >= 0)
        Serial.println(F("aumentarTemp"));
      if(myVR.load((uint8_t)diminuirTemp) >= 0)
        Serial.println(F("diminuirTemp CARREGADO"));
      if(myVR.load((uint8_t)PotTotal) >= 0)
        Serial.println(F("PotTotal CARREGADO"));
      if(myVR.load((uint8_t)PotMin) >= 0)
        Serial.println(F("PotMin CARREGADO"));
      if(myVR.load((uint8_t)terminarB) >= 0)
        Serial.println(F("terminarB CARREGADO"));
        treino=0;
  }


  //faz as contas
  if(automatico)
  {
    //formula 
    ContadordePulso = 0;
    sei();
    delay(1000);
    cli();
    vazao = ContadordePulso/7;//calcula litros por minuito
    Serial.print(F("Vazao ="));
    Serial.println(vazao);
    vazao = vazao/60;//transforma em litros por segundo
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(sensor1);
    Serial.print(F("Tf ="));
    Serial.println(tempC);
    int potencia = (40-tempC)*4180*vazao;//talvez tenha que trocar o 197 por 219
    pwm = map(potencia,0,7500,0,255);
  }
  if(banho)
  {
    analogWrite(contMOSFET, pwm);
    Serial.print(F("pwm ="));
    Serial.println(pwm);
  }
  else
    analogWrite(contMOSFET, 0);
  treino++;
}
void MedVazao()
{
  ContadordePulso++;
}

