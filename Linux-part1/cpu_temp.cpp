#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

std::vector<float> get_cpu_temperatures() {
    std::vector<float> core_temperatures;
    
   
    FILE* fp = popen("sensors | grep 'Core' | awk '{print $3}'", "r");
    if (fp == nullptr) {
        std::cerr << "Unable to get CPU temperatures." << std::endl;
        return core_temperatures;
    }

    char temp[10];  
    while (fscanf(fp, "%s", temp) == 1) {
        
        float core_temp = 0.0;
        if (temp[0] == '+') {
            core_temp = atof(temp + 1);  
        } else {
            core_temp = atof(temp);  
        }
        core_temperatures.push_back(core_temp); 
    }
    fclose(fp);

    return core_temperatures;
}

float calculate_avg_cpu_temperature(const std::vector<float>& core_temperatures) {
    if (core_temperatures.empty()) {
        return -1;  
    }

    float sum = 0.0;
    for (float temp : core_temperatures) {
        sum += temp;
    }
    return sum / core_temperatures.size();  
}

int main() {
 
    std::vector<float> core_temperatures = get_cpu_temperatures();

    if (core_temperatures.empty()) {
        std::cerr << "No core temperature data available." << std::endl;
        return 1;
    }

   
    std::cout << "CPU Core Temperatures:" << std::endl;
    for (size_t i = 0; i < core_temperatures.size(); ++i) {
        std::cout << "Core " << i << ": " << core_temperatures[i] << " °C" << std::endl;
    }

   
    float avg_temp = calculate_avg_cpu_temperature(core_temperatures);
    if (avg_temp != -1) {
        std::cout << "Average CPU Temperature: " << avg_temp << " °C" << std::endl;
    } else {
        std::cerr << "Error calculating average temperature." << std::endl;
    }

    return 0;
}

