# DCCxx-ESP32-Railcom

Ce programme permet la réalisation d'une station de commande DCC en WiFi qui génère le cutout nécessaire pour la détection RailCom

   *******************************************************
   IL NE FONCTIONNE QUE SUR UN ENVIRONNEMENT ESP32
   *******************************************************

   Il reprend pour une part du programme publié par Pascal Barlier ici :
   https://github.com/EditionsENI/Arduino-et-le-train/tree/master/V2/arduino/at-multi2

   Il a été écrit sur la base de classes C++ pour pouvoir intégrer facilement les évolutions futures : commandes de fonctions, réglages et lecture de CVs...

   Il a été conçu pour recevoir des commandes extérieures. Sur le port série et en WiFi.

  Pour assurer une compatibilité maximale avec les différents systèmes de commande déjà existants, le protocole de messagerie adopté est celui de DCC++.
  ce qui permet le pilotage avec JMRI par exemple (Testé).

  Ainsi, une commande de traction devra être écrite de la manière suivante : <t 4 31 100 1>
  Hormis le premier caractère t, indiquant une commande de traction, seuls les 3 dernières valeurs sont utilisées :
  - 31 -> adresse du décodeur
  - 100 -> la vitesse en pas (sur un maxi de 128) Pour des raisons de simplification, seul le protocole à 128 crans est implanté
  - 1 -> la direction (1 = sens avant, 0 = sens arrière)

  Une commande de traction s'écrit de la manière suivante : <f 31 144>
  //      128 à 159 => F0_F4
  //      176 à 191 => F5_F8
  //      160 à 175 => F9_F12

  L'envoi de commande peut être testé avec n'importe quelle application capable d'envoyer de messages sur le port série (115200 bauds)
  comme la fenêtre de commande de l'IDE Arduino ou l'application CoolTerm.

  Pour plus d'informations sur le protocole de messagerie de DCC++ : https://github.com/DccPlusPlus/BaseStation/blob/master/DCC%2B%2B%20Arduino%20Sketch.pdf

  Il est également possible de piloter cette centrale en utilisant un bus CAN. Il est nécessaire de décommenter la ligne #define CAN_INTERFACE du fichier config.h, 
  de relier les liaisons can_h et can_l aux broches GPIO_NUM_22 et GPIO_NUM_23 de l'ESP32.

  /* ----- CAN ----------------------*/
  
  #define CAN_INTERFACE
  
  #define CAN_RX GPIO_NUM_22
  
  #define CAN_TX GPIO_NUM_23
  
  #define CAN_BITRATE 1000UL * 1000UL // 1 Mb/s
  
  /* -------------------------------*/

  
Le message CAN pour commander la central devra respecter les regles suivantes :

- data[0] la valeur interne de la centrale pour commande DCC => 0xF0
- L'adresse de la locomotive (big-endian), c'est à dire, l'adresse courte dans data[2] et 0 dans data[1].
- Et pour une adresse longue, le MSB dans data[1] et le LSB dans data[2]
- La vitesse (sur 128 crans) dans data[3]
- La direction dans data[4]

  Voici l'extrait du code concerné par la transmission CAN :
  
#ifdef CAN_INTERFACE

    CANMessage frame;
    
    if (ACAN_ESP32::can.receive(frame))
    
    {
    
      Serial.println(frame.data[0]);
      
      if (frame.data[0] == 0xF0)
      
      {
      
        //      uint16_t locoAddr = frame.data[1] << 8 + frame.data[2];
        
        //      uint8_t locoSpeed = frame.data[3];
        
        //      uint8_t locoDir = frame.data[4];
        
        dcc.setThrottle((frame.data[1] << 8) + frame.data[2], frame.data[3], frame.data[4]);
        
      }
      
    }
    
#endif



  

  Cette station n'a pour l'instant été testée qu'avec la carte moteur LMD18200 et adopte le brochage de DCC++ pour la voie principale (main)
  Par défaut dans ce programme, le brochage est celui correspondant à l'Arduino MEGA à savoir :

  #define PIN_PWM       GPIO_NUM_12   // ENABLE (PWM)
  
  #define PIN_DIR       GPIO_NUM_13   // SIGNAL (DIR)
  
  #define PIN_BRAKE     GPIO_NUM_14   // CUTOUT (BRAKE)
  
  #define CURRENT_MONITOR_PIN_MAIN GPIO_NUM_36 // Mesure de courant
  
  

****************************************************************************************************************************************
  ATTENTION à parametrer le protection de courant à une valeur correspondant à votre configuration
  #define CURRENT_SAMPLE_MAX 3200 // 2,7 V étant une valeur raisonnable à ne pas dépasser
****************************************************************************************************************************************

        Christophe Bobille / LOCODUINO (http://www.locoduino.org) janv 2022 © locoduino 2022
        Original description from DCC++ BASE STATION a C++ program written by Gregg E. Berman GNU General Public License.
        Original description from Pascal Barlier GNU General Public License
