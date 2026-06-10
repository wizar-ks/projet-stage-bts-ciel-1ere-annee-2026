#include <Wire.h>
#include <U8g2lib.h>

#include "FS.h"
#include <SD.h>

#include "BLEDevice.h"

#define PIN_SPI_CS 5 // CS pin

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

int temps = 0; //nombre de la durée d'enregistrement 
int entrer = 0; //initialisation de la lecture de signal
//int i = 0; 
int choix = 0; //choix du temps
int enregistrement = 1000; // temps d'enregistrement de la carte sd en ms

int batterie_rest = 10; //initialsation de la batterie en pourcentage


///=============================================Millis============================================//
long intervalle_temps = 5000;
long temps_ancien;

long intervalle_temps2 = 1000; //connexion bluetooth
long temps_ancien2;

long temps_ancien_carteSD = 0;
long interval_notif_erreur_carteSD = 10000; //message erreur carte sd sur 10 secondes

long ancien_temps_sauvegarde_carteSD = 0;
long interval_notif_sauvegarde_carteSD = 15000; //inverval de sauvegarde des donnee 

long ancien_temps_suppresion_data_carteSD = 0;
long interval_temps_suppresion_data_carteSD = 30000; //choix sauvegarde fichier ou supprimer

long temps_buffer_ancien = 0;
long buffer_interval = 100; //temps buffer 


//=============================================BLE============================================//
// The remote service we wish to connect to.
static BLEUUID serviceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("cba1d466-344c-4be3-ab3f-189f80dd7518");  

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

bool information_data = false; // permet de sauvegarder dans la carte SD quand le BLE recois des données

bool choix_fichier = false;

bool fichier_enregistrement_0 = false;
bool fichier_enregistrement_1 = false; //permet de selectionné le fichier et d'ecrire dedans
bool fichier_enregistrement_2 = false;
bool fichier_enregistrement_3 = false;
bool fichier_enregistrement_4 = false;

int data_1 = 0;//information pData
int data_2 = 0;
int data_3 = 0;
int data_4 = 0;

//=============================================BUFFER============================================//

  #define emplacement_stock_data_1 20
  #define emplacement_stock_data_2 20
  #define emplacement_stock_data_3 20
  #define emplacement_stock_data_4 20

  int stock_1[emplacement_stock_data_1];
  int stock_2[emplacement_stock_data_2];
  int stock_3[emplacement_stock_data_3];
  int stock_4[emplacement_stock_data_4];

  int head_data_1 = 0;
  int tail_data_1 = 0;
  bool FULL_data_1 = false;

  int head_data_2 = 0;
  int tail_data_2 = 0;
  bool FULL_data_2 = false;

  int head_data_3 = 0;
  int tail_data_3 = 0;
  bool FULL_data_3 = false;

  int head_data_4 = 0;
  int tail_data_4 = 0;
  bool FULL_data_4 = false;


//==========================fichier=================================//
File myFile;//mon fichier 


void ajouterData_1(int code_1)
{

    stock_1[head_data_1] = code_1;

    if (FULL_data_1 == true)
    {
    tail_data_1 =  (tail_data_1 + 1) % emplacement_stock_data_1; // si depassement
    }

    head_data_1 = (head_data_1 + 1) % emplacement_stock_data_1;

    FULL_data_1 = (head_data_1 == tail_data_1);
}

  int recupererData_1()
{
    if((head_data_1 == tail_data_1) && !FULL_data_1)
    {
      Serial.println("buffer vider");
      return 0;
    }

    int code_1;

    code_1 = stock_1[tail_data_1];

    tail_data_1 = (tail_data_1 + 1) % emplacement_stock_data_1;

    FULL_data_1 = false;

    return code_1;
}

void ajouterData_2(int code_2)
{

    stock_2[head_data_2] = code_2;

    if (FULL_data_2 == true)
    {
    tail_data_2 =  (tail_data_2 + 1) % emplacement_stock_data_2; // si depassement
    }

    head_data_2 = (head_data_2 + 1) % emplacement_stock_data_2;

    FULL_data_2 = (head_data_2 == tail_data_2);
}

  int recupererData_2()
{
    if((head_data_2 == tail_data_2) && !FULL_data_2)
    {
      Serial.println("buffer vider");
      return 0;
    }

    int code_2;

    code_2 = stock_2[tail_data_2];

    tail_data_2 = (tail_data_2 + 1) % emplacement_stock_data_2;

    FULL_data_2 = false;

    return code_2;
}

