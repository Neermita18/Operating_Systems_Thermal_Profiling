#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <cstdlib>
#include <cstdio>

namespace fs = std::filesystem;

const std::string CPU_LOG_FILE = "cpu_temperature_log.csv";
const std::string SSD_LOG_FILE = "ssd_temperature_log.csv";
const std::string CASE_LOG_FILE = "case_temperature_log.csv";
const std::string MOTHERBOARD_LOG_FILE = "motherboard_temperature_log.csv";

const int MAX_LOG_SIZE = 10 * 1024 * 1024; 
const int LOG_INTERVAL_SECONDS = 5;       

std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}


std::string executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        return "Command Execution Failed";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Function to get CPU temperature
std::string getCPUTemperature() {
    std::string command = "sensors | grep 'Package id 0:' | awk '{print $4}'";
    std::string output = executeCommand(command);
    return output.empty() ? "N/A" : output;
}

// Function to get NVMe SSD temperature
std::string getSSDTemperature() {
    std::string command = "nvme smart-log /dev/nvme0 | grep 'temperature' | awk '{print $3 \"°C\"}'";
    std::string output = executeCommand(command);
    return output.empty() ? "N/A" : output;
}


std::string getMotherboardTemperature() {
    std::string command = "sensors | grep 'temp1:' | awk 'NR==1 {print $2}'";
    std::string output = executeCommand(command);
    return output.empty() ? "N/A" : output;
}


std::string getCaseTemperature() {
    std::string command = "sensors | grep 'temp1:' | awk 'NR==2 {print $2}'";
    std::string output = executeCommand(command);
    return output.empty() ? "N/A" : output;
}


void writeLog(const std::string& logData, const std::string& logFile) {
    std::ofstream logFileStream(logFile, std::ios::app);
    if (logFileStream.is_open()) {
        logFileStream << logData << std::endl;
        logFileStream.close();
    } else {
        std::cerr << "Error: Unable to open log file for writing: " << logFile << std::endl;
    }
}


void rotateLogs(const std::string& logFile) {
    if (fs::exists(logFile) && fs::file_size(logFile) > MAX_LOG_SIZE) {
        std::string backupFile = logFile + "." + getCurrentTimestamp() + ".bak";
        fs::rename(logFile, backupFile);
        std::cout << "Log file rotated: " << backupFile << std::endl;
    }
}


void collectAndLogTemperatureData() {
    std::string cpuTemp = getCPUTemperature();
    std::string ssdTemp = getSSDTemperature();
    std::string motherboardTemp = getMotherboardTemperature();
    std::string caseTemp = getCaseTemperature();

    // Log data for CPU temperature
    std::string cpuLogData = getCurrentTimestamp() + "," + cpuTemp;
    writeLog(cpuLogData, CPU_LOG_FILE);

    // Log data for SSD temperature
    std::string ssdLogData = getCurrentTimestamp() + "," + ssdTemp;
    writeLog(ssdLogData, SSD_LOG_FILE);

    // Log data for Motherboard temperature
    std::string motherboardLogData = getCurrentTimestamp() + "," + motherboardTemp;
    writeLog(motherboardLogData, MOTHERBOARD_LOG_FILE);

    // Log data for Case temperature
    std::string caseLogData = getCurrentTimestamp() + "," + caseTemp;
    writeLog(caseLogData, CASE_LOG_FILE);
}


void startLogging() {
    std::cout << "Starting temperature logging..." << std::endl;

   
    std::ofstream cpuLogFile(CPU_LOG_FILE, std::ios::app);
    if (cpuLogFile.tellp() == 0) {
        cpuLogFile << "Timestamp,CPU Temperature" << std::endl;
    }
    cpuLogFile.close();

    std::ofstream ssdLogFile(SSD_LOG_FILE, std::ios::app);
    if (ssdLogFile.tellp() == 0) {
        ssdLogFile << "Timestamp,SSD Temperature" << std::endl;
    }
    ssdLogFile.close();

    std::ofstream motherboardLogFile(MOTHERBOARD_LOG_FILE, std::ios::app);
    if (motherboardLogFile.tellp() == 0) {
        motherboardLogFile << "Timestamp,Motherboard Temperature" << std::endl;
    }
    motherboardLogFile.close();

    std::ofstream caseLogFile(CASE_LOG_FILE, std::ios::app);
    if (caseLogFile.tellp() == 0) {
        caseLogFile << "Timestamp,Case Temperature" << std::endl;
    }
    caseLogFile.close();

    while (true) {
        
        rotateLogs(CPU_LOG_FILE);
        rotateLogs(SSD_LOG_FILE);
        rotateLogs(MOTHERBOARD_LOG_FILE);
        rotateLogs(CASE_LOG_FILE);

        
        collectAndLogTemperatureData();

      
        std::this_thread::sleep_for(std::chrono::seconds(LOG_INTERVAL_SECONDS));
    }
}

int main() {
    std::cout << "Initializing Temperature Logger..." << std::endl;

   
    std::thread loggingThread(startLogging);

    
    loggingThread.join();

    return 0;
}

