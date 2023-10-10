/*



   Ce programme permet la réalisation d'une station de commande DCC qui génère le cutout nécessaire pour la détection RailCom

   *******************************************************
   IL NE FONCTIONNE QUE SUR UN ENVIRONNEMENT ESP32
   *******************************************************

   Il reprend une partie du programme publié par Pascal Barlier ici :
   https://github.com/EditionsENI/Arduino-et-le-train/tree/master/V2/arduino/at-multi2

   Il a été écrit sur la base de classes C++ pour pouvoir intégrer facilement les évolutions futures : commandes de fonctions, réglages et lecture de CVs...

   Il a été conçu pour recevoir des commandes extérieures sur le port série, mais aussi en WiFi.
   A ce stade, c'est principalement un programme de test en particulier pour tester le retour d'information RailCom

  Pour assurer une compatibilité maximale avec les différents systèmes de commande déjà existants, le protocole de messagerie adopté est celui de DCC++.
  ce qui permet le pilotage avec JMRI par exemple (Testé).

  Ainsi, une commande de traction devra être écrite de la manière suivante : <t 4 31 100 1>
  Hormis le premier caractère t, indiquant une commande de traction, seuls les 3 dernières valeurs sont utilisées :
  - 31 -> adresse du décodeur
  - 100 -> la vitesse en pas (sur un maxi de 128) Pour des raisons de simplification, seul le protocole à 128 crans est implanté
  - 1 -> la direction (1 = sens avant, 0 = sens arrière)

  Une commande de fonction s'écrit de la manière suivante : <f 31 144>
  //      128 à 159 => F0_F4
  //      176 à 191 => F5_F8
  //      160 à 175 => F9_F12

  L'envoi de commande peut être testé avec n'importe quelle application capable d'envoyer de messages sur le port série (115200 bauds)
  comme la fenêtre de commande de l'IDE Arduino © ou l'application CoolTerm ©.

  Pour plus d'informations sur le protocole de messagerie de DCC++ : https://github.com/DccPlusPlus/BaseStation/blob/master/DCC%2B%2B%20Arduino%20Sketch.pdf

  Cette station n'a pour l'instant été testée qu'avec la carte moteur LMD18200 et adopte le brochage de DCC++ pour la voie principale (main)
  Par défaut dans ce programme, le brochage est :

  #define PIN_PWM       GPIO_NUM_12               // ENABLE (PWM)
  #define PIN_DIR       GPIO_NUM_13               // SIGNAL (DIR)
  #define PIN_BRAKE     GPIO_NUM_14               // CUTOUT (BRAKE)
  #define CURRENT_MONITOR_PIN_MAIN  GPIO_NUM_36   // Mesure de courant

****************************************************************************************************************************************
  ATTENTION à parametrer le protection de courant à une valeur correspondant à votre configuration
  #define CURRENT_SAMPLE_MAX 3200 // 2,7 V étant une valeur raisonnable à ne pas dépasser
****************************************************************************************************************************************

        Christophe Bobille / LOCODUINO (http://www.locoduino.org) janv 2022 © locoduino 2022 - christophe.bobille@gmail.com
        Original description from DCC++ BASE STATION a C++ program written by Gregg E. Berman GNU General Public License.
        Original description from Pascal Barlier GNU General Public License

        v 1.6 oct  2022
        v 1.7 janv 2023  : Inclus la réception de commandes de traction par le bus CAN
        v 1.8 - 15 mars 2023  : Ajout de eStop, fonction emergency()
        v 1.9 - 23 mars 2023  : Optimisation du code
        v 2.0 - 09 oct 2023   : Correction bug sur pointeur CLIENT en TCP (WiFi)
        v 2.1 - 09 oct 2023   : Modification de CURRENT_MONITOR_PIN_MAIN  sur GPIO_NUM_36
*/

#ifndef ARDUINO_ARCH_ESP32
#error "Select an ESP32 board"
#endif

