#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

float get_gpu_temperature_sensors() {
   
    FILE* fp = popen("sensors", "r");
    if (fp == nullptr) {
        std::cerr << "Unable to execute 'sensors' command." << std::endl;
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
       
        if (strstr(line, "GPU") != nullptr) {
           
            float gpu_temp = 0.0;
            if (sscanf(line, "%*s %f", &gpu_temp) == 1) {
                fclose(fp);
                return gpu_temp;
            }
        }
    }

    std::cerr << "GPU temperature not found in 'sensors' output." << std::endl;
    fclose(fp);
    return -1;
}

int main() {
   
    float gpu_temp = get_gpu_temperature_sensors();
    if (gpu_temp != -1) {
        std::cout << "GPU Temperature: " << gpu_temp << " °C" << std::endl;
    }

    return 0;
}

