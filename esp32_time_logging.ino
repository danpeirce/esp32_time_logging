#include "FS.h"
//#include "SPIFFS.h" 
#include "LittleFS.h"
#include <time.h> 
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define SPIFFS LittleFS

/* This examples uses "quick re-define" of SPIFFS to run 
   an existing sketch with LittleFS instead of SPIFFS

   You only need to format LittleFS the first time you run a
   test or else use the LittleFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin */
   
//  Documenting file name limitations #729 https://github.com/littlefs-project/littlefs/issues/729

#define FORMAT_LITTLEFS_IF_FAILED false

long timezone = -8; 
byte daysavetime = 1;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.print (file.name());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.print(file.size());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}



void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void checkLogFile(fs::FS &fs, const char * path, char *starttime){
    Serial.printf("Checking file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.printf("Creating file: %s\n", path);
	    writeFile(LittleFS, path, starttime);
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void setup(){
    Serial.begin(115200);
    // We start by connecting to a WiFi network

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

        // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("AutoConnectAP","physicsPMT"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected... ");
    }
    Serial.println();
    Serial.println();
    //Serial.print("Connecting to ");
    //Serial.println(ssid);

    //WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Contacting Time Server");
	  configTime(3600*timezone, daysavetime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
	  struct tm tmstruct ;
    delay(2000);
    tmstruct.tm_year = 0;
    getLocalTime(&tmstruct, 5000);
	  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);
    Serial.println("");
    static char starttime[32];
    sprintf(starttime, "%d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }

  checkLogFile(LittleFS, "/LogFile.txt", starttime);   // reads file if it exists otherwise creates file and wrights time to file
  appendFile(LittleFS, "/LogFile.txt", starttime);    // logs time of reset
  listDir(LittleFS, "/", 1);
}

void loop(){

}

  /*   lines removed from original example
    Serial.println("----list 1----");
    listDir(SPIFFS, "/", 1);
	
    Serial.println("----remove old dir----");
    removeDir(SPIFFS, "/mydir");
	
    Serial.println("----create a new dir----");
    createDir(SPIFFS, "/mydir");
	
    Serial.println("----remove the new dir----");
    removeDir(SPIFFS, "/mydir");
	
    Serial.println("----create the new again----");
    createDir(SPIFFS, "/mydir");
	
    Serial.println("----create and work with file----");
    writeFile(SPIFFS, "/mydir/hello.txt", "Hello ");
    appendFile(SPIFFS, "/mydir/hello.txt", "World!\n");

    Serial.println("----list 2----");
    listDir(SPIFFS, "/", 1);
	
    Serial.println("----attempt to remove dir w/ file----");
    removeDir(SPIFFS, "/mydir");
	
    Serial.println("----remove dir after deleting file----");
    deleteFile(SPIFFS, "/mydir/hello.txt");
    removeDir(SPIFFS, "/mydir");
	
	Serial.println("----list 3----");
    listDir(SPIFFS, "/", 1);
	
	Serial.println( "Test complete" );
  */