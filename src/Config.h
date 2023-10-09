#ifndef CONFIG_H
#define CONFIG_H

#define PROJECT "DCCxx ESP32 WIFI - RAILCOM"
#define VERSION "v 2.0"

#define PIN_PWM GPIO_NUM_12                  // ENABLE (PWM)
#define PIN_DIR GPIO_NUM_13                  // SIGNAL (DIR)
#define PIN_BRAKE GPIO_NUM_14                // CUTOUT (BRAKE)
#define CURRENT_MONITOR_PIN_MAIN GPIO_NUM_35 // Mesure de courant

/*------------- Configure your own settings ---------------*/

// Define communication interface : 0 = Serial Port, 1 = Wifi
#define COMM_INTERFACE 1 // Select mode
//-----------------------------------------------------------
#if COMM_INTERFACE == 0 // USB serial
#define INTERFACE HardwareSerial

#elif COMM_INTERFACE == 1 // WiFi
#include <WiFi.h>
#define INTERFACE WiFiClient

//------ If WiFi, replace with your network credentials -----
#define WIFI_SSID "REPLACE_WITH_YOUR_SSID"
#define WIFI_PSW "REPLACE_WITH_YOUR_PASSWORD"
#define LOCAL_IP 192, 168, 1, 200
#define LOCAL_GATEWAY 192, 168, 1, 1
#define SUBNET 255, 255, 255, 0
#define PORT 2560
#endif

/* ----- CAN ----------------------*/
#define CAN_INTERFACE
#define CAN_RX GPIO_NUM_22
#define CAN_TX GPIO_NUM_23
#define CAN_BITRATE 1000UL * 1000UL // 1 Mb/s

/*----------------------------------------------------------*/

#define PRINT_CURRENT true

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

#define DCC_STACK_SPEED_SIZE 8	 // Taille de la pile DCC - vitesse des locos
#define DCC_STACK_FUNCTION_SIZE 16   // Taille de la pile DCC - fonctions des locos
#define DCC_STACK_CONTROL_SIZE 16   // Taille de la pile DCC - réglage des CV

#define DCC_STACK_SPEED 1
#define DCC_STACK_FUNCTION 2
#define DCC_STACK_CONTROL 3


#define LM_CRANS128 4

enum packet_type{DCC_SPEED=1,DCC_FCT,DCC_CV,DCC_RESET};

#endif
