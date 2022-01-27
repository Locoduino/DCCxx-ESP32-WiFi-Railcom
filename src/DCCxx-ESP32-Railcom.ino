/*



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

*/

#define VERSION "v 0.8"
#define PROJECT "DCCxx ESP32 WIFI"

#include "DCC.h"
#include "Config.h"
#include "CurrentMonitor.h"

// Mesure de courant
char msg[] = "<p2>";
CurrentMonitor mainMonitor(CURRENT_MONITOR_PIN_MAIN, msg); // create monitor for current on Main Track
void Task0(void *pvParameters);
const uint32_t stackSize = 10000;

#if COMM_INTERFACE == 0
HardwareSerial *CLIENT = &Serial;
#elif COMM_INTERFACE == 1
WiFiServer SERVER(2560); // Create and instance of an WiFiServer
WiFiClient *CLIENT = NULL;
#endif

DCC dcc;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.printf("\n\nProject :      %s\n", PROJECT);
  Serial.printf("\nVersion   :      %s\n", VERSION);
  Serial.printf("\nFichier   :      %s\n", __FILE__);
  Serial.printf("\nCompiled  :      %s - %s \n\n", __DATE__, __TIME__);


// Infos ESP32
  // esp_chip_info_t out_info;
  // esp_chip_info(&out_info);
  // Serial.print("getSketchSize : "); Serial.println(String(ESP.getSketchSize() / 1000) + " Ko");
  // Serial.print("getFreeSketchSpace : "); Serial.println(String(ESP.getFreeSketchSpace() / 1000) + " Ko");
  // Serial.print("CPU freq : "); Serial.println(String(ESP.getCpuFreqMHz()) + " MHz");
  // Serial.print("CPU cores : ");  Serial.println(String(out_info.cores));
  // Serial.print("Flash size : "); Serial.println(String(ESP.getFlashChipSize() / 1000000) + " MB");
  // Serial.print("Free RAM : "); Serial.println(String((long)ESP.getFreeHeap()) + " bytes");
  // //Serial.print("Min. free seen : "); Serial.println(String((long)esp_get_minimum_free_heap_size()) + " bytes");
  // Serial.print("tskIDLE_PRIORITY : "); Serial.println(String((long)tskIDLE_PRIORITY));
  // Serial.print("configMAX_PRIORITIES : "); Serial.println(String((long)configMAX_PRIORITIES));
  // Serial.print("configTICK_RATE_HZ : "); Serial.println(String(configTICK_RATE_HZ) + " Hz");
  // Serial.println();

#if COMM_INTERFACE == 1
   IPAddress local_IP(192, 168, 1, 200);
   IPAddress gateway(192, 168, 1, 1);
   IPAddress subnet(255, 255, 255, 0);
   if (!WiFi.config(local_IP, gateway, subnet))
     Serial.println("STA Failed to configure");
  WiFi.begin(WIFI_SSID, WIFI_PSW);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected.");
  Serial.printf("\nIP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  SERVER.begin();

#endif

  dcc.setup();
  dcc.clear();
  mainMonitor.setup(CLIENT);

  xTaskCreate(
      Task0,     /* Task function. */
      "Task0",   /* String with name of task. */
      stackSize, /* Stack size in words. */
      NULL,      /* Parameter passed as input of the task */
      1,         /* Priority of the task. */
      NULL);     /* Task handle. */

  Serial.printf("End setup\n\n");
}

void Task0(void *parameter)
{
  long compt = 0;
  for (;;)
  {
    mainMonitor.check();
    if (compt == 1000ul)
    {
      if(CLIENT != NULL)
        CLIENT->printf("<a %d>", (int)(mainMonitor.current() / 4));
      compt = 0;
    }
    delay(1);
    compt++;
  }
  vTaskDelete(NULL);
}

void loop()
{
  char c;
  char commandString[16];

#if COMM_INTERFACE == 0
  while (Serial.available() > 0)
  {
    c = Serial.read();
    switch (c)
    {
    case '<':
      sprintf(commandString, "");
      break;
    case '>':
      Serial.println(commandString);
      dcc.parse(commandString, &Serial);
      break;
    default:
      sprintf(commandString, "%s%c", commandString, c);
    }
  } // while

#elif COMM_INTERFACE == 1
  WiFiClient client = SERVER.available();
  if(client) {
    CLIENT = &client;
    while (CLIENT->connected())
    { // loop while the client's connected
      if (CLIENT->available())
      { // while there is data on the network
        c = CLIENT->read();
        switch (c)
        {
        case '<':
          sprintf(commandString, "");
          break;
        case '>':
          Serial.println(commandString);
          dcc.parse(commandString, CLIENT);
          break;
        default:
          sprintf(commandString, "%s%c", commandString, c);
        }
      } // while
    }
  }
#endif
  delay(10);
}
