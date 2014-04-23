

/* Include all necessary libraries. Make sure the .cpp and .h files are in the Arduino
 IDE's 'libraries' folder on your computer */
#include <Adafruit_CC3000.h> 
#include <ccspi.h>
#include <string.h>
#include <SoftPWM.h>
#include <AFMotor.h>
//#include "utility/debug.h"
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

/* DEFINE statements. Use these to define pin numbers and Wifi settings*/

// These are the interrupt and control pins
#define CC3000_IRQ   3  // MUST be an interrupt pin! Replaces all ADA… 
//with the number 3 (For pin usage)

// These can be any two pins
#define CC3000_VBAT  5
#define CC3000_CS    10

// Use hardware SPI for the remaining pins// On an UNO, SCK = 13, MISO = 12, and MOSI = 11 
//NEED TO REDEFINE PINS
Adafruit_CC3000 cc3000 = Adafruit_CC3000(CC3000_CS, CC3000_IRQ, CC3000_VBAT, SPI_CLOCK_DIVIDER); // you can change this clock speed but DI




/*** WIFI - Settings for specific Wifi network. Comment out one to use the other ***/

/* PRODUCTION */
#define WLAN_SSID       "Cisco73736"        // cannot be longer than 32 characters!
#define WLAN_PASS       "robochaser"

/* DEBUG ONLY */
//#define WLAN_SSID       "BluePuppy" 
//#define WLAN_PASS       "2sg3rhKDjU"

#define WLAN_SECURITY   WLAN_SEC_WPA2
/*
   Arduino IP Address: 192.168.1.104
 Host PC IP Address: 192.168.1.119
 */


/* MOTOR - Define pins for motors  */
#define analogturnmotor 14;
#define speedmotor 6;


/* DISTANCE SENSOR - Define pins for ultrasonic distance sensor */
#define sonicecho A0;
#define sonictrigger A1;


/* Variables for UDP communication */
//UDP Port
unsigned int localPort = 9999;
char packetBuffer[255]; //buffer to hold incoming packet   TIM - can be 3 to hold single char?
char replyBuffer[255]; // a string to send back

int motordirection = FORWARD;  //TODO - Check type of FORWARD
char sentcharacter;

int x = 0;

void setup(void)
{
  //Watch for trouble running the motor at this speed
  Serial.begin(115200);

  /* Initialize the module */
  Serial.println(F("\nInitializing the CC3000 ..."));

  //If wifi board can’t be initialized, go into infinite loop. Program will need to be restarted.  
  if (!cc3000.begin())
  {
    Serial.println(F("Unable to initialise the CC3000! Check your wiring?"));
    while(1);
  }


  displayMACAddress();

  /* Optional: Get the SSID list (not available in 'tiny' mode) */
#ifndef CC3000_TINY_DRIVER
  listSSIDResults();
#endif

  /* Delete any old connection data on the module */
  Serial.println(F("\nDeleting old connection profiles"));
  if (!cc3000.deleteProfiles()) {
    Serial.println(F("Failed!"));
    while(1);
  }

  /* Attempt to connect to an access point */
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print(F("\nAttempting to connect to ")); 
  Serial.println(ssid);

  /* NOTE: Secure connections are not available in 'Tiny' mode! */
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Secure Connect Failed!"));
    while(1);
  }

  Serial.println(F("Connected!"));

  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); 
  }  

  /* Display the IP address DNS, Gateway, etc. */
  while (! displayConnectionDetails()) {
    delay(1000);

  }

  /* Set pins for distance sensor */
  pinMode(sonicecho, OUTPUT);
  pinMode(sonictrigger, INPUT);

  //END SETUP
}



