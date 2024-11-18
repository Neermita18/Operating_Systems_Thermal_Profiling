#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>


std::string runCommand(const std::string& command) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: unable to run command." << std::endl;
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    fclose(pipe);
    return result;
}


void getMotherboardTemperature() {
    std::string output = runCommand("sensors");

    
    size_t pos = output.find("Core 0");
    if (pos != std::string::npos) {
        size_t start = output.find(":", pos);
        size_t end = output.find("°C", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string temperature = output.substr(start + 1, end - start - 1);
            std::cout << "Motherboard/Core temperature: " << temperature << "°C" << std::endl;
        }
    } else {
        std::cout << "Motherboard temperature data not found!" << std::endl;
    }
}


void getHardDriveTemperature() {
    
    std::string output = runCommand("sudo smartctl -A /dev/sda");

    
    size_t pos = output.find("Temperature_Celsius");
    if (pos != std::string::npos) {
        size_t start = output.find(" ", pos);
        size_t end = output.find("\n", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string temperature = output.substr(start + 1, end - start - 1);
            std::cout << "Hard Drive Temperature: " << temperature << "°C" << std::endl;
        }
    } else {
        std::cout << "Hard drive temperature data not found!" << std::endl;
    }
}


void getCaseTemperature() {
    std::string output = runCommand("sensors");

    
    size_t pos = output.find("temp1");
    if (pos != std::string::npos) {
        size_t start = output.find(":", pos);
        size_t end = output.find("°C", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string temperature = output.substr(start + 1, end - start - 1);
            std::cout << "Case Temperature: " << temperature << "°C" << std::endl;
        }
    } else {
        std::cout << "Case temperature data not found!" << std::endl;
    }
}


int main() {
    std::cout << "Collecting temperature data from system sensors..." << std::endl;

   
    getMotherboardTemperature();
    getHardDriveTemperature();
    getCaseTemperature();

    return 0;
}

