#include <ESP8266WiFi.h>
#include <SPI.h>
#include <RFID.h>

#define SS_PIN D4
#define RST_PIN D3
#define RELAY_PIN D2 // Pin connected to the relay

RFID rfid(SS_PIN, RST_PIN);

// Setup variables:
const int MAX_REGISTERED_CARDS = 3;
const String registeredUIDs[MAX_REGISTERED_CARDS] = {
"93-94-17-a3-b3", //keychain : blue
"89-c8-b0-b2-43", // card RFID  
"11-22-33-FF-EE" // available 
};

const char* ssid = "smartdoorlockWiFi";
const char* password = "@dtecmelaka";

const char* host = "192.168.188.3"; //IP Address : server/client

WiFiClient client;
const int httpPort = 80;
String url;

unsigned long timeout;

const int id_device = 22;

void setup()
{
Serial.begin(115200);
delay(10);

pinMode(RELAY_PIN, OUTPUT); // Set relay pin as an output

SPI.begin();
rfid.init();


// We start by connecting to a WiFi network
Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);

WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED)
	{
	delay(500);
	Serial.print(".");
	}

	Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());

Serial.println("Put your card to the reader...");
Serial.println();
}

void loop()
{
if (rfid.isCard())
{
if (rfid.readCardSerial())
{
Serial.println(" ");
Serial.println("Card found");

String UIDcard = String(rfid.serNum[0], HEX) + "-" + String(rfid.serNum[1], HEX) + "-" + String(rfid.serNum[2], HEX) + "-" + String(rfid.serNum[3], HEX) + "-" + String(rfid.serNum[4], HEX);

if (isRegisteredUID(UIDcard))
{
Serial.println("Registered card detected: " + UIDcard);

Serial.print("connecting to ");
Serial.println(host);

if (!client.connect(host, httpPort))
{
Serial.println("connection failed");
return;
}

// We now create a URI for the request
url = "/rfidattendance/manage_users_up.php/rfid/insert?id_device=";
url += id_device;
url += "&rfid=";
url += UIDcard;

Serial.print("Requesting URL: ");
Serial.println(url);

// This will send the request to the server
client.print(String("GET ") + url + " HTTP/1.1\r\n" +
"Host: " + host + "\r\n" +
"Connection: close\r\n\r\n");
timeout = millis();
while (client.available() == 0)
{
if (millis() - timeout > 1000)
{
Serial.println(">>> Client Timeout !");
client.stop();
return;
}
}

// Read all the lines of the reply from server and print them to Serial
// while (client.available())
// {
//   String line = client.readStringUntil('\r');
//   Serial.print(line);
// }

Serial.println();
Serial.println("closing connection");
Serial.println();

// Control the relay to unlock the door
digitalWrite(RELAY_PIN, LOW); // Turn on the relay to unlock the door
delay(1000);                  // Keep the relay on for 1 second
digitalWrite(RELAY_PIN, HIGH); // Turn off the relay
}
else
{
Serial.println("Unregistered card detected: " + UIDcard);
}
}
}

rfid.halt();
}

bool isRegisteredUID(String UID)
{
for (int i = 0; i < MAX_REGISTERED_CARDS; i++)
{
if (UID.equals(registeredUIDs[i]))
{
return true;
}
}
return false;
}

