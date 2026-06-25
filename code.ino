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

long ancien_erreur_cartSD = 0;
long erreur_carte_sd_interval = 3000;
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

  #define emplacement_stock_data_1 21
  #define emplacement_stock_data_2 21
  #define emplacement_stock_data_3 21
  #define emplacement_stock_data_4 21

  int stock_1[emplacement_stock_data_1];
  int stock_2[emplacement_stock_data_2];
  int stock_3[emplacement_stock_data_3];
  int stock_4[emplacement_stock_data_4];

  bool buffer_plein_1 = false;
  bool buffer_vide_1 = true;

  bool buffer_plein_2 = false;
  bool buffer_vide_2 = true;

  bool buffer_plein_3 = false;
  bool buffer_vide_3 = true;

  bool buffer_plein_4 = false;
  bool buffer_vide_4 = true;

  int boucle_data_1 = 0;
  int boucle_data_2 = 0;
  int boucle_data_3 = 0;
  int boucle_data_4 = 0;

//==========================fichier=================================//
File myFile;//mon fichier 


void ajouterData_1(int code_1)
{
  if (buffer_vide_1 == true)
  {
    stock_1[boucle_data_1] = code_1;
    Serial.println(stock_1[boucle_data_1]);
    Serial.println(boucle_data_1);
    boucle_data_1++;

    if(boucle_data_1 == 20)
    {
      Serial.println("buffer 1 plein");
      buffer_vide_1 = false;
      buffer_plein_1 = true;
    }
    else if(boucle_data_1 > 20)
    {
      Serial.println("erreur buffer 1 dépassement");
      boucle_data_1 = 0;
    }
  }
}

void ajouterData_2(int code_2)
{
  if (buffer_vide_2 == true)
  {
    stock_2[boucle_data_2] = code_2;

    boucle_data_2++;

    if(boucle_data_2 == 20)
    {
      Serial.println("buffer 2 plein");
      buffer_vide_2 = false;
      buffer_plein_2 = true;
    }
  }
}

void ajouterData_3(int code_3)
{
  if (buffer_vide_3 == true)
  {
    stock_3[boucle_data_3] = code_3;

    boucle_data_3++;

    if(boucle_data_3 == 20)
    {
      Serial.println("buffer 3 plein");
      buffer_vide_3 = false;
      buffer_plein_3 = true;
    }
  }
}

