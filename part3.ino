#include <SoftwareSerial.h>  //include software serial library
SoftwareSerial client(3,4); //Assigning TX(3) & RX(4) pin to the module
#define DEBUG true //defining debug variable
String latitude,longitude,snr,raw_gsm,gsm,imei,ip; 
String payload;
String data_gps[6];
String data_snr[8];
String data_ip[3];
String data_imei[4];
String data_gsm[3];

void setup() 
{
Serial.begin(9600); //begin serial on arduino side
client.begin(9600); //begin serial on module side
delay(500);

connectGPRS(); //Setup GPRS Connection
readIP(); //Read the Ip from htttp://ipv4bot.whatismyipaddress.com 
readIMEI(); //Read Module's Imei

//Below is a sequence in case of reading IP and IMEI fails
if (ip == "") //check if GET HTTP fail to read the IP
  {
    readIP(); //Read the IP
  }
if (imei == "") //check if Module Imei failed to be read
  {
    readIMEI(); //Read Module's Imei
  }
}

void loop() {
readGSM(); //Read GSM Signal Strength
readSNR(); //Read GPS Signal Strength
readGPS(); //Read GPS location

payload = "{\"latitude\":\"" + latitude + "\",\"longitude\":\"" + longitude + "\",\"gps\":\"" + snr +"\",\"gsm\":\"" + gsm +"\",\"imei\":\""+ imei +"\",\"ip\":\"" + ip + "\"}"; //fill the payload
Serial.println(payload); //print the payload 
delay(1000);
postHTTP(); //POST the payload to the endpoint
delay(1000);
clear_buf(); //clear the payload
}

//=========================================================READ GPS==========================================================================
void readGPS()
{
  sendData("AT+CGPSPWR=1",1000,DEBUG); //AT-Command to power up GPS Power
  delay(100);
  sendData("AT+CGPSSTATUS?",1000,DEBUG); //AT-Command to check GPS Status
  delay(100);
  sendGPSData("AT+CGNSINF",1000,DEBUG); //AT-Command to display GPS Info (Including Location, Time, Etc) and parsing the GPS location
  delay(100);
}
//=========================================================PARSE GPS====================================================================
void sendGPSData(String command , const int timeout , boolean debug)
{
  client.println(command); //print the command to module
  long int time = millis(); //Set timer for timeout
  int i = 0; 

  while((time+timeout) > millis()){ //while timeout is not exceeded
    while(client.available()){
      char c = client.read(); //read modules respond
      delay(50);
      if (c != ',') { //parse modules respond with (,) as delimiter
         data_gps[i] +=c; //add modules respond to string array
         delay(50);
      } else {
        i++;  
      }
      if (i == 5) {
        delay(50);
        goto exita;
      }
    }
  }exita:
  if (debug) {
    latitude = data_gps[3]; //get parsed latitude from modules respond
    latitude.trim();
    longitude = data_gps[4]; //get parsed longitude from modules respond
    longitude.trim();
  }
}

//=========================================================READ GPS STRENGTH====================================================================
void readSNR()
{
 sendSnrData("AT+CGPSINF=16",3000,DEBUG); //AT-Command to display GPS Strength Info and parsing the GPS location ranged from 0-99dBHz, null when not tracking
 delay(3000);
}

//=========================================================PARSE GPS STRENGTH====================================================================
void sendSnrData(String command , const int timeout , boolean debug)
{
  client.println(command); //print the command to module
  long int time = millis(); //Set timer for timeout
  int j = 0;

  while((time+timeout) > millis()){ //while timeout is not exceeded
    while(client.available()){
      char d = client.read(); //read modules respond
      delay(50);
      if (d != ',') { //parse modules respond with (,) as delimiter
         data_snr[j] +=d; //add modules respond to string array
         delay(50);
      } else {
        j++;  
      }
      if (j == 8) {
        delay(50);
        goto exitb;
      }
    }
  }exitb:
  if (debug) {
    snr = data_snr[7]; //get parsed gps strength from modules respond
    snr.trim();
  }
}

//=========================================================READ GSM====================================================================
void readGSM()
{
  sendGsmData("AT+CSQ",1000,DEBUG); //AT-Command to display GSM Signal Strength Info and parsing it. Ranged and conversed to 113 - 53 dBM, https://m2msupport.net/m2msupport/atcsq-signal-quality/
  delay(100);
}

//=========================================================PARSE GSM====================================================================
void sendGsmData(String command , const int timeout , boolean debug)
{
  client.println(command); //print the command to module
  long int time = millis(); //Set timer for timeout
  int k = 0;

  while((time+timeout) > millis()){ //while timeout is not exceeded
    while(client.available()){
      char e = client.read(); //read modules respond
      delay(50);
      if (e != '\n') { //parse modules respond with (\n) as delimiter
         data_gsm[k] +=e; //add modules respond to string array
         delay(50);
      } else {
        k++;  
      }
      if (k == 3) {
        delay(50);
        goto exitc;
      }
    }
  }exitc:
  if (debug) {
    data_gsm[1].remove(0,6);
    raw_gsm = data_gsm[1]; //get parsed gps strength from modules respond
    raw_gsm.trim();
    gsm = (((raw_gsm.toInt()-1)*2) - 111); //conversing 0-99 value to (-113)-(-53) dBm, https://m2msupport.net/m2msupport/atcsq-signal-quality/
    gsm.trim();
  }
}

//=========================================================INIT GPRS====================================================================
void connectGPRS()
{ 
  sendData("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",1000,DEBUG); //AT-Command to declare Connection Type
  delay(100);

  sendData("AT+SAPBR=3,1,\"APN\",\"\"",1000,DEBUG); //AT-Command to Enable APN Setting
  delay(100);

  sendData("AT+SAPBR=1,1",1000,DEBUG); //AT-Command to enable GPRS and APN Setting
  delay(100);

  sendData("AT+SAPBR=2,1",1000,DEBUG); //AT-Command to query GPRS context
  delay(100);
}