void ajouterData_3(int code_3)
{

    stock_3[head_data_3] = code_3;

    if (FULL_data_3 == true)
    {
    tail_data_3 =  (tail_data_3 + 1) % emplacement_stock_data_3; // si depassement
    }

    head_data_3 = (head_data_3 + 1) % emplacement_stock_data_3;

    FULL_data_3 = (head_data_3 == tail_data_3);
}

  int recupererData_3()
{
    if((head_data_3 == tail_data_3) && !FULL_data_3)
    {
      Serial.println("buffer vider");
      return 0;
    }

    int code_3;

    code_3 = stock_3[tail_data_3];

    tail_data_3 = (tail_data_3 + 1) % emplacement_stock_data_3;

    FULL_data_3 = false;

    return code_3;
}

void ajouterData_4(int code_4)
{

    stock_4[head_data_4] = code_4;

    if (FULL_data_4 == true)
    {
    tail_data_4 =  (tail_data_4 + 1) % emplacement_stock_data_4; // si depassement
    }

    head_data_4 = (head_data_4 + 1) % emplacement_stock_data_4;

    FULL_data_4 = (head_data_4 == tail_data_4);
}

  int recupererData_4()
{
    if((head_data_4 == tail_data_4) && !FULL_data_4)
    {
      Serial.println("buffer vider");
      return 0;
    }

    int code_4;

    code_4 = stock_4[tail_data_4];

    tail_data_4 = (tail_data_4 + 1) % emplacement_stock_data_4;

    FULL_data_4 = false;

    return code_4;
}


// Callback function to handle notifications
static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) { // ce qui permet d'avoir le résultat
  //Serial.print("Longueur data : ");
  //Serial.println(length);
  
  //Serial.print("Données : ");
  for (size_t i = 0; i < length; i++) {
    //Serial.print(pData[i]); // chiffre 
    //Serial.print(" ");
  }
  //Serial.println();
 if (isNotify == true)
  {
    information_data = true;
  }

 data_1 = 256*pData[0]+pData[1];
 data_2 = 256*pData[2]+pData[3];
 data_3 = 256*pData[4]+pData[5];
 data_4 = 256*pData[6]+pData[7];

ajouterData_1(data_1);
ajouterData_2(data_2);
ajouterData_3(data_3);
ajouterData_4(data_4);

  //Serial.println(data_1);
  //Serial.println(data_2);
  //Serial.println(data_3);
  //Serial.println(data_4);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {}

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  pClient->setMTU(517);  //set client to request maximum MTU from server (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead()) {
    String value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteCharacteristic->canNotify()) {
    // Register/Subscribe for notifications
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  }

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    //Serial.print("BLE Advertised Device found: "); //affiche les information par exemple
    //Serial.println(advertisedDevice.toString().c_str()); // Name: , Address: 2e:5f:ac:8e:af:36, manufacturer data: 060001092022d14b4a221f42d28aefae4dc1778960f532a416b7f97ebe, rssi: -73

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }  // Found our server
  }  // onResult
};  // MyAdvertisedDeviceCallbacks


