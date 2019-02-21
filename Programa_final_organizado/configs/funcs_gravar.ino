/*  Autor: Rodrigo Ferraz Souza
 *  Titulo: TCC - funçoes para gravar a voz
 *  NOTAS:
 */


void gravar()
{
    if(myVR.clear() == 0)
        Serial.println(F("Recognizer cleared."));
    else
    {
        Serial.println(F("Not find VoiceRecognitionModule."));
        Serial.println(F("Please check connection and restart Arduino."));
        while(1);
    }
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

