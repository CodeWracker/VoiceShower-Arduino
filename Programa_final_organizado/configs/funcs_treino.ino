/*  Autor: Rodrigo Ferraz Souza
 *  Titulo: TCC - funçoes para o treino da voz
 *  NOTAS:
 */


void treinar()
{
  pwm=0;     
  digitalWrite(Valvula, LOW);
  delay(250);                                                             //em vez desse delay fazer a sequencia de botão la
  audioCartaoSD.play("SOM-22.wav");                                     //Fala dizendo que a sequencia foi iniciaada
  while(audioCartaoSD.isPlaying())
    digitalWrite(Valvula, LOW);                                        //enquanto estiver reproduzindo audio ele trava
  for(int comando_sequencia_treino = 1; comando_sequencia_treino<8; comando_sequencia_treino++)
  {
     Serial.print(F("Comando_sequencia_treino:"));
     switch(comando_sequencia_treino)
     {
     case 1://chamada
     audioCartaoSD.play("SOM-13.wav"); 
     while(audioCartaoSD.isPlaying())
        digitalWrite(Valvula, LOW); 
     myVR.write(0xAA);
     myVR.write(0x03);
     myVR.write(0x20);
     myVR.write(0x01); //numero do comando que eu quero treinar
     myVR.write(0x0A);
     break;

     case 2://ligar
        audioCartaoSD.play("SOM-14.wav"); 
        while(audioCartaoSD.isPlaying())
            digitalWrite(Valvula, LOW); 
        myVR.write(0xAA);
        myVR.write(0x03);
        myVR.write(0x20);
        myVR.write(0x02);
        myVR.write(0x0A);     
     break;

     case 3://aumentar
        audioCartaoSD.play("SOM-15.wav"); 
        while(audioCartaoSD.isPlaying())
            digitalWrite(Valvula, LOW); 
        myVR.write(0xAA);
        myVR.write(0x03);
        myVR.write(0x20);
        myVR.write(0x03);
        myVR.write(0x0A);
     break;

     case 4://diminuir
        audioCartaoSD.play("SOM-16.wav"); 
        while(audioCartaoSD.isPlaying())
            digitalWrite(Valvula, LOW); 
        myVR.write(0xAA);
        myVR.write(0x03);
        myVR.write(0x20);
        myVR.write(0x04);
        myVR.write(0x0A);
     break;

     case 5://total
        audioCartaoSD.play("SOM-17.wav"); 
        while(audioCartaoSD.isPlaying())
            digitalWrite(Valvula, LOW); 
        myVR.write(0xAA);
        myVR.write(0x03);
        myVR.write(0x20);
        myVR.write(0x05);
        myVR.write(0x0A);
      break;

      case 6://minimo
          audioCartaoSD.play("SOM-18.wav"); 
          while(audioCartaoSD.isPlaying())
              digitalWrite(Valvula, LOW); 
          myVR.write(0xAA);
          myVR.write(0x03);
          myVR.write(0x20);
          myVR.write(0x06);
          myVR.write(0x0A);
      break;

      case 7://desligar
          audioCartaoSD.play("SOM-20.wav"); 
          while(audioCartaoSD.isPlaying())
              digitalWrite(Valvula, LOW); 
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
          digitalWrite(Valvula, LOW); 
  }
  Serial.println(F("fim do treino"));
}