#include "DCC.h"
#include "Config.h"
#include "CurrentMonitor.h"

#ifdef CAN_INTERFACE
#include <ACAN_ESP32.h> // https://github.com/pierremolinaro/acan-esp32
#endif

// Mesure de courant
char msg[] = "<p2>";
CurrentMonitor mainMonitor(CURRENT_MONITOR_PIN_MAIN, msg); // create monitor for current on Main Track

#if COMM_INTERFACE == 0
HardwareSerial *CLIENT = &Serial;
#elif COMM_INTERFACE == 1
WiFiServer SERVER(PORT); // Create and instance of an WiFiServer
WiFiClient *CLIENT = nullptr;
#endif

DCC dcc;

void Task0(void *pvParameters);
void Task1(void *pvParameters);
void Task2(void *pvParameters);

void comm(INTERFACE *);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.printf("\n\nProject :    %s", PROJECT);
  Serial.printf("\nVersion :      %s", VERSION);
  Serial.printf("\nAuthor :       %s", AUTHOR);
  Serial.printf("\nFichier :      %s", __FILE__);
  Serial.printf("\nCompiled  :      %s - %s \n\n", __DATE__, __TIME__);

#ifdef CAN_INTERFACE
  //--- Configure ESP32 CAN
  Serial.println("Configure ESP32 CAN");
  ACAN_ESP32_Settings settings(CAN_BITRATE);
  settings.mRxPin = CAN_RX;
  settings.mTxPin = CAN_TX;
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::singleStandardFilter(ACAN_ESP32_Filter::data, 0x1FA, 0);
  const uint32_t errorCode = ACAN_ESP32::can.begin(settings, filter);
  if (errorCode == 0)
    Serial.println("Can configuration OK !\n");
  else
  {
    Serial.printf("Can configuration error 0x%x\n", errorCode);
    delay(1000);
    return;
  }
#endif

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
  IPAddress local_IP(LOCAL_IP);
  IPAddress gateway(LOCAL_GATEWAY);
  IPAddress subnet(SUBNET);
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

  xTaskCreatePinnedToCore(
      Task0,    /* Task function. */
      "Task0",  /* String with name of task. */
      3 * 1024, /* Stack size. */
      NULL,     /* Parameter passed as input of the task */
      2,        /* Priority of the task. */
      NULL,     /* Task handle. */
      0);       /* Core. */

  xTaskCreatePinnedToCore(
      Task1,
      "Task1",
      2 * 1024,
      NULL,
      2,
      NULL,
      1); /* Core. */

  xTaskCreatePinnedToCore(
      Task2,
      "Task2",
      2 * 1024,
      NULL,
      3,
      NULL,
      1); /* Core. */

  Serial.printf("End setup\n\n");
}

void Task0(void *p)
{

  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    mainMonitor.check();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
  }
}

void Task1(void *p)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  auto comm = [](INTERFACE *CLIENT)
  {
    if (CLIENT->available())
    {
      char c;
      char commandTxt[16];
      size_t commandLen = 0;
      for (uint8_t i = 0; i < 16; i++)
      {
        c = CLIENT->read();
        switch (c)
        {
        case '<':
          commandLen = 0;
          break;
        case '>':
          commandTxt[commandLen] = '\0';
          dcc.parse(commandTxt, CLIENT);
          break;
        default:
          commandTxt[commandLen++] = c;
        }
      }
    }
  };

  for (;;)
  {
#if COMM_INTERFACE == 0
    CLIENT = &Serial;
    comm(CLIENT);
#elif COMM_INTERFACE == 1
    WiFiClient client = SERVER.available();
    if (client)
    {
      CLIENT = &client;
      while (CLIENT->connected())
        comm(CLIENT);
    }
#endif
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
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
  }
}

void Task2(void *p)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    if (CLIENT != nullptr)
    {
      mainMonitor.over(CLIENT);
      CLIENT->printf("<a %d>", (int)(mainMonitor.current() / 4));
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
  }
}

void loop() {}
