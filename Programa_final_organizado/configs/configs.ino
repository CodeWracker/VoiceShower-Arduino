/*  Autor: Rodrigo Ferraz Souza
 *  Titulo: TCC - configuraçoes
 *  NOTAS:
 */

#include <SD.h>
#include <TMRpcm.h>
#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"
#include <OneWire.h>
#include <DallasTemperature.h>

VR myVR(7,8);                                                             // 7:TX 8:RX, ta certo
SoftwareSerial VR3(7,8);

/*  Mapeamento de Hardware
 *  Conexão modulo sd
 *  MISO 12
 *  MOSI 11
 *  SCK 13
 *  CS 4 
 *  Conexão VoiceRecognitionModule
 *  7   ------->     TX
 *  8   ------->     RX
 *  
 *  9   ------->     Alto falante
*/
#define pinoChipSelectSD  4                                                 //Pino CS do Modulo SD
#define botao_treinar     5
#define contMOSFET        6
#define Valvula           10
#define SensorTemp        A1
//o sensor de fluxo está na interrupção do pino 2

//define para os comandos
#define chamada           1
#define iniciarB          2
#define aumentarTemp      3
#define diminuirTemp      4
#define PotTotal          5
#define PotMin            6
#define terminarB         7

TMRpcm audioCartaoSD;                                                     //Cria a saida de audio

uint8_t records[7];                                                       //Salva a Gravação
uint8_t buf[64];

boolean aux        = false;
boolean cham       = false; //para saber se houve chamada
boolean banho      = false; //para saber se ta tomando o banho
boolean automatico = true;  //para saber se esta ou não no modo automatico
int treino         = 0;
int pwm            = 0;
int ContadordePulso;        //var global para contar os pulsos da vazao

//define uma instancia do oneWire para a comunicacao com o sensor
OneWire oneWire(SensorTemp);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf     --> command length
           len     --> number of parameters
*/
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
 
//tem que fazer um if desses parta carregar cada um dos comandos que eu colocar
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

  if(!banho)
    digitalWrite(Valvula, LOW); 

  //Treina os comandos
  if(digitalRead(botao_treinar))
    treinar();

  //Executa os comandos
  ret = myVR.recognize(buf, 50);
  if(ret>0){
    //execução dos comandos
    switch(buf[1]){
      case chamada:       
          chama();
      break;

      case iniciarB:
          iniciar();
      break;

      case aumentarTemp:
          aumentarT();
      break;

      case diminuirTemp:
          diminuirT();
      break;

      case PotTotal:
          total();
      break;

      case PotMin:
          minimo();
      break;

      case terminarB:
          encerrarB();
      break;
      
      default:                                                                //Caso nenhuma dos casos acima seja reconhecido isso acontece, ou seja aqui que eu vou falar p maluco repetir
        Serial.println(F("Record function undefined"));
      break;
    }
    /** voice recognized */
    printVR(buf);
  }

  //Grava os comandos
  if(treino>=50)
  {
    gravar();
  }
  
  //faz as contas
  if(automatico)
  {
      contas();
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


