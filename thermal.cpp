#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#pragma comment(lib, "wbemuuid.lib")

void logTemperatureData() {
    HRESULT hres;

    // Initialize COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED); 
    if (FAILED(hres)) {
        std::cout << "Failed to initialize COM library." << std::endl;
        return;
    }

    // Set COM security levels
    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );
    if (FAILED(hres)) {
        std::cout << "Failed to initialize security." << std::endl;
        CoUninitialize();
        return;
    }

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *)&pLoc
    );
    if (FAILED(hres)) {
        std::cout << "Failed to create IWbemLocator object." << std::endl;
        CoUninitialize();
        return;
    }

    // Connect to WMI
    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc
    );
    if (FAILED(hres)) {
        std::cout << "Could not connect to WMI namespace." << std::endl;
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE
    );

    if (FAILED(hres)) {
        std::cout << "Could not set proxy blanket." << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Create log file
    std::ofstream logFile("temperature_log.txt", std::ios::out | std::ios::app);
    if (!logFile) {
        std::cout << "Could not open log file." << std::endl;
        return;
    }

    // Loop to retrieve and log temperature data
    for (int i = 0; i < 10; ++i) { // Run for 10 intervals
        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_PerfFormattedData_Counters_ThermalZoneInformation"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );

        if (FAILED(hres)) {
            std::cout << "WMI query failed." << std::endl;
            break;
        }

        IWbemClassObject *pclsObj = NULL;
        ULONG uReturn = 0;

        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn) {
                break;
            }

            VARIANT vtProp;
            hr = pclsObj->Get(L"Temperature", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr) && vtProp.vt != VT_NULL) {
                // Display and log the temperature in Celsius
                double temperatureCelsius = vtProp.uintVal / 10.0;
                std::cout << "Current Temperature: " << temperatureCelsius << " °C" << std::endl;
                logFile << "Current Temperature: " << temperatureCelsius << " °C" << std::endl;
            } else {
                std::cout << "Failed to retrieve temperature data." << std::endl;
                logFile << "Failed to retrieve temperature data." << std::endl;
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

        pEnumerator->Release();

        // Wait for 1 second before next log
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clean up
    logFile.close();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
}

int main() {
    std::cout << "Starting Temperature Logging..." << std::endl;
    logTemperatureData();
    std::cout << "Temperature Logging Complete." << std::endl;
    return 0;
}