void ajouterData_4(int code_4)
{
  if (buffer_vide_4 == true)
  {
    stock_4[boucle_data_4] = code_4;

    boucle_data_4++;

    if(boucle_data_4 == 20)
    {
      Serial.println("buffer 4 plein");
      buffer_vide_4 = false;
      buffer_plein_4 = true;
    }
  }
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
    writeFile(SD, "/batterie.txt", "tension \t Courant \t Température \t Temps  \r\n");
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
  if (SD.exists("/batterie.txt") && information_data == true && buffer_plein_1 == true && buffer_plein_2 == true && buffer_plein_3 == true && buffer_plein_4 == true )
  {

        myFile = SD.open("/batterie.txt");
        String batterie =     String(stock_1[0]) + "\t      " + String(stock_2[0]) + "\t" + String(stock_3[0])  + "\t" + String(stock_4[0]) + "\r\n" +
                              String(stock_1[1]) + "\t      " + String(stock_2[1]) + "\t" + String(stock_3[1])  + "\t" + String(stock_4[1]) + "\r\n" +
                              String(stock_1[2]) + "\t      " + String(stock_2[2]) + "\t" + String(stock_3[2])  + "\t" + String(stock_4[2]) + "\r\n" +
                              String(stock_1[3]) + "\t      " + String(stock_2[3]) + "\t" + String(stock_3[3])  + "\t" + String(stock_4[3]) + "\r\n" +
                              String(stock_1[4]) + "\t      " + String(stock_2[4]) + "\t" + String(stock_3[4])  + "\t" + String(stock_4[4]) + "\r\n" +
                              String(stock_1[5]) + "\t      " + String(stock_2[5]) + "\t" + String(stock_3[5])  + "\t" + String(stock_4[5]) + "\r\n" +
                              String(stock_1[6]) + "\t      " + String(stock_2[6]) + "\t" + String(stock_3[6])  + "\t" + String(stock_4[6]) + "\r\n" +
                              String(stock_1[7]) + "\t      " + String(stock_2[7]) + "\t" + String(stock_3[7])  + "\t" + String(stock_4[7]) + "\r\n" +
                              String(stock_1[8]) + "\t      " + String(stock_2[8]) + "\t" + String(stock_3[8])  + "\t" + String(stock_4[8]) + "\r\n" +
                              String(stock_1[9]) + "\t      " + String(stock_2[9]) + "\t" + String(stock_3[9])  + "\t" + String(stock_4[9]) + "\r\n" +
                              String(stock_1[10]) + "\t      " + String(stock_2[10]) + "\t" + String(stock_3[10])  + "\t" + String(stock_4[10]) + "\r\n" +
                              String(stock_1[11]) + "\t      " + String(stock_2[11]) + "\t" + String(stock_3[11])  + "\t" + String(stock_4[11]) + "\r\n" +
                              String(stock_1[12]) + "\t      " + String(stock_2[12]) + "\t" + String(stock_3[12])  + "\t" + String(stock_4[12]) + "\r\n" +
                              String(stock_1[13]) + "\t      " + String(stock_2[13]) + "\t" + String(stock_3[13])  + "\t" + String(stock_4[13]) + "\r\n" +
                              String(stock_1[14]) + "\t      " + String(stock_2[14]) + "\t" + String(stock_3[14])  + "\t" + String(stock_4[14]) + "\r\n" +
                              String(stock_1[15]) + "\t      " + String(stock_2[15]) + "\t" + String(stock_3[15])  + "\t" + String(stock_4[15]) + "\r\n" +
                              String(stock_1[16]) + "\t      " + String(stock_2[16]) + "\t" + String(stock_3[16])  + "\t" + String(stock_4[16]) + "\r\n" +
                              String(stock_1[17]) + "\t      " + String(stock_2[17]) + "\t" + String(stock_3[17])  + "\t" + String(stock_4[17]) + "\r\n" +
                              String(stock_1[18]) + "\t      " + String(stock_2[18]) + "\t" + String(stock_3[18])  + "\t" + String(stock_4[18]) + "\r\n" +
                              String(stock_1[19]) + "\t      " + String(stock_2[19]) + "\t" + String(stock_3[19])  + "\t" + String(stock_4[19]) + "\r\n" ;
                             
        appendFile(SD, "/batterie.txt", batterie.c_str()); // Sauvegarde sur la carte SD
        myFile.close();

        Serial.println(batterie);
      
      boucle_data_1 = 0;
      boucle_data_2 = 0;
      boucle_data_3 = 0;
      boucle_data_4 = 0;

      buffer_plein_1 = false;
      buffer_vide_1 = true;

      buffer_plein_2 = false;
      buffer_vide_2 = true;

      buffer_plein_3 = false;
      buffer_vide_3 = true;

      buffer_plein_4 = false;
      buffer_vide_4 = true;
  }

  long erreur_cartSD = millis();
  if(erreur_cartSD - ancien_erreur_cartSD > erreur_carte_sd_interval)
  {
    if (!SD.exists("/batterie.txt"))
    {
      Serial.println("le fichier n'existe pas");
    }
    else if(information_data == false)
    {
      Serial.println("il est impossible de retrouver les information bluetooth");
    }
    else
    {
      Serial.println("buffer non plein");
    }
  ancien_erreur_cartSD = erreur_cartSD;
  }
}

void setup() 
{
  Serial.begin(115200);

  init_bluetooth();

  carte_sd();

  u8g2.begin();

}
void loop() 
{
  long temps_actu2 =millis();

  if (temps_actu2-temps_ancien2 > intervalle_temps2)// 1 seconde d'interval
  {
    connect_bluetooth();
    
    temps_ancien2 = temps_actu2;
  }
  
  sauvegarde_boucle();

  affichage_batterie();
}
