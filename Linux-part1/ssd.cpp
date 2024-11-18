#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

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

void getSSDNVMeTemperature() {
    std::string output = runCommand("sudo nvme smart-log /dev/nvme0");

   
    size_t pos = output.find("temperature");
    if (pos != std::string::npos) {
        size_t start = output.find(":", pos) + 2;
        size_t end = output.find("\n", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string temperature = output.substr(start, end - start);
            std::cout << "NVMe SSD Temperature: " << temperature << "°C" << std::endl;
        }
    } else {
        std::cout << "No temperature data found for NVMe SSD!" << std::endl;
    }
}

void getHDDTemperature() {
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
        std::cout << "No hard drive temperature data found!" << std::endl;
    }
}

int main() {
    std::cout << "Collecting temperature data..." << std::endl;
    getSSDNVMeTemperature();  
    getHDDTemperature();    
    return 0;
}