void loop() {
  /*** All code for normal operation (not setup) goes here. ***/

  /* Get current distance from sensor */
 
//DEBUG CHECK
  long duration
  int distance;     //Distance will be in cm. 
  digitalWrite(sonictrigger, LOW);  
  delayMicroseconds(2); 
  digitalWrite(sonictrigger, HIGH);

  //Delay may need to increase depending on needed frequency of signal.
  delayMicroseconds(10); 
  digitalWrite(sonictrigger, LOW);
  duration = pulseIn(sonicecho, HIGH);
  distance = (duration/2) / 29.1;
 
  Serial.print("Current distance :");
  Serial.print(distance);


  /* BEGIN PACKET RECEIVE */
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    packetBuffer = Udp.
      replyBuffer = Udp
      IPAddress remoteIp = Udp.remoteIP();
    Serial.println(Udp.remotePort());


    // read the packet into packetBuffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);


    //Get sent character. Make sure to grab single char (1 byte)
    packetBuffer[0] = sentcharacter;

    if (sentcharacter == ’c’)
    {
      //Cruise control toggle character received
      //If cruise control is enabled, disable it. if disabled, enable.
      if (cc==true)
      {
        cc = false;
      }
      else
      { 
        cc = true;
        motordirection = FORWARD; 
      }
    }
    if (sentcharacter == ’w’)
    {
      x = 255;
      motor.run(FORWARD);
      motor.setspeed(x);
    }  
    if (sentcharacter == ’s’)
    {
      x = 255;
      motor.run(REVERSE);
      motor.setspeed(x);
    }

    if (sentcharacter == ’+’)
    {
      if ((x+5)<=255)
      {
        x = x + 5;
      } 
      motor.setspeed(x);
      //increase motor speed by 5
    }
    if (sentcharacter == ’-’)
    {
      if ((x-5)>=0)
      {
        x = x - 5;
      } 
      motor.setspeed(x);
      //decrease motor speed by 5
    }

    //x = motorspeed

    /* END PACKET RECEIVE */
  }




  //Cruise control logic
  if (cc == true)
  {
    motor.run(motordirection);
    if (distance>61)
    {
      //car is far away
      //maintain speed
      motor.setspeed(x); 
    }
    else
    {
      if (distance<61)
      {
        x = x/2;
        motor.setspeed(x);
        
        //car is too close, decrease speed by half
      }
      else
      {
        if (distance==61) 
        {
          //maintain speed. leave x as is. 
        }
        if (distance>61)
        {
        //Car is over 61 cm 
           if ((x*(x/4))<=255)
           {
              x = x* (x/4);
           } 
        motor.setspeed(x);
        }
        
        
      }
    }

  }
  else
  {
     
  }

  //END LOOP
}


/***** CC3000 Specific Code. DO NOT MODIFY.   *****/

/**************************************************************************/
/*!
 @brief  Tries to read the 6-byte MAC address of the CC3000 module
 */
/**************************************************************************/
void displayMACAddress(void)
{
  uint8_t macAddress[6];

  if(!cc3000.getMacAddress(macAddress))
  {
    Serial.println(F("Unable to retrieve MAC Address!\r\n"));
  }
  else
  {
    Serial.print(F("MAC Address : "));
    cc3000.printHex((byte*)&macAddress, 6);
  }
}


/**************************************************************************/
/*!
 @brief  Tries to read the IP address and other connection details
 */
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); 
    cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); 
    cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); 
    cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); 
    cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); 
    cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

/**************************************************************************/
/*!
 @brief  Begins an SSID scan and prints out all the visible networks
 */
/**************************************************************************/

void listSSIDResults(void)
{
  uint8_t valid, rssi, sec, index;
  char ssidname[33]; 

  index = cc3000.startSSIDscan();

  Serial.print(F("Networks found: ")); 
  Serial.println(index);
  Serial.println(F("================================================"));

  while (index) {
    index--;

    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);

    Serial.print(F("SSID Name    : ")); 
    Serial.print(ssidname);
    Serial.println();
    Serial.print(F("RSSI         : "));
    Serial.println(rssi);
    Serial.print(F("Security Mode: "));
    Serial.println(sec);
    Serial.println();
  }
  Serial.println(F("================================================"));

  cc3000.stopSSIDscan();
}



