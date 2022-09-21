#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <cstring>
#include <iostream>

String company{""};
String service{""};

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

// server
String ssid;
String password;
const char *server = "https://noq.ddns.net/includes/qrquing.inc.php";
HTTPClient http;
bool connected = false;

/*
  Tries to find info at a very specific location of the string
*/
String getInfo(std::string &target, String text, std::string data, int offset = 0)
{
  String result{""};

  if (target.find(data) != std::string::npos)
  {
    int pos = target.find(data) + offset;
    for (int i = pos; i < text.length(); i++)
    {
      if (text[i] == ';')
        break;
      result += text[i];
    }
  }
  else
  {
    return "error";
  }

  return result;
}

int onQrCodeTask()
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("Found QRCode");
      if (qrCodeData.valid)
      {
        Serial.println("Valid QR Code");
        Serial.println("Payload is :");
        Serial.println((const char *)qrCodeData.payload);

        String qrData = (const char *)qrCodeData.payload;
        std::string qrDataS = qrData.c_str();

        // First see if you got the wifi data
        if (!connected)
        {
          String s_ssid = getInfo(qrDataS, qrData, "S:", 2);
          String s_password = getInfo(qrDataS, qrData, "P:", 2);

          ssid = s_ssid;
          password = s_password;
        }

        // If wifi data is valid connect to the wifi
        if (ssid != "" && !connected)
        {
          WiFi.begin(ssid.c_str(), password.c_str());
          Serial.print("Connecting..");
          while (WiFi.status() != WL_CONNECTED)
          {
            delay(250);
            Serial.print(".");
          }
          connected = true;
          return 1;
        }

        String cn{""};
        String s{""};
        String u{""};

        cn = getInfo(qrDataS, qrData, "cn=", 3);
        s = getInfo(qrDataS, qrData, "s=", 2);
        u = getInfo(qrDataS, qrData, "u=", 2);

        // First select a company and a service then you can start queuing up
        if (company == "" && service == "" && cn != "error" && s != "error")
        {
          company = cn;
          service = s;

          return 2;
        }
        else if (company != "" && service != "" && u != "error")
        {
          String body = "companies=" + company + "&services=" + service + "&uId=" + u;
          int length = body.length();

          // Send data to the server
          http.begin(server);
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          http.addHeader("Content-Length", (String)length);
          int result = http.POST(body);
          Serial.println("Result " + (String)result);
          Serial.println("Message : " + http.getString());

          return 3;
        }
      }
      else
      {
        Serial.print("Invalid: ");
        Serial.println((const char *)qrCodeData.payload);
      }
    }
  }
}

void setup()
{
  pinMode(4, OUTPUT);
  Serial.begin(115200);
  Serial.println();

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");
  Serial.println("Get wifi connection...");
}

void loop()
{
  int result = onQrCodeTask();

  switch (result)
  {
  case 1:
    Serial.println("WIFI CONNECTED!");
    digitalWrite(4, HIGH);
    delay(2000);
    digitalWrite(4, LOW);
    break;
  case 2:
    Serial.println("COMPANY AND SERVICE SELECTED!");
    digitalWrite(4, HIGH);
    delay(2000);
    digitalWrite(4, LOW);
    break;
  case 3:
    Serial.println("USER QUEUE STARTED!");
    digitalWrite(4, HIGH);
    delay(2000);
    digitalWrite(4, LOW);
    break;

  default:
    break;
  };

  delay(100);
}