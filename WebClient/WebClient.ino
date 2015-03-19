#include <aJSON.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <rgb_lcd.h>

rgb_lcd lcd;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0x98, 0x4F, 0xEE, 0x01, 0x87, 0x30 };

// Azure Mobile Service address
// You can find this in your service dashboard
const char *server = "spartakiade2015.azure-mobile.net";
 
// Azure Mobile Service table name
// The name of the table you created
const char *table_name = "telemetry";
 
// Azure Mobile Service Application Key
// You can find this key in the 'Manage Keys' menu on the dashboard
const char *ams_key = "eCwuHEJspgPeuOSiEDFapozuMfKMTc29";

// Define the pin to which the temperature sensor is connected.
const int pinTemp = A0;

// Define the B-value of the thermistor.
// This value is a property of the thermistor used in the Grove - Temperature Sensor,
// and used to convert from the analog value it measures and a temperature value.
const int B = 3975;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// String buffers for the ethernet client, don't care
// we have enough ram on Intel Galileo
char buffer[64];
const int MAXBUFFER = 512;
static char stringBuffer[MAXBUFFER];

void send_telemetry_data(float value)
{
  Serial.println("\nconnecting...");
 
  if (client.connect(server, 80)) {
 
    Serial.print("sending ");
    Serial.println(value);
 
    // POST URI
    sprintf(buffer, "POST /tables/%s HTTP/1.1", table_name);
    client.println(buffer);
 
    // Host header
    sprintf(buffer, "Host: %s", server);
    client.println(buffer);
 
    // Azure Mobile Services application key
    sprintf(buffer, "X-ZUMO-APPLICATION: %s", ams_key);
    client.println(buffer);
 
    // JSON content type
    client.println("Content-Type: application/json");
 
    // POST body
    sprintf(buffer, "{\"value\": %f}", value);
 
    // Content length
    client.print("Content-Length: ");
    client.println(strlen(buffer));
 
    // End of headers
    client.println();
 
    // Request body
    client.println(buffer);
    
  } 
  else 
  {
    Serial.println("connection failed");
  }
}

/*
** Wait for response
*/
void wait_response()
{
  while (!client.available()) 
  {
    if (!client.connected()) 
    {
      Serial.println("Disconnected");
      return;
    }
  }
}
 
/*
** Read the response and dump to serial
*/
void read_response()
{
  bool print = true;
  
  while (client.available()) 
  {
    char c = client.read();
    // Print only until the first carriage return
    if (c == '\n')
    {
      print = false;
    }
    if (print)
    {
      Serial.print(c);
    }
  }
  Serial.print("\n");
}

/*
** Send request to get the message from azure
*/
void send_message_request()
{
  Serial.println("Reading message.");
  if (client.connect(server, 80)) 
  {
    Serial.println("connected");
    lcd.clear();
    lcd.print("Server connected");
    // Make a HTTP request:
            
    // POST URI
    client.print("GET /tables/messages?$top=1&$orderby=__createdAt%20desc HTTP/1.1");
                
    sprintf(buffer, "");
    client.println(buffer);
             
    // Host header
    sprintf(buffer, "Host: %s", server);
    client.println(buffer);
             
    // Azure Mobile Services application key
    sprintf(buffer, "X-ZUMO-APPLICATION: %s", ams_key);
    client.println(buffer);
             
    client.println("Connection: close");
    client.println();
    
    Serial.println("Sended");
  }
  else
  {
    Serial.println("Cannot connect.");
  }
}

/*
** Read the response, parse the json and display
*/
void read_message_response()
{
  // if there are incoming bytes available 
  // from the server, read them and print them:
  boolean jsonFound = false;
  int bytes = 0;
  while (client.available()) 
  {
    char c = client.read();
    //look for first [, yes it could be something else but I'm thinking positively.
    if (c == '[')
    {
      jsonFound = true;
    }
    if (!jsonFound) 
    {
      continue;
    }
	
    stringBuffer[bytes++] = c;
    if (bytes >= MAXBUFFER)
   {
      break; //that's all we have room for or we're done
    }
  }

  Serial.write(stringBuffer); // let's see what we got

  aJsonObject* root = aJson.parse(stringBuffer);
  if (root != NULL) 
  {
    Serial.println("Root not null");
    aJsonObject* element = aJson.getArrayItem(root, 0);
    if(element != NULL)
    {
      Serial.println("element not null");
          
      String mesg = read_json_string(element, "Message");
      lcd.setCursor(0, 1);
      lcd.print(mesg);
      int r = read_json_int(element, "R");
      int g = read_json_int(element, "G");
      int b = read_json_int(element, "B");
      lcd.setRGB(r, g, b);
    }
    else
    {
      Serial.println("Element null");
    }
  }
}

/*
** Read the message from mobile service and displays it
*/
void read_message()
{
  send_message_request();
  wait_response();
  read_message_response();	
}
   
/*
** Read named string from an json element
*/
String read_json_string(aJsonObject* element, char* name)
{
  aJsonObject* msg = aJson.getObjectItem(element, name);
  if(msg != NULL)
  {
    Serial.println("msg not null");
    return msg->valuestring;
  }
  Serial.println("msg is null");
  return "";
}

/*
** Read named integer value from an json element
*/
int read_json_int(aJsonObject* element, char* name)
{
  aJsonObject* msg = aJson.getObjectItem(element, name);
  if(msg != NULL)
  {
    Serial.println("msg not null");
    return msg->valueint;
  }
  Serial.println("msg is null");
  return 0;
}

/*
** Close the connection
*/
void end_request()
{
  client.stop();
}

float get_temperature()
{
    // Get the (raw) value of the temperature sensor.
    int val = analogRead(pinTemp);

    // Determine the current resistance of the thermistor based on the sensor value.
    float resistance = (float)(1023-val)*10000/val;

    // Calculate the temperature based on the resistance value.
    float temperature = 1/(log(resistance/10000)/B+1/298.15)-273.15;
    
    Serial.println(temperature);
    
    return temperature;
  }
 
 
/*
** Arduino Setup
*/
void setup() 
{
  Serial.begin(9600);
  while (!Serial) 
  {
    ; // wait for serial port to connect.
  }
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
 
  Serial.println("ethernet");
 
  if (Ethernet.begin(mac) == 0) 
  {
    Serial.println("ethernet failed");
    for (;;) ;
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop()
{
    float temperature = get_temperature();
    //send_request(temperature);
    //wait_response();
    //read_response();
    //end_request();
    
    read_message();
    end_request();

    // Wait one second between measurements.
    delay(10000);
}

