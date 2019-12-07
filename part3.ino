#include <SoftwareSerial.h>
SoftwareSerial client(3,4);
#define DEBUG true
String latitude,longitude,snr,raw_gsm,gsm,imei,ip;
String payload;
String data_gps[6];
String data_snr[8];
String data_ip[3];
String data_imei[4];
String data_gsm[3];

void setup() 
{
Serial.begin(9600);
client.begin(9600);
delay(500);

connectGPRS();
readIP();
readIMEI();
}

void loop() {
readGSM();
readSNR();
readGPS();
payload = "{\"latitude\":\"" + latitude + "\",\"longitude\":\"" + longitude + "\",\"gps\":\"" + snr +"\",\"gsm\":\"" + gsm +"\",\"imei\":\""+ imei +"\",\"ip\":\"" + ip + "\"}";
Serial.println(payload); 
delay(1000);
postHTTP();
delay(1000);
clear_buf();
}

//=========================================================READ GPS==========================================================================
void readGPS()
{
  sendData("AT+CGPSPWR=1",1000,DEBUG);
  delay(1000);
  sendData("AT+CGPSSTATUS?",1000,DEBUG);
  delay(1000);
  sendGPSData("AT+CGNSINF",1000,DEBUG);
  delay(1000);
}
//=========================================================PARSE GPS====================================================================
void sendGPSData(String command , const int timeout , boolean debug)
{
  client.println(command);
  long int time = millis();
  int i = 0;

  while((time+timeout) > millis()){
    while(client.available()){
      char c = client.read();
      delay(100);
      if (c != ',') {
         data_gps[i] +=c;
         delay(100);
      } else {
        i++;  
      }
      if (i == 5) {
        delay(100);
        goto exita;
      }
    }
  }exita:
  if (debug) {
    latitude = data_gps[3];
    latitude.trim();
    longitude = data_gps[4];
    longitude.trim();
  }
}

//=========================================================READ GPS STRENGTH====================================================================
void readSNR()
{
 sendSnrData("AT+CGPSINF=16",1000,DEBUG);
 delay(3000);
}

//=========================================================PARSE GPS STRENGTH====================================================================
void sendSnrData(String command , const int timeout , boolean debug)
{
  client.println(command);
  long int time = millis();
  int j = 0;

  while((time+timeout) > millis()){
    while(client.available()){
      char d = client.read();
      delay(100);
      if (d != ',') {
         data_snr[j] +=d;
         delay(100);
      } else {
        j++;  
      }
      if (j == 8) {
        delay(100);
        goto exitb;
      }
    }
  }exitb:
  if (debug) {
    snr = data_snr[7];
    snr.trim();
  }
}

//=========================================================READ GSM====================================================================
void readGSM()
{
  sendGsmData("AT+CSQ",1000,DEBUG);
  delay(1000);
}

//=========================================================PARSE GSM====================================================================
void sendGsmData(String command , const int timeout , boolean debug)
{
  client.println(command);
  long int time = millis();
  int k = 0;

  while((time+timeout) > millis()){
    while(client.available()){
      char e = client.read();
      delay(100);
      if (e != '\n') {
         data_gsm[k] +=e;
         delay(100);
      } else {
        k++;  
      }
      if (k == 3) {
        delay(100);
        goto exitc;
      }
    }
  }exitc:
  if (debug) {
    data_gsm[1].remove(0,6);
    raw_gsm = data_gsm[1];
    raw_gsm.trim();
    gsm = (((raw_gsm.toInt()-1)*2) - 111);
    gsm.trim();
  }
}

//=========================================================INIT GPRS====================================================================
void connectGPRS()
{ 
  sendData("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",1000,DEBUG);
  delay(1000);

  sendData("AT+SAPBR=3,1,\"APN\",\"\"",1000,DEBUG);//APN
  delay(1000);

  sendData("AT+SAPBR=1,1",1000,DEBUG);
  delay(1000);

  sendData("AT+SAPBR=2,1",1000,DEBUG);
  delay(1000);
}