//=========================================================READ IMEI====================================================================
void readIMEI()
{
  sendImeiData("AT+CGSN",1000,DEBUG); //AT-Command to display Imei Info and parsing it.
  delay(100);
}

//=========================================================PARSE IMEI====================================================================
void sendImeiData(String command , const int timeout , boolean debug)
{
  client.println(command); //print the command to module
  long int time = millis(); //Set timer for timeout
  int l = 0;

  while((time+timeout) > millis()){ //while timeout is not exceeded
    while(client.available()){
      char f = client.read(); //read modules respond
      delay(50);
      if (f != '\n') { //parse modules respond with (\n) as delimiter
         data_imei[l] +=f; //add modules respond to string array
         delay(50);
      } else {
        l++;  
      }
      if (l == 3) {
        delay(50);
        goto exitd;
      }
    }
  }exitd:
  if (debug) {
    imei = data_imei[1]; //get parsed Imei from modules respond
    imei.trim();
  }
}

//=========================================================READ IP====================================================================
void readIP()
{
  sendData("AT+HTTPINIT",1000,DEBUG); //AT-Command to initialize HTTP
  delay(100);
  sendData("AT+HTTPPARA=\"URL\",\"http://ipv4bot.whatismyipaddress.com\"",1000,DEBUG); //AR-Command Setting HTTP Parameter
  delay(100);
  sendData("AT+HTTPACTION=0",1000,DEBUG); //AT-Command to Enable GET Action
  delay(100); 
  sendIpData("AT+HTTPREAD=0,20",3000,DEBUG); //AT-Command to read HTTP GET respond and parsing it
  delay(100);
  sendData("AT+HTTPTERM",1000,DEBUG); //AT-Command to Terimnate HTTP Service
  delay(100);
}

//=========================================================PARSE IP====================================================================
void sendIpData(String command , const int timeout , boolean debug)
{
  client.println(command); //print the command to module
  long int time = millis(); //Set timer for timeout
  int m = 0;

  while((time+timeout) > millis()){ //while timeout is not exceeded
    while(client.available() > 0){ 
      char g = client.read(); //read modules respond
      delay(50);
      if (g != '\n')  //parse modules respond with (\n) as delimiter
      {
         data_ip[m] +=g; //add modules respond to string array
         delay(50);
      } else 
      {
        m++;  
      }
      if (m == 3) 
      {
        delay(50);
        goto exitd;
      }
    }
  }
  exitd:
  if (debug) 
  {
    ip = data_ip[2]; //get parsed gps strength from modules respond
    ip.trim();
  }
}

//=========================================================POST JSON====================================================================
void postHTTP()
{ 
  sendData("AT+HTTPINIT",1000,DEBUG); //AT-Command to initialize HTTP
  delay(100);

  sendData("AT+HTTPPARA=\"CID\",1",1000,DEBUG); //AT-Command to setting CID
  delay(100); 
  
  sendData("AT+HTTPPARA=\"URL\",\"167.71.217.129:5000\"",1000,DEBUG); //AT-Command Setting HTTP Parameter
  delay(100);
  
  sendData("AT+HTTPPARA=\"CONTENT\",\"application/json\"",1000,DEBUG); //AT-Command Setting Content Parameter
  delay(100);

  sendData("AT+HTTPDATA=" + String(payload.length()) + ",15000",1000,DEBUG); //AT-Command Setting Payload Data Parameter
  delay(100);

  client.println(payload); //Send the payload to the module
  delay(100);

  sendData("AT+HTTPACTION=1",1000,DEBUG); //AT-Command to Enable POST Action
  delay(100);

  sendData("AT+HTTPREAD",1000,DEBUG); //AT-Command to read HTTP respond and parsing it
  delay(100);

  sendData("AT+HTTPTERM",1000,DEBUG); //AT-Command to Terimnate HTTP Service
  delay(100);
  
}

//=========================================================WRITE FUNCTION2====================================================================
String sendData (String command , const int timeout ,boolean debug)
{
  String response = ""; //Clear modules respond
  delay(1000);
  client.println(command); //Print/Send Command to modules
  long int time = millis();
  int h = 0;

  while ( (time+timeout ) > millis()) //While timeout not exceeded 
  {
    while (client.available())
    {
      char b = client.read(); //Read the module's respond
      delay(50);
      response +=b; //store the respond to variable
    }
  }
  if (debug) 
    {
     Serial.print(response); //Serial print the respond
     delay(50);
    }
     return response;
}

//==================================================Clear Serial=================================
void clear_buf() //Clear Payload
{
  payload = "";
  latitude = "";
  longitude = "";
  snr = "";
  raw_gsm = "";
  gsm = "";
  data_gps[0]="";
  data_gps[1]="";
  data_gps[2]="";
  data_gps[3]="";
  data_gps[4]="";
  data_snr[5]="";
  data_snr[0]="";
  data_snr[1]="";
  data_snr[2]="";
  data_snr[3]="";
  data_snr[4]="";
  data_snr[5]="";
  data_snr[6]="";
  data_snr[7]="";
  data_snr[8]="";
  data_gsm[0]="";
  data_gsm[1]="";
  data_gsm[2]="";
  data_gsm[3]="";
  data_imei[0]="";
  data_imei[1]="";
  data_imei[2]="";
  data_imei[3]="";
  data_imei[4]="";
  data_ip[0]="";
  data_ip[1]="";
  data_ip[2]="";
  data_ip[3]="";
}
