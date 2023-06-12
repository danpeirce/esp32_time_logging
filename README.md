# ESP32 Time Logging

Uses LittleFS to store log file. A new time stamp is added to log file after each reset (including on power up).

Also uses WiFi_Manager to connect to an access point. The WiFiManager stores SSID and pass phrase. If SSID information 
is not stored for current location the WiFi Manager will instead provide an access point. One connects to the ESP32 access
point and points a web browser to 192.168.4.1 to display a web form. One fills the SSID and pass phrase into the form (using a phone or laptop).
The ESP32 then goes into station mode, connects to a WiFi router and uses a NTP time server to get the time and date. It sets
the ESP32 real time clock to the time obtained.

The program appends the log file with the current time.