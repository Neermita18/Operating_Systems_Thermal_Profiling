#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <string>
#include <cstdlib>
#include <array>
#include <stdexcept>


const float CPU_TEMP_THRESHOLD = 40.0;
const float MOTHERBOARD_TEMP_THRESHOLD = 65.0;


std::string executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

// Function to get CPU temperature
std::string getCPUTemperature() {
    std::string command = "sensors | grep 'Package id 0:' | awk '{print $4}'";
    std::string output = executeCommand(command);
    return output.empty() ? "N/A" : output;
}

// Function to get Motherboard temperature
std::string getMotherboardTemperature() {
    std::string command = "sensors | grep 'temp1:' | awk 'NR==1 {print $2}'";
    std::string output = executeCommand(command);
    return output.empty() ? "N/A" : output;
}

// Function to log temperature data to respective CSV files
void logTemperatureData(const std::string& cpuTemp, const std::string& motherboardTemp) {
    
    std::time_t now = std::time(nullptr);
    std::string timeStr = std::ctime(&now);
    timeStr.pop_back(); 

   
    std::ofstream cpuFile("cpu_temp.csv", std::ios::app);
    if (cpuFile.is_open()) {
        cpuFile << timeStr << ", " << cpuTemp << std::endl;
        cpuFile.close();
    } else {
        std::cerr << "Error: Could not open cpu_temp.csv for writing!" << std::endl;
    }

    
    std::ofstream motherboardFile("motherboard_temp.csv", std::ios::app);
    if (motherboardFile.is_open()) {
        motherboardFile << timeStr << ", " << motherboardTemp << std::endl;
        motherboardFile.close();
    } else {
        std::cerr << "Error: Could not open motherboard_temp.csv for writing!" << std::endl;
    }
}


void logAlert(const std::string& component, const std::string& temperature) {
    std::ofstream alertFile("temperature_alerts.log", std::ios::app);
    if (alertFile.is_open()) {
        // Get current time for alert
        std::time_t now = std::time(nullptr);
        std::string timeStr = std::ctime(&now);
        timeStr.pop_back(); // Remove the newline character

        alertFile << timeStr << " - ALERT: " << component << " temperature exceeded threshold! Current: "
                  << temperature << std::endl;
        alertFile.close();
    } else {
        std::cerr << "Error: Could not open temperature_alerts.log for writing!" << std::endl;
    }
}


void monitorTemperatures() {
    while (true) {
        // Get CPU and Motherboard temperatures
        std::string cpuTemp = getCPUTemperature();
        std::string motherboardTemp = getMotherboardTemperature();

        // Log the temperature data to CSV
        logTemperatureData(cpuTemp, motherboardTemp);

       
        if (cpuTemp != "N/A" && std::stof(cpuTemp.substr(1)) > CPU_TEMP_THRESHOLD) {
            std::cerr << "ALERT: CPU temperature high! (" << cpuTemp << ")" << std::endl;
            logAlert("CPU", cpuTemp);
        }

        
        if (motherboardTemp != "N/A" && std::stof(motherboardTemp.substr(1)) > MOTHERBOARD_TEMP_THRESHOLD) {
            std::cerr << "ALERT: Motherboard temperature high! (" << motherboardTemp << ")" << std::endl;
            logAlert("Motherboard", motherboardTemp);
        }

       
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

int main() {
    std::cout << "Starting temperature monitoring and alerting system..." << std::endl;

    
    std::thread monitorThread(monitorTemperatures);

    
    monitorThread.detach();

    
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }

    return 0;
}

