#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <thread>       
#include <chrono>     

float getSSDTemperature() {
    FILE* fp = popen("sudo smartctl -a /dev/sda | grep -i 'Temperature_Celsius'", "r");
    if (fp == nullptr) {
        return -1; 
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        
        std::string output(buffer);
        size_t pos = output.find("Temperature_Celsius");
        if (pos != std::string::npos) {
            std::stringstream ss(output.substr(pos));
            float temp;
            ss >> temp;
            fclose(fp);
            return temp;
        }
    }
    fclose(fp);
    return -1; 
}


void logSSDTemperature() {
    std::ofstream file("ssd_temperature_log_updated.csv", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error opening file for logging." << std::endl;
        return;
    }

    
    std::time_t now = std::time(nullptr);
    std::tm* currentTime = std::localtime(&now);

   
    std::stringstream timeStream;
    timeStream << (currentTime->tm_year + 1900) << "-"
               << (currentTime->tm_mon + 1) << "-"
               << currentTime->tm_mday << " "
               << currentTime->tm_hour << ":"
               << currentTime->tm_min << ":"
               << currentTime->tm_sec;

    
    float temp = getSSDTemperature();
    if (temp == -1) {
        file << timeStream.str() << ", NA" << std::endl; 
    } else {
        file << timeStream.str() << ", " << temp << "°C" << std::endl;
    }

    file.close();
}

int main() {
    // Log the SSD temperature every 10 seconds
      while (true) {
        logSSDTemperature();
        std::this_thread::sleep_for(std::chrono::seconds(10));  
    }
    return 0;
}
