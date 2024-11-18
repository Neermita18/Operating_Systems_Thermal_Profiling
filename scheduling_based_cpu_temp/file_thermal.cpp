#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iterator>

namespace fs = std::filesystem;


float getCPUTemperature() {
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    float temperature = 0;
    if (tempFile.is_open()) {
        tempFile >> temperature;
        tempFile.close();
        temperature /= 1000; 
    }
    return temperature;
}


std::map<int, float> getProcessCPUUsage() {
    std::map<int, float> cpuUsageMap;

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string dirname = entry.path().filename().string();

            
            if (std::all_of(dirname.begin(), dirname.end(), ::isdigit)) {
                int pid = std::stoi(dirname);
                std::string statPath = "/proc/" + dirname + "/stat";

                std::ifstream statFile(statPath);
                if (statFile.is_open()) {
                    std::string line;
                    std::getline(statFile, line);

                    std::istringstream iss(line);
                    std::vector<std::string> stats((std::istream_iterator<std::string>(iss)),
                                                   std::istream_iterator<std::string>());


                    if (stats.size() > 13) {
                        long utime = std::stol(stats[13]);
                        long stime = std::stol(stats[14]);
                        float totalTime = (utime + stime) / static_cast<float>(sysconf(_SC_CLK_TCK));
                        cpuUsageMap[pid] = totalTime;
                    }
                }
            }
        }
    }

    return cpuUsageMap;
}


void adjustProcessPriority(int pid, int newPriority) {
    if (setpriority(PRIO_PROCESS, pid, newPriority) == 0) {
        std::cout << "Adjusted priority for PID: " << pid << " to: " << newPriority << "\n";
    } else {
        perror("Failed to adjust priority");
    }
}

void adjustProcessAffinity(int pid, int core) {
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(core, &cpuSet);
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpuSet) == 0) {
        std::cout << "Set affinity for PID: " << pid << " to core: " << core << "\n";
    } else {
        perror("Failed to set affinity");
    }
}

// Function to simulate thermal-aware scheduling
void thermalAwareScheduler() {
    const float THERMAL_THRESHOLD = 42.0; 
    const int LOWER_PRIORITY = 19;        
    const int TARGET_CORE = 1;            

    while (true) {
        float currentTemp = getCPUTemperature();
        std::cout << "Current CPU Temperature: " << currentTemp << "°C\n";

        if (currentTemp > THERMAL_THRESHOLD) {
            std::cout << "Temperature exceeded threshold. Adjusting processes...\n";

          
            auto processUsage = getProcessCPUUsage();

            
            int topPid = -1;
            float maxCpuUsage = 0.0;
            for (const auto& [pid, usage] : processUsage) {
                if (usage > maxCpuUsage) {
                    maxCpuUsage = usage;
                    topPid = pid;
                }
            }

            if (topPid != -1) {
                std::cout << "Top CPU-consuming PID: " << topPid << " with usage: " << maxCpuUsage << "\n";

                
                adjustProcessPriority(topPid, LOWER_PRIORITY);
                adjustProcessAffinity(topPid, TARGET_CORE);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); 
    }
}

int main() {
    std::cout << "Starting Thermal-Aware Scheduler...\n";
    thermalAwareScheduler();
    return 0;
}
