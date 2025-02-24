#include <iostream>
#include <string>
#include <Windows.h>
#include <wininet.h>
#include <iphlpapi.h>
#include <sstream>

#pragma comment(lib, "iphlpapi.lib") // Link the required library

using namespace std;

// Function to set console text color
void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

string GetHWID() {
    DWORD serialNum = 0;
    if (GetVolumeInformationA("C:\\", nullptr, 0, &serialNum, nullptr, nullptr, nullptr, 0)) {
        stringstream hwidStream;
        hwidStream << hex << serialNum;
        return hwidStream.str();
    }
    return "Unknown";
}

bool SendHttpRequest(const string& url, const string& postData, string& response) {
    HINTERNET hInternet = InternetOpen(L"HTTPClient", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet) {
        cerr << "[Error] Failed to open internet connection. Error code: " << GetLastError() << endl;
        return false;
    }

    HINTERNET hConnect = InternetConnectA(hInternet, "www.ghostwaretools.com", INTERNET_DEFAULT_HTTPS_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        cerr << "[Error] Failed to connect to server. Error code: " << GetLastError() << endl;
        InternetCloseHandle(hInternet);
        return false;
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/Admin/server.php", nullptr, nullptr, nullptr, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        cerr << "[Error] Failed to open request. Error code: " << GetLastError() << endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    const char* headers = "Content-Type: application/x-www-form-urlencoded";
    if (!HttpSendRequestA(hRequest, headers, strlen(headers), (LPVOID)postData.c_str(), postData.length())) {
        cerr << "[Error] Failed to send request. Error code: " << GetLastError() << endl;
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    response.clear();
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        response.append(buffer, bytesRead);
    }

    if (response.empty()) {
        cerr << "[Error] No response received from server." << endl;
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return true;
}

// Function to process the server response
void ProcessResponse(const string& responseBody) {
    SetColor(10); // Green
    cout << "\nResponse:\n" << responseBody << endl;

    if (responseBody.find("\"status\":\"success\"") != string::npos) {
        SetColor(10);
        cout << " [*] Account verified successfully!" << endl;
    }
    else {
        SetColor(12);
        cout << " [!] Error: " << responseBody << endl;
    }
}

// Function to get the PC Name
string GetPCName() {
    char computerName[MAX_PATH];
    DWORD size = sizeof(computerName);
    return (GetComputerNameA(computerName, &size)) ? string(computerName) : "Unknown";
}

// Function to get the IP Address
string GetPCIP() {
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD dwSize = sizeof(adapterInfo);
    if (GetAdaptersInfo(adapterInfo, &dwSize) == ERROR_SUCCESS) {
        return string(adapterInfo[0].IpAddressList.IpAddress.String);
    }
    return "127.0.0.1";
}

// Function to handle user input
void HandleUserInput() {
    string username, password;

    SetColor(15);
    cout << "=========================\n";
    cout << " Login to Hyperion Panel\n";
    cout << "=========================\n\n";
    cout << "Enter Username: ";
    getline(cin, username);
    cout << "Enter Password: ";
    getline(cin, password);

    if (username.empty() || password.empty()) {
        SetColor(12);
        cout << "Username or Password cannot be empty. Please try again.\n";
        return;
    }

    string pcName = GetPCName();
    string pcIp = GetPCIP();
    string pcHwid = GetHWID();

    // Debugging output for POST data
    cout << "\n[DEBUG] POST Data being sent:\n";
    cout << "username=" << username << "&password=" << password << "&ip=" << pcIp << "&hwid=" << pcHwid << "&pc_name=" << pcName << endl;

    string postData = "username=" + username + "&password=" + password +
        "&ip=" + pcIp + "&hwid=" + pcHwid + "&pc_name=" + pcName;
    string response;
    cout << "\n[!] CONNECTING...\n";
    if (SendHttpRequest("YOUR LINK HERE", postData, response)) {
        cout << "[!] CONNECTED...\n";
        ProcessResponse(response);
    }
    else {
        SetColor(12);
        cout << " [!] Request error: Failed to connect.\n";
    }
}

int main() {
    HandleUserInput();
    return 0;
}

//THIS CODE STATES PURELY AS AN EXAMPLE ON HOW TO SENT HTTP REQUESTS, AND SENT SOME ADDITIONAL "SAFETY" DATA.
