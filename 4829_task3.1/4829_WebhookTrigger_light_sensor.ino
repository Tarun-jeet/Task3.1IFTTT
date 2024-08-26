#include <SPI.h>
#include <WiFiNINA.h>
#include <hp_BH1750.h>
#include "arduino_secrets.h" 

//sensitive data stored in header file
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

hp_BH1750 BH1750;       //  creating the sensor
int status = WL_IDLE_STATUS;

//initializing the wifi client library
WiFiClient client;
float threshold = 700; //for daylight
bool sunlight = false;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) 
  {
    ; // wait for serial port to connect. 
  }

  // check for the WiFi module to connect
  ConnectToWifi();
  
  // wait 10 seconds for connection:
    delay(10000);
  
  Serial.println("Connected to WiFi");
  printWifiStatus();

  //checking if the BH1750 sensor connected
  bool avail = BH1750.begin(BH1750_TO_GROUND);
  //if not on the pin then give not found
  if (!avail) 
  {
    Serial.println("No BH1750 sensor found!");
    while (true) {};                                        
  }
}

void loop() {
 
    float intensity_lux = Check_the_intensity_lux();
    delay(5000);
    Notification_Send_Conditions(intensity_lux);
    
    while (client.connected()) 
    {
      if (client.available()) 
      {
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
      }
    
  // if the server's disconnected, stop the client
  if (!client.connected()) 
  {
    client.stop();
  }
}

//function for the time when to send the request 
  void Notification_Send_Conditions(float intensity)
  {
    //when the sunlight is on terrarium then send webhook request
    if(intensity>threshold && !sunlight)
    {
      sunlight=true;
      Send_HTTP_request("maker.ifttt.com","/trigger/sent_request/with/key/cGcUN4qQK6xbK1gp1tq1P8zal7F3M5cn0QPzCkrT8J6");
      Serial.println(intensity);
      
    }

    //when the terrarium is not in sunlight exposure then send webhook
    else if(intensity<=threshold && sunlight)
    {
      sunlight=false; 
     Send_HTTP_request("maker.ifttt.com","/trigger/sent_request2/with/key/cGcUN4qQK6xbK1gp1tq1P8zal7F3M5cn0QPzCkrT8J6");
     Serial.println(intensity);
     
    }
  }
  
//printing the wifi status, signal strength and ip address
void printWifiStatus() {
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//function to connect to wifi
void ConnectToWifi()
{
  //checking for the wifi module if connected or not with conditional statement
  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue if not connected
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  //checking if the module has latest wifi firmware
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) 
  {
    Serial.println("Please upgrade the firmware");
  }

  //if not connected to wifi then trying unitl connected
  while (status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }
}

//function taking parameters server and the path name to send a webhook request
void Send_HTTP_request(char server[],String path_name)
{
  Serial.println("\nStarting connection to server...");
  //connecting via 80 web port
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Making a HTTP request:
    client.println("GET "+path_name +" HTTP/1.1");
    client.println("Host: "+String(server));
    client.println("Connection: close");
    client.println();
}
}

//function for the intensity calcultion from the sensor
float Check_the_intensity_lux()
{
  BH1750.start();   //starts a measurement
  float lux = BH1750.getLux();//  waits until a conversion finished
  Serial.println(lux);
  return lux;
}
  