//=========================================================READ IMEI====================================================================
void readIMEI()
{
  sendImeiData("AT+CGSN",1000,DEBUG);
  delay(1000);
}

//=========================================================PARSE IMEI====================================================================
void sendImeiData(String command , const int timeout , boolean debug)
{
  client.println(command);
  long int time = millis();
  int l = 0;

  while((time+timeout) > millis()){
    while(client.available()){
      char f = client.read();
      delay(100);
      if (f != '\n') {
         data_imei[l] +=f;
         delay(100);
      } else {
        l++;  
      }
      if (l == 3) {
        delay(100);
        goto exitd;
      }
    }
  }exitd:
  if (debug) {
    imei = data_imei[1];
    imei.trim();
  }
}

//=========================================================READ IP====================================================================
void readIP()
{
  sendData("AT+HTTPINIT",1000,DEBUG);
  delay(1000);
  sendData("AT+HTTPPARA=\"URL\",\"http://ipv4bot.whatismyipaddress.com\"",1000,DEBUG);
  delay(1000);
  sendData("AT+HTTPACTION=0",1000,DEBUG);
  delay(1000);
  sendData("AT+HTTPREAD=0,20",1000,DEBUG);
  delay(1000);
  sendIpData("AT+HTTPREAD=0,20",3000,DEBUG);
  delay(1000);
  sendData("AT+HTTPTERM",1000,DEBUG);
  delay(1000);
}

//=========================================================PARSE IP====================================================================
void sendIpData(String command , const int timeout , boolean debug)
{
  client.println(command);
  long int time = millis();
  int m = 0;

  while((time+timeout) > millis()){
    while(client.available() > 0){
      char g = client.read();
      delay(100);
      if (g != '\n') 
      {
         data_ip[m] +=g;
         delay(100);
      } else 
      {
        m++;  
      }
      if (m == 3) 
      {
        delay(100);
        goto exitd;
      }
    }
  }
  exitd:
  if (debug) 
  {
    ip = data_ip[2];
    ip.trim();
  }
}

//=========================================================POST JSON====================================================================
void postHTTP()
{ 
  sendData("AT+HTTPINIT",1000,DEBUG);
  delay(1000);

  sendData("AT+HTTPPARA=\"CID\",1",1000,DEBUG);
  delay(1000);

  //client.println("AT+HTTPPARA=\"URL\",\"http://webhook.site/bad936b2-0fa1-48f2-ac12-74c58a8d91b5\"");//Public server address
  //delay(1000);
  //ShowSerialData();  
 
  
  sendData("AT+HTTPPARA=\"URL\",\"167.71.217.129:5000\"",1000,DEBUG);//Public server address
  delay(1000);
  
  sendData("AT+HTTPPARA=\"CONTENT\",\"application/json\"",1000,DEBUG);
  delay(1000);

  sendData("AT+HTTPDATA=" + String(payload.length()) + ",15000",1000,DEBUG);
  delay(10000);

  client.println(payload);
  delay(1000);

  sendData("AT+HTTPACTION=1",1000,DEBUG);
  delay(1000);

  sendData("AT+HTTPREAD",1000,DEBUG);
  delay(1000);

  sendData("AT+HTTPTERM",1000,DEBUG);
  delay(1000);
  
  clear_buf();
}

//=========================================================WRITE FUNCTION1====================================================================
void ShowSerialData()
{
  while(client.available()!=0)
  {
  Serial.write(client.read());
  delay(100);
  }
}

//=========================================================WRITE FUNCTION2====================================================================
String sendData (String command , const int timeout ,boolean debug)
{
  String response = "";
  delay(1000);
  client.println(command);
  long int time = millis();
  int h = 0;

  while ( (time+timeout ) > millis())
  {
    while (client.available())
    {
      char b = client.read();
      delay(100);
      response +=b;
    }
  }
  if (debug) 
    {
     Serial.print(response);
     delay(100);
    }
     return response;
}

//==================================================Clear Serial=================================
void clear_buf()
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
