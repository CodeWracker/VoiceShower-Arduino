/*  Autor: Rodrigo Ferraz Souza
 *  Titulo: TCC - funçoes de execuão de comandos
 *  NOTAS:
 */

#include <DallasTemperature.h>
void chama()
{
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
}

void iniciar()
{
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
}

void aumentarT()
{
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
}

void diminuirT()
{
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
}

void total()
{
    if(cham&&banho)
    {
        audioCartaoSD.play("SOM-5.wav"); 
        while(audioCartaoSD.isPlaying());
        pwm = 255;
        cham = false;//diz que ja usou a chamada
        automatico = false;
    }
}

void minimo()
{
    if(cham&&banho)
    {
        audioCartaoSD.play("SOM-4.wav"); 
        while(audioCartaoSD.isPlaying());
        pwm = 0;
        cham = false;//diz que ja usou a chamada
        automatico = false;
    }
}

void encerrarB()
{
    if(cham&&banho)
    {
        digitalWrite(Valvula, LOW);
        audioCartaoSD.play("SOM-7.wav"); 
        while(audioCartaoSD.isPlaying())
            digitalWrite(Valvula, LOW); 
        digitalWrite(Valvula, LOW);
        pwm = 0;
        banho = false;//diz que terminou o banho
        cham = false;//diz que ja usou a chamada
    }
}

void contas()
{
    //formula 
     float vazao=0;
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

void MedVazao()
{
  ContadordePulso++;
}
