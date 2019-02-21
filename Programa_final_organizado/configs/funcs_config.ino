/*  Autor: Rodrigo Ferraz Souza
 *  Titulo: TCC - funçoes de configuraçao
 *  NOTAS:
 */

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
