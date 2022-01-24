
#ifndef DCC_H
#define DCC_H

#include <Arduino.h>
#include "Config.h"


class DCC {

  private:
    static const byte m_pinPwm = PIN_PWM;
    static const byte m_pinDir = PIN_DIR;
    static const byte m_pinBrake = PIN_BRAKE;

    static byte m_cut;                    // 1/2 bit en cours d'envoi (pour la coupure de signal)
    static byte m_subBit;                 // Partie du bit en cours d'envoi
    static byte m_dataMode;               // Variable d'état de l'automate paquet
    static byte m_packetUsed;             // Nombre de paquets à envoyer
    static byte m_headerCount;            // Comptage des bits à un du préambule
    static byte m_bit;                    // Bit en cours d'envoi
    static byte m_byteCount;              // Index de l'octet en cours d'envoi
    static byte m_bitShift;               // Comptage des bits de l'octet à envoyer
    static byte m_packetData[DCC_PACKET_NUM][DCC_PACKET_SIZE]; // Paquets de données à envoyer
    static byte m_packetIndex;            // Paquet en cours d'envoi
    static byte m_packetSize[DCC_PACKET_NUM]; // Taille des paquets à envoyer
    static byte m_packetType[DCC_PACKET_NUM]; // Type des paquets à envoyer
    static byte m_cutoutCount;            // Comptage de la séquence de coupure
    static long m_dccCount;

    DCC() {}

  public:

    static void setup();
    static void packetFormat(byte, uint16_t, uint16_t, uint16_t);
    static void setThrottle(uint16_t, uint8_t, uint8_t);
    static void setFunction(uint16_t, byte, byte);
    static void clear();
    static void reset();
    static void dccAdd(byte*, byte, byte);
    static void dumpPackets();
    static void interrupt(void);
    static void parse(char *, INTERFACE *);
    //static void parse(char *, WiFiClient *);
};

#endif
