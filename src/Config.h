#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "v 1.1"
#define PROJECT "DCCxx ESP32 WIFI RAILCOM"

#define PIN_PWM       GPIO_NUM_12   // ENABLE (PWM)
#define PIN_DIR       GPIO_NUM_13   // SIGNAL (DIR)
#define PIN_BRAKE     GPIO_NUM_14   // CUTOUT (BRAKE)

#define CURRENT_MONITOR_PIN_MAIN    GPIO_NUM_36


#define WIFI_SSID              "Freebox-5C00B0"
#define WIFI_PSW               "relat@@-apiarius3-evitandam#-resolvatis9"


// DEFINE COMMUNICATIONS INTERFACE
//  0 = Serial Port
//  1 = Wifi
#define COMM_INTERFACE   0

#if COMM_INTERFACE == 0
  #define INTERFACE HardwareSerial
#elif COMM_INTERFACE == 1
  #include <WiFi.h>
  #define INTERFACE   WiFiClient
  #define LOCAL_IP    192,168,1,200
  #define PORT        2560
#endif 

#define PRINT_CURRENT 0   // PRINT_CURRENT 1 : Permet le retour de la consommation de courant lue toute les secondes
                          // selon le protocole DCC++ <a xxx> . La valeur est rapportée sur une échelle de 0 à 1024
                          // pour être compatible avec les centrales comme JMRI

#define DCC_CUT_0 0 // Les deux états de l'automate cutout
#define DCC_CUT_1 1

#define DCC_BIT_HIGH 0 // Les quatre états de l'automate bit
#define DCC_BIT_HIGH0 1
#define DCC_BIT_LOW 2
#define DCC_BIT_LOW0 3

#define DCC_PACKET_IDLE 0 // Les cinq états de l'automate paquet
#define DCC_PACKET_HEADER 1
#define DCC_PACKET_START 2
#define DCC_PACKET_BYTE 3
#define DCC_PACKET_STOP 4
#define DCC_PACKET_END 5
#define DCC_PACKET_CUTOUT 6

#define DCC_PACKET_NUM 16
#define DCC_PACKET_SIZE 6 // Taille maximum d'un paquet DCC
#define DCC_HEADER_SIZE 20
#define DCC_CUTOUT_SIZE 18
#define DCC_FUNCTION_MAX 12 // Nombre de fonctions à commander

#define DCC_PACKET_TYPE_MODE 0xF
#define DCC_PACKET_TYPE_SPEED 0
#define DCC_PACKET_TYPE_F0_F4 1
#define DCC_PACKET_TYPE_F5_F8 2
#define DCC_PACKET_TYPE_F9_F12 3
#define DCC_PACKET_TYPE_CV 8
#define DCC_PACKET_TYPE_CV_BIT 9
#define DCC_PACKET_TYPE_RESET 10

#define DCC_PACKET_TYPE_ADDR_LONG 0x80
#define DCC_PACKET_TYPE_STEP 0x30
#define DCC_PACKET_TYPE_STEP_14 0x00
#define DCC_PACKET_TYPE_STEP_27 0x10
#define DCC_PACKET_TYPE_STEP_28 0x20
#define DCC_PACKET_TYPE_STEP_128 0x30


#endif
