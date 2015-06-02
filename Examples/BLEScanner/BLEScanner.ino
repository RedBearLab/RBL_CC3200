/*

Copyright (c) 2015 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/*

BLE Scanner

In this sketch, we use the WiFi Mini or RBL CC3200 board to connect to the BLE Mini to 
do a BLE to Wi-Fi gateway that scans the BLE devices in the area and displays them on
a webpage hosted by the WiFi Mini or RBL CC3200 board.

BLE Mini
https://github.com/RedBearLab/BLEMini

The BLE Mini running HCI firmware (UART with 57600 bps), the CC3200 sends commands 
and handle BLE events via the UART interface. This sketch requires BLE HCI library to run.

BLE HCI
https://github.com/RedBearLab/BLE_HCI

The default password of this demo is "00000000".

This demo requires to map the Serial1 to
pin 6 and 7 for connecting to the BLE Mini's serial.

There are two places of code to be changed:

1.
static const unsigned long g_ulUARTPinmode[2] =
{
//	PIN_MODE_3, PIN_MODE_7  // original
	PIN_MODE_3, PIN_MODE_2  // set pin function
};

2.
static const unsigned long g_ulUARTConfig[2][2] =
{
//	{PIN_57, PIN_55}, {PIN_02, PIN_01} // original
	{PIN_57, PIN_55}, {PIN_17, PIN_16} // map Serial1 to pin 17 and 16 of CC3200 module
};

*/

#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#endif

#define SCAN_BEACON
#define PRINT_DEV_NAME

#include <WiFi.h>
#ifdef SCAN_BEACON
#include <stdarg.h>
#include "typedef.h"
#include "blemini_central.h"
#include "ble_hci.h"
#include "central.h"
#endif /* SCAN_BEACON */

#define  PIN_COUNT  20 

// your network name also called SSID
char ssid[] = "RBL CC3200 CC2540";

// your network password
char password[] = "00000000";

// your network key Index number (needed only for WEP)
//int keyIndex = 0;

WiFiServer server(80);


#ifdef SCAN_BEACON
#define HTML_BUF_SIZE 640
static uint8_t buf_html[HTML_BUF_SIZE];
static uint16 received_len = 0;
static volatile uint8 number_devs = 0;
uint8_t found_address[6];
uint8 scan_ble();

void p(char *fmt, ... )
{
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end (args);
  Serial.print(tmp);
}

byte ble_event_available()
{
	return Serial1.available(); 
}

byte ble_event_process()
{
	uint8_t type, event_code, data_len, status1;
	uint16_t event;
	uint8_t buf[64];

	type = Serial1.read();
	delay(35);
	event_code = Serial1.read();
	data_len = Serial1.read();

	p("-----------------------------\r\n");
	p("-Type 	   : 0x%02X\r\n", type);
	p("-EventCode   : 0x%02X\r\n", event_code);
	p("-Data Length : 0x%02X\r\n", data_len);

	for (int i = 0; i < data_len; i++)
	 buf[i] = Serial1.read();
	 
	event = BUILD_UINT16(buf[0], buf[1]);
	status1 = buf[2];

	p(" Event	   : 0x%04X\r\n", event);
	p(" Status	   : 0x%02X\r\n", status1);

	switch (event)
	{
	 case 0x060D: // GAP_DeviceInformation
	 {
		#ifdef SCAN_BEACON
		 uint8 total_len = 0;
		#endif /* SCAN_BEACON */
		 
		 p("==> GAP_DeviceInformation\r\n");

		 int8_t rssi = buf[11];
		 p("RSSI: %d\r\n", rssi);

		 p("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", buf[10], buf[9], buf[8], buf[7], buf[6], buf[5]);
		 
		#ifdef SCAN_BEACON
		 if(type == 0x04 && event_code == 0xff) { /* event and HCI_LE_ExtEvent */
			 total_len = 3 + data_len;
			 if ( (received_len + total_len) > HTML_BUF_SIZE) {
				 p("\r\n!!!!!!!!!!!!!!! more then 10 devs !!!!!!!!!!\r\n");
				 break;
			 }
			 memcpy(&buf_html[received_len+0], &type, 1);
			 memcpy(&buf_html[received_len+1], &event_code, 1);
			 memcpy(&buf_html[received_len+2], &data_len, 1);
			 
			 memcpy(&buf_html[received_len+3], &buf[0], data_len);
			 received_len += total_len;
		 } else {
			 p("\r\n not HCI_LE_ExtEvent\r\n");
		 }
		#endif /* SCAN_BEACON */
	 }
	 break;
	 case 0x0601: // GAP_DeviceDiscoveryDone
	 {
		 p("==> GAP_DeviceDiscoveryDone\r\n");

		 uint8_t num_devs = buf[3];
		#ifdef SCAN_BEACON
		 number_devs = num_devs;
		#endif
		 p(" NumDevs	 : 0x%02X\r\n", num_devs);
		 
		 if (num_devs > 0)
		   memcpy(found_address, &buf[6], 6); // store 1 device address only in this demo
		 return 1;
	 }
	 break;
	 
	 case 0x051B: // ATT_HandleValueNotification
	   {
		 p("==> ATT_HandleValueNotification\r\n");
		 
		 uint8_t data[21] = {0};
		 uint8_t len = data_len - 8;
		 
		 if (len > 20)
		   len = 20;
		   
		 memcpy(data, &buf[8], len);
		 
		 p(" -------------> Received from Biscuit peripheral: %s\r\n", data);
	   }
	   break;
	   
	 default:
	   p(" -> Not handled yet.\r\n");
	}  
}


