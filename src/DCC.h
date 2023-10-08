
#ifndef DCC_H
#define DCC_H

#include <Arduino.h>
#include "Config.h"

class DCC
{

private:
  const byte m_pinPwm = PIN_PWM;
  const byte m_pinDir = PIN_DIR;
  const byte m_pinBrake = PIN_BRAKE;

  byte m_cut;                                         // 1/2 bit en cours d'envoi (pour la coupure de signal)
  byte m_subBit;                                      // Partie du bit en cours d'envoi
  byte m_dataMode;                                    // Variable d'état de l'automate paquet
  byte m_packetUsed;                                  // Nombre de paquets à envoyer
  byte m_headerCount;                                 // Comptage des bits à un du préambule
  byte m_bit;                                         // Bit en cours d'envoi
  byte m_byteCount;                                   // Index de l'octet en cours d'envoi
  byte m_bitShift;                                    // Comptage des bits de l'octet à envoyer
  byte m_packetData[DCC_PACKET_NUM][DCC_PACKET_SIZE]; // Paquets de données à envoyer
  byte m_packetIndex;                                 // Paquet en cours d'envoi
  byte m_packetSize[DCC_PACKET_NUM];                  // Taille des paquets à envoyer
  byte m_packetType[DCC_PACKET_NUM];                  // Type des paquets à envoyer
  byte m_cutoutCount;                                 // Comptage de la séquence de coupure
  long m_dccCount;

public:
  DCC() {}
  hw_timer_t *timer { nullptr };
  void IRAM_ATTR onTime(void);
  void setup();
  void packetFormat(byte, uint16_t, uint16_t, uint16_t);
  void setThrottle(uint16_t, uint8_t, uint8_t);
  void setFunction(uint16_t, byte, byte);
  void clear();
  void reset();
  void dccAdd(byte *, byte, byte);
  void dumpPackets();
  void interrupt(void);
  void parse(char *, INTERFACE *);
};

#endif
