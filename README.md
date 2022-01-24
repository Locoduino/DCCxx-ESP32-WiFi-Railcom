# DCCxx-ESP32-Railcom

Ce programme permet la réalisation d'une station de commande DCC qui génère le cutout nécessaire pour la détection RailCom

   *******************************************************
   IL NE FONCTIONNE QUE SUR UN ENVIRONNEMENT ESP32
   *******************************************************

   Il reprend pour une part du programme publié par Pascal Barlier ici :
   https://github.com/EditionsENI/Arduino-et-le-train/tree/master/V2/arduino/at-multi2

   Il a été écrit sur la base de classes C++ pour pouvoir intégrer facilement les évolutions futures : commandes de fonctions, réglages et lecture de CVs...

   Il a été conçu pour recevoir des commandes extérieures. Pour l'instant sur le seul port série, mais à l'avenir en Ethernet et en WiFi.
   A ce stade, c'est principalement un programme de test en particulier pour tester le retour d'information RailCom

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

  Cette station n'a pour l'instant été testée qu'avec la carte moteur LMD18200 et adopte le brochage de DCC++ pour la voie principale (main)
  Par défaut dans ce programme, le brochage est celui correspondant à l'Arduino MEGA à savoir :

  #define PIN_PWM       GPIO_NUM_12   // ENABLE (PWM)
  #define PIN_DIR       GPIO_NUM_13   // SIGNAL (DIR)
  #define PIN_BRAKE     GPIO_NUM_14   // CUTOUT (BRAKE)

  Pour Arduino Uno en particulier, se reporter ici : https://www.locoduino.org/spip.php?article187

****************************************************************************************************************************************
  ATTENTION à parametrer le protection de courant à une valeur correspondant à votre configuration
  #define CURRENT_SAMPLE_MAX 3200 // 2,7 V étant une valeur raisonnable à ne pas dépasser
****************************************************************************************************************************************

        Chritophe Bobille / LOCODUINO (http://www.locoduino.org) janv 2022 © locoduino 2022
        Original description from DCC++ BASE STATION a C++ program written by Gregg E. Berman GNU General Public License.
        Original description from Pascal Barlier GNU General Public License