#endif /* SCAN_BEACON */
                                
unsigned int n;
unsigned int ret;

void setup() 
{

#ifdef SCAN_BEACON
	Serial1.begin(57600);
	Serial.begin(57600);
	//while(!Serial);
	ble_hci_init();
	blemini_central_init();
#else
	Serial.begin(57600);	   // initialize serial communication
	Serial1.begin(57600);
#endif /* SCAN_BLE */

    Serial.println("1.CC3200 Wifi Init...");
    // attempt to connect to Wifi network:
    Serial.print("    Setting up Access Point named: ");
    // print the network name (SSID);
    Serial.println(ssid); 
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:

    WiFi.beginNetwork(ssid, password);
	// WiFi.beginNetwork((char *)ssid, (char *)wifipw);
  
    while (WiFi.localIP() == INADDR_NONE)
    {
        // print dots while we wait for an ip addresss
        Serial.print("    ");
        Serial.print(".");
        delay(300);
    }

    Serial.println("\n    IP Address obtained");
  
    // you're connected now, so print out the status  
    printWifiStatus();

    Serial.println("    Starting webserver on port 80");
    server.begin();                           
    Serial.println("    Webserver started!");
    Serial.println("  ");
    delay(100);
}	

void loop() {
	int i = 0;
	boolean get_info = FALSE;
	static uint8 time = 0;
	WiFiClient client = server.available();   // listen for incoming clients 

	#ifdef PRINT_DEV_NAME
	const char* str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ";
	#endif /* PRINT_DEV_NAME */

#if 1
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    char buffer[150] = {0};                 // make a buffer to hold incoming data
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
		Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (strlen(buffer) == 0) {
			#ifdef SCAN_BEACON
			received_len = 0; /* record total received data length */
			memset(buf_html, 0, HTML_BUF_SIZE);
			number_devs = 0;
		  	blemini_central_start_discovery();
			while(1) {
				if( scan_ble() == 1 ) {
					get_info = TRUE;
					break;
				} else {
					get_info = FALSE;
				}
			}
			#endif /* SCAN_BEACON */
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
			client.println("Refresh: 3");  // refresh the page automatically
            client.println();

             // the content of the HTTP response follows the header:
            client.println("<html><head><title>RedBearLab BLE Scanner - WiFi Mini and BLE Mini</title></head><body align=center>");
			client.println("<style type=\"text/css\">p{color:black;font-size:16px;text-align:left;}</style> "); 
            client.println("<h1 align=center><font color=\"red\">RedBearLab Booth #5603 (Zone 5)<br>BLE Scanner - WiFi Mini and BLE Mini</font></h1>");
			#ifdef SCAN_BEACON
                        client.print("refreshed: "); 
			client.print(time++);
                        client.println(" times"); 
			client.println("<p>");
			if ( get_info ) {
				uint16 next_data_pos = 0;
				#ifdef PRINT_DEV_NAME
				uint8*  p_ad_data = NULL;
				#endif
				
				for ( int s = 0; s < number_devs; s++) {
					//client.print(" ");client.print(buf_html[s]);
					client.print("</br>");
					client.println("-------------------------------------------------------");
					client.print("</br>");
					/* Dev Name */
					#ifdef PRINT_DEV_NAME
					uint8 len = buf_html[next_data_pos+15];
					uint8 event_type = buf_html[next_data_pos+6];
					p_ad_data = &buf_html[next_data_pos+16];
					if (*p_ad_data == 0x02) 							/* eg: 0x02: len:2, following maybe: flag(type), flag */
					{
						p_ad_data += 3; 								/* skip to next block of data */
					}
					if ((len > 3)&&(*(p_ad_data+1) == 0x09 || *(p_ad_data+1) == 0x08)) /* complete/shortened local name */
					{	
						client.print("</br>");
						client.print("Dev Name: "); 
						//p("===> Dev name \r\n");
						for (int s = 0; s < *p_ad_data; s++)
						{
							//client.print(*(p_ad_data+2+s));
							//p_http(*(p_ad_data+2+s));
							if (*(p_ad_data+2+s) >= 97 && *(p_ad_data+2+s) <= 122) {
								*(p_ad_data+2+s) -= 97;
								*(p_ad_data+2+s) += 26;
							} else if (*(p_ad_data+2+s) >= 65 && *(p_ad_data+2+s) <= 90) {
								*(p_ad_data+2+s) -= 65;
							} else {
								*(p_ad_data+2+s) = 52; /* Space */
							}
							client.print(*(str+*(p_ad_data+2+s)));
							//p("%c ",*(p_ad_data+2+s));
						}
					} else {
						client.print("</br>");
						client.print("Dev Name: &ltUnknown&gt");
					}
					client.print("</br>");
					#endif /* PRINT_DEV_NAME */
					/* Response Type */
					if (event_type == 0x00) {
						client.print("EventType: Scan Response");
					} else if (event_type == 0x04) {
						client.print("EventType: Advertisement");
					}
					client.print("</br>");
					/* Addr */
					client.print("Addr: "); 
					for (int s = 10; s > 4; s--)
					{
						if ( buf_html[next_data_pos+s+3] > 15 ) {
							client.print(buf_html[next_data_pos+s+3] ,HEX);
						} else {
							client.print('0');
							client.print(buf_html[next_data_pos+s+3] ,HEX);
						}
						if ( s != 5 )
						{
							client.print(":");
						}
					}
					client.print("</br>");
					/* RSSI */
					client.print("RSSI: -");  
					client.print(256-buf_html[next_data_pos+14]); 
					client.print("</br>");
					/* Adv Data */
					client.print("Adv Data: "); 
					for (int s = 0; s < buf_html[next_data_pos+15]; s++) /* loop time: data length */
					{
						client.print("0x");
						
						if ( buf_html[next_data_pos+s+16] > 15 ) {
							client.print(buf_html[next_data_pos+16+s],HEX);
						} else {
							client.print('0'); 							/* add '0', print 0x01-0x0f */
							client.print(buf_html[next_data_pos+16+s],HEX);
						}
						client.print(" ");
						
					}
					/* update next_data_pos */
					next_data_pos += buf_html[next_data_pos+2] + 3; 
					
				} 
			} else {
				client.println("===> No Info\n"); 
			}
			//client.print("Uniflash Standalone Flash Tool for TI Microcontrollers (MCUs) and Sitara Processors");
			client.println("</p><br>");
			#else
			client.print("Yellow LED on board <button onclick=\"location.href='/H'\">ON</button>");
            client.println(" <button onclick=\"location.href='/L'\">OFF</button>");
			#endif

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear the buffer:
            memset(buffer, 0, 150);
            i = 0;
          }
        }
        else if (c != '\r') {    // if you got nothing else but a carriage return character,
          buffer[i++] = c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (endsWith(buffer, "GET /H")) {
          digitalWrite(13, HIGH);               // GET /H turns the LED on
          
		  client.println("LED ON !");
        }
        if (endsWith(buffer, "GET /L")) {
          digitalWrite(13, LOW);                // GET /L turns the LED off
          client.println("LED OFF !");
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
#endif
}

//
//a way to check if one array ends with another array
//
boolean endsWith(char* inString, char* compString) {
  int compLength = strlen(compString);
  int strLength = strlen(inString);
  
  //compare the last "compLength" values of the inString
  int i;
  for (i = 0; i < compLength; i++) {
    char a = inString[(strLength - 1) - i];
    char b = compString[(compLength - 1) - i];
    if (a != b) {
      return false;
    }
  }
  return true;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("    SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("    IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("    signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("    To see this page in action, open a browser to http://");
  Serial.println(ip);
}

#ifdef SCAN_BEACON
uint8 scan_ble()
{
  while ( ble_event_available() ) { 
	if ( ble_event_process() == 1 ) {
		Serial.print("===> scan finish\n");
		return 1;
	}
  }
}

boolean isChar(uint8 c)
{
	if ( ('0' <= c && c <= '9') || (('a' <= c && c <= 'z')) || (('A' <= c && c <= 'Z')))
		return TRUE;
	else 
		return FALSE;
}



#endif /* SCAN_BEACON */
  
