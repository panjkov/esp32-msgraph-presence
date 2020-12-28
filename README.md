# esp32-msgraph-presence
Demonstrating how to read presence data with Microsoft Graph using ESP32

The branch ["GetPresenceFromMeEndpoint"](../../tree/GetPresenceFromMeEndpoint) can obtain only presence for the user that is currently logged in. 

## Configuration
1. Register an application in Azure Active Directory with necessary permissions to access Presence endpoint on Microsoft Graph (Presence.Read, Presence.Read.All). More info https://docs.microsoft.com/en-us/graph/api/presence-get?view=graph-rest-1.0&tabs=http
2. Rename file [params.h.txt](params.h.txt) to params.h and update values for necessary parameters:
* WLAN credentials, 
* Tenant ID from AAD, 
* Client ID for the app registered in previous step
* User ID of the user that we want to read presence for. Easiest way is to read /me endpoint from Microsoft Graph in Graph Explorer aka.ms/ge
3. Build a circuit as explained on https://www.dragan-panjkov.com/reading-microsoft-teams-presence-information-with-microsoft-graph-and-esp32
4. Deploy code using Arduino IDE, Arduino CLI, Visual Studio Code with Arduino extension 

## Read more
Read more about this solution on https://www.dragan-panjkov.com/reading-microsoft-teams-presence-information-with-microsoft-graph-and-esp32

## Author
Dragan Panjkov, www.dragan-panjkov.com

## License
This repo is licensed with MIT License: [LICENSE](LICENSE)
