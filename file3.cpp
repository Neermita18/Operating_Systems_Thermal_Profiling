#include <iostream>
#include <string>
#include <memory>
#include <cstdio>
#include <stdexcept>
#include <regex>
#include <fstream>
#include <chrono>
#include <thread>

// Function to execute the `sensors` command and capture its output
std::string getTemperatureData() {
    std::string result;
    char buffer[128];

    // Run the `sensors` command and open a pipe to read its output
    std::shared_ptr<FILE> pipe(popen("sensors", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to run sensors command.");
    }

    // Read the output of sensors line by line and append to result string
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

// Function to extract CPU temperature from the `sensors` output using regex
float extractCPUTemperature(const std::string& data) {
    // Regular expression to find lines like "Core 0: +60.0°C"
    std::regex tempRegex("Core\\s+[0-9]+:\\s+\\+([0-9]+\\.[0-9]+)°C");
    std::smatch match;

    // Search for the pattern and extract the temperature if found
    if (std::regex_search(data, match, tempRegex) && match.size() > 1) {
        return std::stof(match[1]);  // Convert matched temperature to float
    } else {
        throw std::runtime_error("CPU temperature not found.");
    }
}

// Function to get the current timestamp as a string
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&in_time_t));
    return std::string(buffer);
}

// Function to log temperature data at regular intervals
void logTemperatureData(const std::string& filename, int interval_seconds) {
    // Open the file in append mode
    std::ofstream logfile(filename, std::ios::app);
    if (!logfile.is_open()) {
        throw std::runtime_error("Failed to open log file.");
    }

    while (true) {
        try {
            // Get the current temperature data
            std::string temperatureData = getTemperatureData();
            float cpuTemp = extractCPUTemperature(temperatureData);

            // Get the current timestamp
            std::string timestamp = getCurrentTimestamp();

            // Write the timestamp and temperature to the log file
            logfile << timestamp << ", " << cpuTemp << "°C" << std::endl;
            std::cout << "Logged: " << timestamp << ", " << cpuTemp << "°C" << std::endl;

            // Flush to ensure data is written to disk
            logfile.flush();

        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Wait for the specified interval before the next reading
        std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
    }
}

int main() {
    // Log file name and logging interval in seconds
    const std::string logFilename = "temperature_log.csv";
    const int intervalSeconds = 5;

    try {
        // Start logging temperature data
        logTemperatureData(logFilename, intervalSeconds);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
