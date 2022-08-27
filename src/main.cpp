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
const int port = 443;
bool connected = false;

String getInfo(std::string &target, String text, std::string data, int offset = 0)
{
  String result{""};

  if (target.find(data) != std::string::npos)
  {
    int pos = target.find(data) + offset;
    for (int i = pos; i < text.length(); i++)
    {
      if (text[i] == ';')
      {
        break;
      }
      result += text[i];
    }
  }
  else
  {
    return "error";
  }

  return result;
}

void onQrCodeTask()
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

        if (!connected)
        {
          Serial.println(qrData);
          String s_ssid = getInfo(qrDataS, qrData, "S:", 2);
          String s_password = getInfo(qrDataS, qrData, "P:", 2);
          Serial.println(s_ssid.length());
          Serial.println(s_password.length());
          ssid = s_ssid;
          password = s_password;
        }

        if (ssid != "" && !connected)
        {
          Serial.println("This is what I get: ");
          Serial.println(ssid);
          Serial.println(password);
          WiFi.begin(ssid.c_str(), password.c_str());
          Serial.print("Connecting..");
          while (WiFi.status() != WL_CONNECTED)
          {
            delay(250);
            Serial.print(".");
          }
          Serial.print("Connected!\n");
          connected = true;
        }

        String cn{""};
        String s{""};
        String u{""};

        cn = getInfo(qrDataS, qrData, "cn=", 3);
        s = getInfo(qrDataS, qrData, "s=", 2);
        u = getInfo(qrDataS, qrData, "u=", 2);

        if (company == "" && service == "" && cn != "error" && s != "error")
        {
          company = cn;
          service = s;

          Serial.print("Payload: ");
          Serial.println(qrData);
          Serial.print("Cn is: ");
          Serial.println(company);
          Serial.print("S is: ");
          Serial.println(service);
        }
        else if (company != "" && service != "" && u != "error")
        {
          Serial.print("Payload: ");
          Serial.println(qrData);
          Serial.print("Cn is: ");
          Serial.println(company);
          Serial.print("S is: ");
          Serial.println(service);
          Serial.print("U is: ");
          Serial.println(u);
          String body = "companies=" + company + "&services=" + service + "&uId=" + u;
          int length = body.length();
          http.begin(server);
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          http.addHeader("Content-Length", (String)length);
          int result = http.POST(body);
          Serial.println("Result " + (String)result);
          Serial.println("Message : " + http.getString());
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
  Serial.begin(115200);
  Serial.println();

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");
  Serial.println("Get wifi connection...");

/*
  ssid = "Innbox-internet-e5a130";
  password = "INNBOX3138267000098";
  WiFi.mode(WIFI_STA); // The WiFi is in station mode. The    other is the softAP mode
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected to: ");
  Serial.println(ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
*/
  /*
  http.begin(server);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Content-Length", "100");
  int result = http.POST("companies=y&services=one&uId=145");
  Serial.println("Resultt " + (String)result);
  Serial.println("Poruka: " + http.getString());
  */
}

void loop()
{
  /*
  String c{"y"};
  String s{"s"};
  String u{"145"};
  String body = "companies=" + c + "&services=" + s + "&uId=" + u;
  int length = body.length();

  http.begin(server);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Content-Length", (String)length);
  int result = http.POST(body);
  Serial.println("Resultt " + (String)result);
  Serial.println("Poruka: " + http.getString());
  */
  onQrCodeTask();

  delay(100);
}