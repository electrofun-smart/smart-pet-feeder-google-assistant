// Include the Arduino Stepper.h library:
#include <MQTTClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <WiFiClientSecure.h>
//#include <AccelStepper.h>
#include <Stepper.h>

const int stepsPerRevolution = 2038;

// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 5, 19, 18, 21); // ESP32 pins used

String device_pet_feeder = "xxx"; // your switch device id created for google home (check my videos to see how to create a new device)

char ssid[] = "";   //  your network SSID (name)
char pass[] = ""; // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;
// MQTT info
const char *thehostname = "postman.cloudmqtt.com"; // MQTT broker
const char *user = "xxx";
const char *user_password = "yyy";
const char *id = "ESP32-Pet-Feeder";

boolean onOff = false;
#define FEED_INTERVAL 3000 // 3 seconds

uint64_t timestamp = 0;
int counter = 0;

const char *rootCACertificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIF6TCCA9GgAwIBAgIQBeTcO5Q4qzuFl8umoZhQ4zANBgkqhkiG9w0BAQwFADCB\n"
    "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
    "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
    "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQw\n"
    "OTEyMDAwMDAwWhcNMjQwOTExMjM1OTU5WjBfMQswCQYDVQQGEwJGUjEOMAwGA1UE\n"
    "CBMFUGFyaXMxDjAMBgNVBAcTBVBhcmlzMQ4wDAYDVQQKEwVHYW5kaTEgMB4GA1UE\n"
    "AxMXR2FuZGkgU3RhbmRhcmQgU1NMIENBIDIwggEiMA0GCSqGSIb3DQEBAQUAA4IB\n"
    "DwAwggEKAoIBAQCUBC2meZV0/9UAPPWu2JSxKXzAjwsLibmCg5duNyj1ohrP0pIL\n"
    "m6jTh5RzhBCf3DXLwi2SrCG5yzv8QMHBgyHwv/j2nPqcghDA0I5O5Q1MsJFckLSk\n"
    "QFEW2uSEEi0FXKEfFxkkUap66uEHG4aNAXLy59SDIzme4OFMH2sio7QQZrDtgpbX\n"
    "bmq08j+1QvzdirWrui0dOnWbMdw+naxb00ENbLAb9Tr1eeohovj0M1JLJC0epJmx\n"
    "bUi8uBL+cnB89/sCdfSN3tbawKAyGlLfOGsuRTg/PwSWAP2h9KK71RfWJ3wbWFmV\n"
    "XooS/ZyrgT5SKEhRhWvzkbKGPym1bgNi7tYFAgMBAAGjggF1MIIBcTAfBgNVHSME\n"
    "GDAWgBRTeb9aqitKz1SA4dibwJ3ysgNmyzAdBgNVHQ4EFgQUs5Cn2MmvTs1hPJ98\n"
    "rV1/Qf1pMOowDgYDVR0PAQH/BAQDAgGGMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n"
    "VR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMCIGA1UdIAQbMBkwDQYLKwYBBAGy\n"
    "MQECAhowCAYGZ4EMAQIBMFAGA1UdHwRJMEcwRaBDoEGGP2h0dHA6Ly9jcmwudXNl\n"
    "cnRydXN0LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNy\n"
    "bDB2BggrBgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6Ly9jcnQudXNlcnRy\n"
    "dXN0LmNvbS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAlBggrBgEFBQcwAYYZ\n"
    "aHR0cDovL29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0BAQwFAAOCAgEAWGf9\n"
    "crJq13xhlhl+2UNG0SZ9yFP6ZrBrLafTqlb3OojQO3LJUP33WbKqaPWMcwO7lWUX\n"
    "zi8c3ZgTopHJ7qFAbjyY1lzzsiI8Le4bpOHeICQW8owRc5E69vrOJAKHypPstLbI\n"
    "FhfFcvwnQPYT/pOmnVHvPCvYd1ebjGU6NSU2t7WKY28HJ5OxYI2A25bUeo8tqxyI\n"
    "yW5+1mUfr13KFj8oRtygNeX56eXVlogMT8a3d2dIhCe2H7Bo26y/d7CQuKLJHDJd\n"
    "ArolQ4FCR7vY4Y8MDEZf7kYzawMUgtN+zY+vkNaOJH1AQrRqahfGlZfh8jjNp+20\n"
    "J0CT33KpuMZmYzc4ZCIwojvxuch7yPspOqsactIGEk72gtQjbz7Dk+XYtsDe3CMW\n"
    "1hMwt6CaDixVBgBwAc/qOR2A24j3pSC4W/0xJmmPLQphgzpHphNULB7j7UTKvGof\n"
    "KA5R2d4On3XNDgOVyvnFqSot/kGkoUeuDcL5OWYzSlvhhChZbH2UF3bkRYKtcCD9\n"
    "0m9jqNf6oDP6N8v3smWe2lBvP+Sn845dWDKXcCMu5/3EFZucJ48y7RetWIExKREa\n"
    "m9T8bJUox04FB6b9HbwZ4ui3uRGKLXASUoWNjDNKD/yZkuBjcNqllEdjB+dYxzFf\n"
    "BT02Vf6Dsuimrdfp5gJ0iHRc2jTbkNJtUQoj1iM=\n"
    "-----END CERTIFICATE-----\n";

