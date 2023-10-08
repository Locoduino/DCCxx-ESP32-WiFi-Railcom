

#include <Arduino.h>
#include "DCC.h"

DCC *ptrToClass;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

static void timerHandler(void)
{
  ptrToClass->onTime();
}

void IRAM_ATTR DCC::onTime()
{
  portENTER_CRITICAL_ISR(&timerMux);
  {
    // Automate d'envoi bit à bit des données DCC
    m_dccCount++;
    switch (m_cut)
    {
      case DCC_CUT_0:
        switch (m_subBit) // Automate bit
        {
          case DCC_BIT_HIGH:
            switch (m_dataMode) // Automate paquet
            {
              case DCC_PACKET_IDLE:
                if (m_packetUsed)
                {
                  m_dataMode = DCC_PACKET_HEADER;
                  m_headerCount = DCC_HEADER_SIZE;
                }
                break;
              case DCC_PACKET_HEADER:
                m_bit = 1;
                if (!--m_headerCount)
                {
                  m_dataMode = DCC_PACKET_START;
                  m_byteCount = 0;
                }
                break;
              case DCC_PACKET_START:
                m_bit = 0;
                m_bitShift = 0x80;
                m_dataMode = DCC_PACKET_BYTE;
                break;
              case DCC_PACKET_BYTE:
                m_bit = !!(m_packetData[m_packetIndex][m_byteCount] & m_bitShift);
                m_bitShift >>= 1;
                if (!m_bitShift)
                {
                  if (m_packetSize[m_packetIndex] == ++m_byteCount) // Fin du paquet
                    m_dataMode = DCC_PACKET_STOP;
                  else
                    m_dataMode = DCC_PACKET_START;
                }
                break;
              case DCC_PACKET_STOP:
                m_bit = 1;
                if (m_packetType[m_packetIndex] & DCC_PACKET_TYPE_CV) // Les paquets de programmation sont effacés après envoi
                {
                  m_packetSize[m_packetIndex] = 0;
                  m_packetUsed--; // Suppression du paquet envoyé
                }
                if (m_packetUsed)
                {
                  for (char i = DCC_PACKET_NUM; --i >= 0;)
                  {
                    m_packetIndex++;
                    if (m_packetIndex == DCC_PACKET_NUM)
                      m_packetIndex = 0;
                    if (m_packetSize[m_packetIndex])
                      break;
                  }
                  m_dataMode = DCC_PACKET_END;
                  m_cutoutCount = DCC_CUTOUT_SIZE;
                }
                else
                {
                  m_dataMode = DCC_PACKET_IDLE;
                }
                break;
              case DCC_PACKET_END:
                m_dataMode = DCC_PACKET_CUTOUT;
                m_cutoutCount = DCC_CUTOUT_SIZE;
                break;
            }
            digitalWrite(PIN_DIR, HIGH);
            if (m_bit)
              m_subBit = DCC_BIT_LOW;
            else
              m_subBit = DCC_BIT_HIGH0;
            break;
          case DCC_BIT_HIGH0:
            digitalWrite(PIN_DIR, HIGH);
            m_subBit = DCC_BIT_LOW;
            break;
          case DCC_BIT_LOW:
            digitalWrite(PIN_DIR, LOW);
            if (m_bit)
              m_subBit = DCC_BIT_HIGH;
            else
              m_subBit = DCC_BIT_LOW0;
            break;
          case DCC_BIT_LOW0:
            digitalWrite(PIN_DIR, LOW);
            m_subBit = DCC_BIT_HIGH;
            break;
        }
        m_cut = DCC_CUT_1;
        break;
      case DCC_CUT_1: // 1/2 bit : zone de coupure
        switch (m_dataMode)
        {
          case DCC_PACKET_CUTOUT:
            if (!--m_cutoutCount)
            {
              m_dataMode = DCC_PACKET_HEADER;
              m_headerCount = DCC_HEADER_SIZE;
              digitalWrite(PIN_BRAKE, LOW);
            }
            else
              digitalWrite(PIN_BRAKE, HIGH);
            break;
          default:
            m_cut = DCC_CUT_0;
        }
        break;
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void DCC::setup()
{
  pinMode(m_pinPwm, OUTPUT);
  pinMode(m_pinDir, OUTPUT);
  pinMode(m_pinBrake, OUTPUT);
  digitalWrite(m_pinPwm, LOW);
  digitalWrite(m_pinDir, LOW);
  digitalWrite(m_pinBrake, LOW);
  ptrToClass = this;
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, timerHandler, true);
  timerAlarmWrite(timer, 28, true);
  timerAlarmEnable(timer);
} // DCC::setup

void DCC::reset()
{
  // Envoie une séquence de réinitialisation générale des décodeurs
  byte i;
  for (i = 0; i < 5; i++)
    packetFormat(DCC_PACKET_TYPE_RESET, 0xFF, 0, 0);
} // DCC::reset

void DCC::setThrottle(uint16_t locoAddr, uint8_t locoSpeed, uint8_t locoDir)
{
  uint8_t dccSpeed = (locoSpeed > 127) ? 127 : locoSpeed;
  byte type = DCC_PACKET_TYPE_SPEED | DCC_PACKET_TYPE_STEP_128;
  if (dccSpeed == 1)
    dccSpeed++; // Pas de cran 1
  if (locoAddr > 127)
    type |= DCC_PACKET_TYPE_ADDR_LONG;
  uint16_t data = (locoDir > 0) ? 0x100 : 0;
  data |= dccSpeed;
  packetFormat(type, locoAddr, data, 0);
  // Serial.print("data : "); Serial.println(data, BIN);
} // DCC::setThrottle

void DCC::setFunction(uint16_t locoAddr, byte fByte, byte eByte)
{
  byte type = 0;
  uint16_t data = 0;

  //
  //      B100xxxxx => DCC_PACKET_TYPE_F0_F4
  //      B1011xxxx => DCC_PACKET_TYPE_F5_F8
  //      B1010xxxx => DCC_PACKET_TYPE_F9_F12

  switch (fByte >> 5)
  {
    case (B100):
      type = DCC_PACKET_TYPE_F0_F4;
      data = (fByte & 0x1F);
      break;
    case (B101):
      switch (fByte >> 4)
      {
        case (B1011):
          type = DCC_PACKET_TYPE_F5_F8;
          data = (fByte & 0xF);
          break;
        case (B1010):
          type = DCC_PACKET_TYPE_F9_F12;
          data = (fByte & 0xF);
          break;
      }
      break;
  }
  if (locoAddr > 127)
    type |= DCC_PACKET_TYPE_ADDR_LONG;
  packetFormat(type, locoAddr, data, 0);
} //DCC::setFunction

void DCC::packetFormat(byte type, uint16_t addr, uint16_t data1, uint16_t data2)
{
  // Formate des données pour former un paquet DCC et le stocker

  byte packetData[DCC_PACKET_SIZE];
  byte checksum = 0;
  byte packetSize = 1;
  byte *packetPtr = packetData;
  byte dir;
  byte ext;

  if (!(type & DCC_PACKET_TYPE_CV)) // Paquet de pilotage
  {
    if (type & DCC_PACKET_TYPE_ADDR_LONG)
    {
      checksum ^= *packetPtr++ = 0xC0 | ((addr >> 8) & 0x3F);
      checksum ^= *packetPtr++ = addr & 0xFF;
      packetSize += 2;
    }
    else
    {
      checksum ^= *packetPtr++ = addr & 0x7F;
      packetSize++;
    }
  }
  switch (type & DCC_PACKET_TYPE_MODE)
  {
    case DCC_PACKET_TYPE_SPEED:
      dir = !!(data1 & 0x100);
      switch (type & DCC_PACKET_TYPE_STEP)
      {
        case DCC_PACKET_TYPE_STEP_14:
          checksum ^= *packetPtr++ = (data1 & 0xF) | (dir ? 0x60 : 0x20);
          packetSize++;
          break;
        case DCC_PACKET_TYPE_STEP_27:
        case DCC_PACKET_TYPE_STEP_28:
          ext = (data1 & 1) << 4;
          data1 >>= 1;
          checksum ^= *packetPtr++ = (data1 & 0xF) | (dir ? 0x60 : 0x20) | ext;
          packetSize++;
          break;
        case DCC_PACKET_TYPE_STEP_128:
          checksum ^= *packetPtr++ = 0x3F;
          checksum ^= *packetPtr++ = (data1 & 0x7F) | (dir ? 0x80 : 0);
          packetSize += 2;
          break;
      }
      break;
    case DCC_PACKET_TYPE_F0_F4:
      checksum ^= *packetPtr++ = 0x80 | (data1 & 0x1F);
      packetSize++;
      break;
    case DCC_PACKET_TYPE_F5_F8:
      checksum ^= *packetPtr++ = 0xB0 | (data1 & 0xF);
      packetSize++;
      break;
    case DCC_PACKET_TYPE_F9_F12:
      checksum ^= *packetPtr++ = 0xA0 | (data1 & 0xF);
      packetSize++;
      break;
    case DCC_PACKET_TYPE_CV: // data1 = Numéro de la variable ; data2 = Valeur
      checksum ^= *packetPtr++ = ((data1 >> 8) & 3) | 0x7C;
      checksum ^= *packetPtr++ = data1 & 0xFF;
      checksum ^= *packetPtr++ = data2 & 0xFF;
      packetSize += 3;
      break;
    case DCC_PACKET_TYPE_CV_BIT: // data1 = Numéro de la variable ; data2 = masque + bit
      checksum ^= *packetPtr++ = ((data1 >> 8) & 3) | 0x78;
      checksum ^= *packetPtr++ = data1 & 0xFF;
      checksum ^= *packetPtr++ = 0xF0 | data2;
      packetSize += 3;
      break;
    case DCC_PACKET_TYPE_RESET:
      checksum ^= *packetPtr++ = 0;
      checksum ^= *packetPtr++ = 0;
      packetSize += 2;
      break;
  }
  *packetPtr = checksum;
  dccAdd(packetData, packetSize, type & DCC_PACKET_TYPE_MODE); // TODO pour PILOT
} // DCC::packetFormat

void DCC::dccAdd(byte *packetData, byte packetSize, byte packetType)
{
  // Ajoute un paquet dans la liste d'envoi
  if (packetSize > DCC_PACKET_SIZE)
    return;
  byte index = DCC_PACKET_NUM;
  if (!(packetType & DCC_PACKET_TYPE_CV)) // Paquet de pilotage
    for (index = 0; index < DCC_PACKET_NUM; index++)
      if (m_packetSize[index] && (m_packetType[index] == packetType)) // On remplace un paquet existant
        break;
  if (index == DCC_PACKET_NUM)
    for (index = 0; index < DCC_PACKET_NUM; index++) // On cherche un emplacement libre
      if (!m_packetSize[index])
      {
        m_packetUsed++; // Nouveau paquet
        break;
      }
  if (index == DCC_PACKET_NUM)
    return; // Plus de place libre
  memcpy(m_packetData[index], packetData, packetSize);
  m_packetType[index] = packetType;
  m_packetSize[index] = packetSize;
} // DCC::dccAdd

void DCC::dumpPackets()
{ // DEBUG
  // Fonction de débuggage
  // Serial.println(m_dccCount);
  // static byte counter = 0;
  // if(++counter==10)
  //{
  // counter = 0;

  for (int i = 0; i < DCC_PACKET_NUM; i++)
  {
    if (m_packetSize[i] > 0)
    {
      Serial.print(m_packetSize[i]);
      Serial.print(" [");
      Serial.print(m_packetType[i]);
      Serial.print("] : ");
      // Preambule
      for (int i = 0; i < 20; i++)
        Serial.print("1");

      for (int j = 0; j < m_packetSize[i]; j++)
      {
        // for (int j = 0; j < DCC_PACKET_SIZE; j++) {
        //  Bit separateur
        Serial.print(" ");
        Serial.print("0");
        Serial.print(" ");
        Serial.print(m_packetData[i][j], BIN);
        Serial.write(' ');
      }
      Serial.print(" ");
      Serial.print("1");
      Serial.print(" ");
      Serial.println();
    }
  }
} // DCC::dumpPackets

void DCC::clear()
{
  // Vide la liste d'envoi
  for (int i = 0; i < DCC_PACKET_NUM; i++)
  {
    m_packetData[i][0] = 0xFF;
    m_packetSize[i] = 0;
  }
  m_packetUsed = 0;
} // DCC::clear

void DCC::parse(char *com, INTERFACE *client)
{
  int a, x, s, d;
  int fByte, eByte;
  int locoAddr = 0;
  int locoSpeed = 0;
  int locoDir = 0;
  switch (com[0])
  {
    case '0':
      digitalWrite(PIN_PWM, LOW);
      client->printf("<p%c>", com[0]);
      break;

    case '1':
      digitalWrite(PIN_PWM, HIGH);
      client->printf("<p%c>", com[0]);
      break;

    case 't': // <t REGISTER CAB SPEED DIRECTION>
      sscanf(com + 1, "%d %d %d %d", &x, &a, &s, &d);
      locoAddr = a;
      locoSpeed = s;
      locoDir = d;
      DCC::setThrottle(locoAddr, locoSpeed, locoDir);
      client->printf("<T %d %d %d>", x, s, d);
      break;

    case 'f': // <f CAB BYTE1 [BYTE2]>
      sscanf(com + 1, "%d %d %d", &locoAddr, &fByte, &eByte);
      DCC::setFunction(locoAddr, fByte, eByte);
      break;

    case 's':

      if (digitalRead(PIN_PWM) == LOW)
        client->print("<p0>");
      else
        client->print("<p1>");

      client->print("<iDCCxx BASE STATION FOR ESP32 ");
      client->print(VERSION);
      client->print(" / ");
      client->print(__DATE__);
      client->print(" ");
      client->print(__TIME__);
      client->print(">");

      client->print("<N");
      if (COMM_INTERFACE == 0)
        client->print("Serial>");
      else
      {
        client->print("WiFi");
        client->print(": ");
        //client->print(WiFi.localIP());
        client->print(">");
      }

      break;

    default:
      return;
  }
} // DCC::parse