void init_bluetooth()
{
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void connect_bluetooth()
{
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothing more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    String newValue = "Time since boot: " + String(millis() / 1000);
    //Serial.println("Setting new characteristic value to \"" + newValue + "\"");

    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  } else if (doScan) {
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
}
  
void affichage_batterie()
{
  u8g2.clearBuffer();					// Nettoyage interne de la mémoire

  u8g2.setFont(u8g2_font_ncenB08_tr);	// Choisir la police de charactère
  u8g2.setCursor(50,10); 
  u8g2.print(data_1);	// Ce qu'on veut faire afficher sur l'écran comme variable

  u8g2.setFont(u8g2_font_ncenB08_tr);	// Choisir la police de charactère
  u8g2.setCursor(50,20); 
  u8g2.print(data_2);	// Ce qu'on veut faire afficher sur l'écran comme variable

  u8g2.setFont(u8g2_font_ncenB08_tr);	// Choisir la police de charactère
  u8g2.setCursor(50,30); 
  u8g2.print(data_3);	// Ce qu'on veut faire afficher sur l'écran comme variable

  u8g2.setFont(u8g2_font_ncenB08_tr);	// Choisir la police de charactère
  u8g2.setCursor(50,40); 
  u8g2.print(data_4);	// Ce qu'on veut faire afficher sur l'écran comme variable

  //u8g2.drawStr(65,30,"%");	// Ce qu'on veut faire afficher sur l'écran premier 0 = axe horizontal  et deuxième 0 = axe vertical

  u8g2.sendBuffer();// transfert de mémoire
}

void carte_sd()
{

  if (!SD.begin(PIN_SPI_CS)) { //pin CS
    while (1) {
      Serial.println(F("Erreur de la cart SD, Ou elle n'est pas present !"));
      delay(1000);
    }
  }

  if (!SD.exists("/batterie.txt")) {
    Serial.println(F("batterie.txt n'existe pas. creation du fichier batterie.txt"));
    // creer un nouveau fichier pour ouvrir directement le fichier puis le fermer
    myFile = SD.open("/batterie.txt");
    writeFile(SD, "/batterie.txt", "batterie restante, consomation batterie en W ,total utilisation batterie en W, date d'enregistrement  \r\n");
    myFile.close();
  }

  if (!SD.exists("/enregistrement_1.txt")) 
  {
    Serial.println(F("fichier enregistrement_1 n'existe pas. creation du fichier enregistrement_1.txt"));
    // creer un nouveau fichier pour ouvrir directement le fichier puis le fermer
    myFile = SD.open("/enregistrement_1.txt");
    writeFile(SD, "/enregistrement_1.txt", "batterie restante, consomation batterie en W ,total utilisation batterie en W, date d'enregistrement  \r\n");
    myFile.close();
  }

  if (!SD.exists("/enregistrement_2.txt")) 
  {
    Serial.println(F("fichier enregistrement_2 n'existe pas. creation du fichier enregistrement_2.txt"));
    // creer un nouveau fichier pour ouvrir directement le fichier puis le fermer
    myFile = SD.open("/enregistrement_2.txt");
    writeFile(SD, "/enregistrement_2.txt", "batterie restante, consomation batterie en W ,total utilisation batterie en W, date d'enregistrement  \r\n");
    myFile.close();
  }

    if (!SD.exists("/enregistrement_3.txt")) 
  {
    Serial.println(F("fichier enregistrement_3 n'existe pas. creation du fichier enregistrement_3.txt"));
    // creer un nouveau fichier pour ouvrir directement le fichier puis le fermer
    myFile = SD.open("/enregistrement_3.txt");
    writeFile(SD, "/enregistrement_3.txt", "batterie restante, consomation batterie en W ,total utilisation batterie en W, date d'enregistrement  \r\n");
    myFile.close();
  }

    if (!SD.exists("/enregistrement_4.txt")) 
  {
    Serial.println(F("fichier enregistrement_4 n'existe pas. creation du fichier enregistrement_4.txt"));
    // creer un nouveau fichier pour ouvrir directement le fichier puis le fermer
    myFile = SD.open("/enregistrement_4.txt");
    writeFile(SD, "/enregistrement_4.txt", "batterie restante, consomation batterie en W ,total utilisation batterie en W, date d'enregistrement  \r\n");
    myFile.close();
  }

}
//===============================================================
//                ECRIRE SUR LA CARTE SD (TITRE)
//===============================================================
void writeFile(fs::FS &fs, const char * path, const char * message) 
{
  //Serial.printf("Ecriture: %s\n", path);

  File myFile = fs.open(path , FILE_WRITE);
  if(!myFile) 
    {
    Serial.println("Erreur d'ouverture du fichier pour ecrire");
    return;
    }
  if(myFile.print(message)) 
    {
    Serial.println("le contenu a bien ete ecris");
    } 
  else 
    {
    Serial.println("Erreur d'ecriture");
    }
  myFile.close();
}
//===============================================================
//                ECRIRE DU TEXT DANS LA CARTE SD
//===============================================================
void appendFile(fs::FS &fs, const char * path, const char * message) 
{
  //Serial.printf("Appending to file: %s\n", path);

  File myFile = fs.open(path, FILE_APPEND);
  if(!myFile) 
    {
    Serial.println();
    Serial.println("Erreur d'ouverture du fichier pour l'ajout");
    return;
    }
  if(myFile.print(message)) 
    {
    Serial.println();
    Serial.println("Message ajouter");
    } 
  else 
    {
    Serial.println();
    Serial.println("Erreur d'ajout");
    }
  myFile.close();
}

void sauvegarde_boucle ()
{

  if (SD.exists("/batterie.txt") && information_data == true)
  {

        myFile = SD.open("/batterie.txt");
        String batterie = String(data_1) + "                " + String(data_2) + "                        " + String(data_3)  + "                              " + String(data_4) + "\r\n";

        //Serial.println(batterie);  // Afficher dans le moniteur série
        appendFile(SD, "/batterie.txt", batterie.c_str()); // Sauvegarde sur la carte SD

        Serial.println("sauvegarde effectue ! valeur :");
        Serial.print(data_1);
        Serial.print("  /   ");
        Serial.print(data_2);
        Serial.print("  /   ");
        Serial.print(data_3);
        Serial.print("  /   ");
        Serial.print(data_4);

        myFile.close();
  }
  else if (!SD.exists("/batterie.txt"))
  {
    Serial.println("le fichier n'existe pas");

  }
  else
  {
    Serial.println("il est impossible de retrouver les information bluetooth");
  }
}

void choix_fichier_enregistrement_1()
{
  if (SD.exists("/enregistrement_1.txt") && information_data == true)
  {

    myFile = SD.open("/enregistrement_1.txt");
    String enregistrement_1 = String(data_1) + "                " + String(data_2) + "                        " + String(data_3)  + "                              " + String(data_4) + "\r\n";

     appendFile(SD, "/enregistrement_1.txt", enregistrement_1.c_str()); // Sauvegarde sur la carte SD

        Serial.println("sauvegarde effectue ! valeur :");
        Serial.print(data_1);
        Serial.print("  /   ");
        Serial.print(data_2);
        Serial.print("  /   ");
        Serial.print(data_3);
        Serial.print("  /   ");
        Serial.print(data_4);

    myFile.close();

  }
  else if (!SD.exists("/enregistrement_1.txt"))
  {
    Serial.println("le fichier n'existe pas");

  }
  else
  {
     Serial.println("il est impossible de retrouver les information bluetooth");
  }
}

void choix_fichier_enregistrement_2()
{
  if (SD.exists("/enregistrement_2.txt") && information_data == true)
  {
    myFile = SD.open("/enregistrement_2.txt");
    String enregistrement_2 = String(data_1) + "                " + String(data_2) + "                        " + String(data_3)  + "                              " + String(data_4) + "\r\n";

     appendFile(SD, "/enregistrement_2.txt",enregistrement_2.c_str()); // Sauvegarde sur la carte SD

        Serial.println("sauvegarde effectue ! valeur :");
        Serial.print(data_1);
        Serial.print("  /   ");
        Serial.print(data_2);
        Serial.print("  /   ");
        Serial.print(data_3);
        Serial.print("  /   ");
        Serial.print(data_4);

        myFile.close();
  }
  else if (!SD.exists("/enregistrement_2.txt"))
  {
    Serial.println("le fichier n'existe pas");

  }
  else
  {
     Serial.println("il est impossible de retrouver les information bluetooth");
  }
}

void choix_fichier_enregistrement_3()
{
  if (SD.exists("/enregistrement_3.txt") && information_data == true)
  {
    myFile = SD.open("/enregistrement_3.txt");
    String enregistrement_3 = String(data_1) + "                " + String(data_2) + "                        " + String(data_3)  + "                              " + String(data_4) + "\r\n";

     appendFile(SD, "/enregistrement_3.txt", enregistrement_3.c_str()); // Sauvegarde sur la carte SD

        Serial.println("sauvegarde effectue ! valeur :");
        Serial.print(data_1);
        Serial.print("  /   ");
        Serial.print(data_2);
        Serial.print("  /   ");
        Serial.print(data_3);
        Serial.print("  /   ");
        Serial.print(data_4);

    myFile.close();

  }
  else if (!SD.exists("/enregistrement_3.txt"))
  {
    Serial.println("le fichier n'existe pas");

  }
  else
  {
     Serial.println("il est impossible de retrouver les information bluetooth");
  }
}

void choix_fichier_enregistrement_4()
{
  if (SD.exists("/enregistrement_4.txt") && information_data == true)
  {
      myFile = SD.open("/enregistrement_4.txt");
      String enregistrement_4 = String(data_1) + "                " + String(data_2) + "                        " + String(data_3)  + "                              " + String(data_4) + "\r\n";

      appendFile(SD, "/enregistrement_4.txt", enregistrement_4.c_str()); // Sauvegarde sur la carte SD

        Serial.println("sauvegarde effectue ! valeur :");
        Serial.print(data_1);
        Serial.print("  /   ");
        Serial.print(data_2);
        Serial.print("  /   ");
        Serial.print(data_3);
        Serial.print("  /   ");
        Serial.print(data_4);

    myFile.close();

  }
  else if (!SD.exists("/enregistrement_4.txt"))
  {
    Serial.println("le fichier n'existe pas");

  }
  else
  {
     Serial.println("il est impossible de retrouver les information bluetooth");
  }
}

void choix_carteSD_moniteur_de_serie()
{ 
  if (choix_fichier == false)
  {

        if(Serial.available() > 0)
        {
          entrer = Serial.read();
          switch (entrer)
          {
          
          case '0':

          sauvegarde_boucle ();

          Serial.println("Choix fichier abtterie selectionné");

          fichier_enregistrement_0 = true;

          choix_fichier = true;

          break;

          case '1':
          Serial.println("Choix fichier 1 selectionné");
          choix_fichier_enregistrement_1();

          fichier_enregistrement_1 = true;

          choix_fichier = true;

          break;

          case '2':

          Serial.println("Choix fichier 2 selectionné");
          choix_fichier_enregistrement_2();

          fichier_enregistrement_2 = true;

          choix_fichier = true;
          break;

          case '3':
          Serial.println("Choix fichier 3 selectionné");
          choix_fichier_enregistrement_3();

          fichier_enregistrement_3 = true;

          choix_fichier = true;
          break;

          case '4':
          Serial.println("Choix fichier 4 selectionné");
          choix_fichier_enregistrement_4();

          fichier_enregistrement_4 = true;

          choix_fichier = true;
          break;
          }
        }

      long temps_suppresion_data_carteSD = millis();

      if(temps_suppresion_data_carteSD - ancien_temps_suppresion_data_carteSD > interval_temps_suppresion_data_carteSD )
      {
       Serial.println("delai depassé choix fichier batterie selectionné par défault");

       fichier_enregistrement_0 = true;

       choix_fichier= true;
      }
  }    
}

void setup() 
{
  Serial.begin(115200);

  Serial.println("Choirsir dans quel fichier sauvegarder les données?");
  Serial.println(" 0 = fichier batterie || 1 = fichier data_1 || 2 = fichier data_2 || 3 = fichier data_3 || 4 = fichier data_4");
  Serial.println("!! après une minutes le fichier batterie sera selectionné !!");

  init_bluetooth();

  carte_sd();

  u8g2.begin();

}
void loop() 
{

  choix_carteSD_moniteur_de_serie() ;

  long temps_actu =millis();

  /*
  if (temps_actu-temps_ancien > intervalle_temps)//5 secondes d'interval
  {

    sauvegarde_temps();

    temps_ancien = temps_actu;
  }
*/

  long temps_actu2 =millis();

  if (temps_actu2-temps_ancien2 > intervalle_temps2)// 1 seconde d'interval
  {
    connect_bluetooth();
    
    temps_ancien2 = temps_actu2;
  }

  long temps_buffer = millis();

  if (temps_buffer - temps_buffer_ancien > buffer_interval)// 100ms d'interval 
  {
    int save_data_1 = recupererData_1();
    int save_data_2 = recupererData_2();
    int save_data_3 = recupererData_3();
    int save_data_4 = recupererData_4();

    Serial.print("data_1_buffer = ");
    Serial.println(save_data_1);

    Serial.print("data_2_buffer = ");
    Serial.println(save_data_2);

    Serial.print("data_3_buffer = ");
    Serial.println(save_data_3);

    Serial.print("data_4_buffer = ");
    Serial.println(save_data_4);
    
    temps_buffer_ancien = temps_buffer;

  }
  
  long temps_sauvegarde_carteSD = millis(); // delai 1 seonde
    if(temps_sauvegarde_carteSD - ancien_temps_sauvegarde_carteSD > interval_notif_sauvegarde_carteSD) 
    {

      if (fichier_enregistrement_0 == true)
      {
        sauvegarde_boucle();
      }

      if (fichier_enregistrement_1 == true)
      {
        choix_fichier_enregistrement_1();
      }

      if (fichier_enregistrement_2 == true)
      {
        choix_fichier_enregistrement_2();
      }

      if (fichier_enregistrement_3 == true)
      {
        choix_fichier_enregistrement_3();
      }

      if (fichier_enregistrement_4 == true)
      {
        choix_fichier_enregistrement_4();
      }

      ancien_temps_sauvegarde_carteSD = temps_sauvegarde_carteSD;
    } 

  affichage_batterie();
}
