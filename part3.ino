#include <SoftwareSerial.h>
SoftwareSerial myGsm(3,4);
#define DEBUG true
String data_gps[5];
String data_snr[13];
String data_gsm[3];
String data_ip[3];
String data_imei[3];
String longitude,latitude,ip,snr,gsm,imei;
String one,im1,ip2;

void setup()
{
myGsm.begin(9600);
Serial.begin(9600);
delay(500);

sendData("AT+CGPSPWR=1",1000,DEBUG); //AT-Command to Power Up GPS
delay(1000);
sendData("AT+CGPSSTATUS?",1000,DEBUG); //AT-Command to check on GPS Status
delay(1000);
sendData("AT+CGSN",1000,DEBUG); //AT-Command to check on module's IMEI
delay(1000);
sendImeiData("AT+CGSN",1000,DEBUG); //Parsed AT-Command for module's IMEI
delay(1000);

sendData("AT+CGATT=1",200,DEBUG); //AT-Command to enable GPRS Attribute
delay(1000);
sendData("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",1000,DEBUG); //AT-Command to declare Connection Type
delay(1000);
sendData("AT+SAPBR=3,1,\"APN\",\"\"",1000,DEBUG); //AT-Command to Enable APN Setting
delay(1000);
sendData("AT+SAPBR=1,1",1000,DEBUG); //AT-Command to enable GPRS and APN Setting
delay(1000);
sendData("AT+HTTPINIT",1000,DEBUG); //AT-Command Initializing HTTP
delay(1000);
sendData("AT+HTTPPARA=\"URL\",\"http://ipv4bot.whatismyipaddress.com\"",1000,DEBUG); //Setting HTTP Parameter
delay(1000);
sendData("AT+HTTPACTION=0",1000,DEBUG); //Enabling GET Action
delay(1000);
sendData("AT+HTTPREAD=0,20",3000,DEBUG); //Read HTTP Data
delay(1000);
sendIpData("AT+HTTPREAD=0,20",3000,DEBUG); //Parsed AT-Command for the Data (IP Address)
delay(1000);
sendData("AT+HTTPTERM",1000,DEBUG); //Terimnate HTTP Service
delay(1000);
}

void loop()
{
sendData("AT+CSQ",1000,DEBUG); //AT Command to check GSM Signal Quality
delay(1000);
sendGsmData("AT+CSQ",1000,DEBUG); //Parsed AT-Command for GSM Signal Quality
delay(1000);

sendData("AT+CGNSINF",1000,DEBUG); //AT Command for GPS Info that include Location
delay(1000);
sendTabData("AT+CGNSINF",1000,DEBUG); //Parsed AT-Command for GPS Location
delay(1000);

sendData("AT+CGPSINF=16",1000,DEBUG); //AT-Command for GPS info that include GPS Signal To Noise Ratio
delay(1000);
sendSnrData("AT+CGPSINF=16",1000,DEBUG); //Parsed AT-Command for GPS SNR
delay(1000);

Serial.println("");
delay(1000);

Serial.println("Latitude:"+latitude);
delay(1000);
Serial.println("Longitude:"+longitude);
delay(1000);
Serial.println("Gps:"+snr);
delay(1000);
Serial.println("Gsm:"+gsm);
delay(1000);
Serial.println("imei:"+im1);
delay(1000);
Serial.println("Ip:"+ip2);
delay(2000);

Serial.println("");
delay(1000);
/*
payload = "{\"latitude\" : latitude, \"longitude\" : longitude, \"Gps\" : snr, \"Gsm\" : gsm + \"Imei\" : im1 + \"Ip\" : ip2}";
delay(1000);
payload = ("Latitude:" + latitude +   ',' + "Longitude:" + longitude + ',' + "Gps:" + snr + ',' + "Gsm:" + gsm + ',' + "Imei:" + im1 + ',' + "Ip:" + ip2);
delay(1000);
Serial.println("Payload:"+payload);
String payload[100] = "{\"latitude\":\"" + latitude + "\",\"longitude\":\"" + longitude +"\",\"Gps\":\"" + snr +"\",\"Gsm\":\"" + gsm +"\",\"Imei\":\"" + im1 +"\",\"Ip\":\"" + ip2 + "\"}";
Serial.println(payload[100]);
String payload = "{\"latitude\":\"" + latitude + "\",\"longitude\":\"" + longitude +"\",\"Gps\":\"" + snr +"\",\"Gsm\":\"" + gsm +"\",\"Imei\":\"" + im1 +"\",\"Ip\":\"" + ip2 + "\"}";
*/
delay(5000);
clear_buf();

}

//==================================PRINT AT COMMAND========================================

String sendData (String command , const int timeout ,boolean debug){
  String response = "";
  myGsm.println(command);
  long int time = millis();
  int i = 0;

  while ( (time+timeout ) > millis()){
    while (myGsm.available()){
      char c = myGsm.read();
      response +=c;
    }
  }
  if (debug) {
     Serial.print(response);
     }
     return response;
}

//================================PARSING GPS POS===============================================

void sendTabData(String command , const int timeout , boolean debug)
{
  myGsm.println(command);
  long int time = millis();
  int i = 0;

  while((time+timeout) > millis()){
    while(myGsm.available()){
      char c = myGsm.read();
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
    longitude = data_gps[4];
  }
}

//================================PARSING GPS STRENGTH===============================================

void sendSnrData(String command , const int timeout , boolean debug)
{
  myGsm.println(command);
  long int time = millis();
  int j = 0;

  while((time+timeout) > millis()){
    while(myGsm.available()){
      char d = myGsm.read();
      if (d != ',') {
         data_snr[j] +=d;
         delay(100);
      } else {
        j++;  
      }
      if (j == 13) {
        delay(100);
        goto exitb;
      }
    }
  }exitb:
  if (debug) {
    snr = data_snr[12];
  }
}

//================================PARSING GSM STRENGTH===============================================

void sendGsmData(String command , const int timeout , boolean debug)
{
  myGsm.println(command);
  long int time = millis();
  int k = 0;

  while((time+timeout) > millis()){
    while(myGsm.available()){
      char e = myGsm.read();
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
    one = data_gsm[1];
    gsm = (((one.toInt()-1)*2) - 111);  //Value from AT+CSQ 1 = -111 dBm with +2 dBm each incerement on value
  }
}

//================================PARSING IMEI===============================================

void sendImeiData(String command , const int timeout , boolean debug)
{
  myGsm.println(command);
  long int time = millis();
  int l = 0;

  while((time+timeout) > millis()){
    while(myGsm.available()){
      char f = myGsm.read();
      if (f != '\n') {
         data_imei[l] +=f;
         delay(100);
      } else {
        l++;  
      }
      if (l == 2) {
        delay(100);
        goto exitd;
      }
    }
  }exitd:
  if (debug) {
    im1 = data_imei[1];
  }
}

//================================PARSING IP===============================================

void sendIpData(String command , const int timeout , boolean debug)
{
  myGsm.println(command);
  long int time = millis();
  int m = 0;

  while((time+timeout) > millis()){
    while(myGsm.available()){
      char g = myGsm.read();
      if (g != '\n') {
         data_ip[m] +=g;
         delay(100);
      } else {
        m++;  
      }
      if (m == 3) {
        delay(100);
        goto exitd;
      }
    }
  }exitd:
  if (debug) {
    ip2 = data_ip[2];
  }
}

void clear_buf()
{
  data_gps[3].remove(0);
  data_gps[4].remove(0);
  data_snr[12].remove(0);
  data_gsm[1].remove(0);
  data_imei[1].remove(0);
  data_ip[2].remove(0);
}
