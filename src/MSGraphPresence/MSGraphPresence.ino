#include <Arduino_JSON.h>
#include <JSON.h>
#include <JSONVar.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "params.h"

const int ledRed = 21;
const int ledGreen = 19;
const int ledBlue = 18;

const int pwmFreq = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;
const int pwmResolution = 8;

String scopes = "Presence.Read Presence.Read.All openid profile offline_access"; //Presence.Read Presence.Read.All openid profile offline_access

String presenceUrl = "https://graph.microsoft.com/beta/users/" + userId + "/presence";
String tokenUrl = "https://login.microsoftonline.com/" + tenantId + "/oauth2/v2.0/token";
String deviceAuthUrl = "https://login.microsoftonline.com/" + tenantId + "/oauth2/v2.0/devicecode";

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

String presenceReading;

String deviceCode = "";
String accessToken = "";

String refreshToken = "";

void setup()
{
	ledcSetup(redChannel, pwmFreq, pwmResolution);
    ledcSetup(greenChannel, pwmFreq, pwmResolution);
    ledcSetup(blueChannel, pwmFreq, pwmResolution);
    ledcAttachPin(ledRed, redChannel);
    ledcAttachPin(ledGreen, greenChannel);
    ledcAttachPin(ledBlue, blueChannel);

    delay(20000);
    Serial.begin(115200);

    WiFi.begin(ssid, pass);
    Serial.println("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to WiFi network with IP address: ");
    Serial.println(WiFi.localIP());

    httpAuthorizeDevice();
}

void loop()
{
    if ((millis() - lastTime) > timerDelay)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            if (accessToken != "")
            {
                presenceReading = httpGETPresence(presenceUrl.c_str(), accessToken);

                if (presenceReading == "401")
                {
                    httpRenewTokens();
                }
                else
                {
                    lightLeds(presenceReading);
                }
            }
            else if (refreshToken != "")
            {
                httpRenewTokens();
            }
            else if (deviceCode != "")
            {
                httpPopulateTokens();
            }
        }
        else
        {
            Serial.println("WiFi disconnected.");
        }

        lastTime = millis();
    }
	
}

void httpAuthorizeDevice()
{
    Serial.println("httpAuthorizeDevice(): ");
    HTTPClient client;
    client.begin(deviceAuthUrl);

    client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "client_id=" + clientId + "&scope=" + scopes;
    
    // Send HTTP POST request
    int httpResponseCode = client.POST(httpRequestData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200)
    {
        String payload = client.getString();
        // Serial.println(payload);
        
        processCodes(payload);
    }
    client.end();
}

void processCodes(String codesResponse)
{
    Serial.println("processCodes(): ");
    JSONVar myObject = JSON.parse(codesResponse);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined")
    {
        Serial.println("Parsing input failed!");
        return;
    }

    if (myObject.hasOwnProperty("user_code"))
    {
        Serial.print("User Code: ");
        Serial.println((const char *)myObject["user_code"]);
    }

    if (myObject.hasOwnProperty("device_code"))
    {
        deviceCode = (const char *)myObject["device_code"];
        // Serial.println(deviceCode);
    }

    if (myObject.hasOwnProperty("message"))
    {
        Serial.println((const char *)myObject["message"]);
    }

    return;
}

String httpGETPresence(const char *targetUrl, String accessToken)
{
    Serial.println("httpGETPresence(): ");
    HTTPClient client;

    client.begin(targetUrl);

    client.addHeader("Authorization", "Bearer " + accessToken);

    int httpResponseCode = client.GET();

    String payload = "{}";

    if (httpResponseCode > 0)
    {
        if (httpResponseCode == 200)
        {
            payload = client.getString();
        }
        else
        {
            payload = String(httpResponseCode);
        }
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }

    client.end();

    return payload;
}

void httpRenewTokens()
{
    Serial.println("httpRenewTokens(): ");
    HTTPClient client;
    client.begin(tokenUrl);

    client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "client_id=" + clientId + "&scope=" + scopes + "&grant_type=refresh_token&refresh_token=" + refreshToken; 
    // Send HTTP POST request
    int httpResponseCode = client.POST(httpRequestData);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String payload = client.getString();

    client.end();

    parseTokens(payload);
}

void parseTokens(String tokenResponse)
{
    Serial.println("parseTokens(): ");
    JSONVar myObject = JSON.parse(tokenResponse);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined")
    {
        Serial.println("Parsing input failed!");
        return;
    }

    if (myObject.hasOwnProperty("access_token"))
    {
        accessToken = (const char *)myObject["access_token"];
        // Serial.println(accessToken);
    }

    if (myObject.hasOwnProperty("refresh_token"))
    {
        refreshToken = (const char *)myObject["refresh_token"];
        // Serial.println(refreshToken);
    }
}

void lightLeds(String payload)
{
    JSONVar myObject = JSON.parse(payload);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined")
    {
        Serial.println("Parsing input failed!");
        return;
    }

    String userAvailability = "";

    if (myObject.hasOwnProperty("availability"))
    {
        userAvailability = (const char *)myObject["availability"];
    }

    if (userAvailability != "")
    {
        if (userAvailability == "Available")
        {
            ledcWrite(redChannel, 0);
            ledcWrite(greenChannel, 255);
            ledcWrite(blueChannel, 0);
        }
        else if (userAvailability == "Busy" || userAvailability == "DoNotDisturb")
        {
            ledcWrite(redChannel, 255);
            ledcWrite(greenChannel, 0);
            ledcWrite(blueChannel, 0);
        }
        else if (userAvailability == "Away" || userAvailability == "BeRightBack")
        {
            ledcWrite(redChannel, 255);
            ledcWrite(greenChannel, 255);
            ledcWrite(blueChannel, 0);
        }
        else
        {
            ledcWrite(redChannel, 0);
            ledcWrite(greenChannel, 0);
            ledcWrite(blueChannel, 0);
        }
    }
    else
    {
        ledcWrite(redChannel, 181);
        ledcWrite(greenChannel, 34);
        ledcWrite(blueChannel, 157);
    }
}

void httpPopulateTokens()
{
    Serial.println("httpPopulateTokens(): ");
    HTTPClient client;
    client.begin(tokenUrl);

    client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "grant_type=urn:ietf:params:oauth:grant-type:device_code&code=" + deviceCode + "&client_id=" + clientId;
    // Send HTTP POST request
    int httpResponseCode = client.POST(httpRequestData);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200)
    {
        String payload = client.getString();
        parseTokens(payload);
    }
    client.end();
}