void setup()
{

    Serial.begin(115200);
    WiFi.begin(ssid, pass);

    client.begin(thehostname, 16157, net);
    client.onMessage(messageReceived);
    connect();
    delay(1000);

    timestamp = millis() + FEED_INTERVAL + 1;
    onOff = false;
    counter = 0;
    delay(500);
    sendHttps(device_pet_feeder);
}

void connect()
{
    Serial.print("checking wifi…");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
    }
    Serial.print("\nconnecting…");
    while (!client.connect(id, user, user_password))
    {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nconnected!");
    client.subscribe(device_pet_feeder + "-client");
    delay(1000);
}

void messageReceived(String &topic, String &payload)
{
    Serial.println("incoming: " + topic + " - " + payload);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(payload);
    String deviceOn = json["on"];

    if (topic == (device_pet_feeder + "-client"))
    {
        if (deviceOn == "true")
        {
            onOff = true;
            counter = 0;
            timestamp = millis();
        }
    }
}

void loop()
{
    client.loop();
    if (!client.connected())
    {
        connect();
    }
    else
    {
        uint64_t now = millis();
        if ((now - timestamp) < FEED_INTERVAL)
        {
            // feeding....
            if (onOff)
            {
                Serial.println("steps motor on...");
                myStepper.setSpeed(6);
                myStepper.step(400);
                delay(100);
                counter++;
                Serial.println("counter = ");
                Serial.print(counter, DEC);
                Serial.println(" ");
                if (counter < 9)
                {
                    timestamp = millis();
                }
            }
        }
        else
        {
            if (onOff && counter > 7)
            {
                onOff = false;
                Serial.println("stop motor");
                sendHttps(device_pet_feeder);
            }
        }
    }
}

// Update Google Home Switch to OFF when timer of Step Motor expire

void sendHttps(String deviceId)
{

    WiFiClientSecure *client = new WiFiClientSecure;
    if (client)
    {
        client->setCACert(rootCACertificate);

        HTTPClient https;

        https.begin("https://xxx.appspot.com/smarthome/update"); // your url for Java application that interfaces with Google Smart Home
        https.addHeader("Content-Type", "application/json");

        Serial.print("[HTTPS] POST...\n");
        // start connection and send HTTP header

        StaticJsonBuffer<256> jsonBuffer;
        JsonObject &res = jsonBuffer.createObject();

        res["userId"] = "1234";
        res["deviceId"] = deviceId;
        res["errorCode"] = "";

        JsonObject &states = res.createNestedObject("states");
        states["online"] = true;
        states["on"] = false;

        String msg;
        res.prettyPrintTo(Serial);
        res.printTo(msg);

        int httpCode = https.POST(msg);

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = https.getString();
                Serial.println(payload);
            }
        }
        else
        {
            Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
    }
    else
    {
        Serial.printf("[HTTPS] Unable to connect\n");
    }
}
