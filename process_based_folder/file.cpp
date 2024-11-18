#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <algorithm> 
#include <unistd.h>  


std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

float getCPUTemperature() {
    const std::string hwmon_path = "/sys/class/hwmon/";
    for (const auto& entry : std::filesystem::directory_iterator(hwmon_path)) {
        std::ifstream name_file(entry.path() / "name");
        std::string sensor_name;
        if (name_file >> sensor_name && sensor_name == "coretemp") {
            std::ifstream temp_file(entry.path() / "temp1_input");
            int temp_millidegrees;
            if (temp_file >> temp_millidegrees) {
                return temp_millidegrees / 1000.0; // Convert to degrees Celsius
            }
        }
    }
    return -1; 
}


std::string getProcessName(int pid) {
    std::ifstream comm_file("/proc/" + std::to_string(pid) + "/comm");
    std::string process_name;
    if (comm_file.is_open()) {
        std::getline(comm_file, process_name);
    }
    return process_name.empty() ? "Unknown" : process_name;
}

// Function to get CPU usage of all processes
std::map<int, float> getProcessCPUUsage() {
    std::map<int, float> process_cpu_usage;
    std::ifstream stat_file("/proc/stat");
    std::string line;

    
    getline(stat_file, line);
    std::istringstream iss(line);
    std::string cpu;
    long total_jiffies = 0, value;
    iss >> cpu;
    while (iss >> value) {
        total_jiffies += value;
    }

  
    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string dirname = entry.path().filename();
            if (std::all_of(dirname.begin(), dirname.end(), ::isdigit)) { // Check if PID
                int pid = std::stoi(dirname);
                std::ifstream proc_stat(entry.path() / "stat");
                if (proc_stat.is_open()) {
                    std::string stat_line;
                    getline(proc_stat, stat_line);
                    std::istringstream proc_iss(stat_line);
                    std::string temp;
                    long utime, stime, starttime;
                    proc_iss >> temp >> temp >> temp; 
                    for (int i = 0; i < 11; i++) proc_iss >> temp; 
                    proc_iss >> utime >> stime >> starttime; 
                    long total_time = utime + stime; 
                    float cpu_time = (float)total_time / sysconf(_SC_CLK_TCK); 
                    process_cpu_usage[pid] = cpu_time; 
                }
            }
        }
    }
    return process_cpu_usage;
}


void logContributionsToCSV(const std::string& filename, const std::string& timestamp, 
                           float base_temp, float current_temp, const std::map<int, float>& process_usage) {
    std::ofstream csv_file;
    bool file_exists = std::filesystem::exists(filename);

    // Open the file in append mode
    csv_file.open(filename, std::ios::app);
    if (!file_exists) {
        // Write the header if the file doesn't exist
        csv_file << "Timestamp,Base Temperature (°C),Current Temperature (°C),PID,Process Name,Process CPU Usage (seconds),Thermal Contribution (°C)\n";
    }

    float temp_diff = current_temp - base_temp;
    float total_cpu = 0;

    for (const auto& [pid, cpu_usage] : process_usage) {
        total_cpu += cpu_usage;
    }

    for (const auto& [pid, cpu_usage] : process_usage) {
        float contribution = (total_cpu > 0) ? (cpu_usage / total_cpu) * temp_diff : 0;
        std::string process_name = getProcessName(pid);
        csv_file << timestamp << "," << base_temp << "," << current_temp << "," << pid << ","
                 << process_name << "," << std::fixed << std::setprecision(2) 
                 << cpu_usage << "," << contribution << "\n";
    }

    csv_file.close();
}


int main() {
    const std::string output_csv = "thermal_profile_updated_latest.csv";

    std::cout << "Starting Thermal Profiling...\n";

    float base_temp = getCPUTemperature();
    if (base_temp < 0) {
        std::cerr << "Failed to read CPU temperature. Exiting...\n";
        return 1;
    }

    std::cout << "Base Temperature: " << base_temp << "°C\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Sampling interval
        float current_temp = getCPUTemperature();
        if (current_temp < 0) {
            std::cerr << "Failed to read CPU temperature. Skipping...\n";
            continue;
        }

        auto process_usage = getProcessCPUUsage();
        std::string timestamp = getCurrentTimestamp();

        logContributionsToCSV(output_csv, timestamp, base_temp, current_temp, process_usage);

        std::cout << "Logged data at " << timestamp << " to " << output_csv << "\n";

        
        base_temp = current_temp;
    }

    return 0;
}

