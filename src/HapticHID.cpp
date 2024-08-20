#define NOMINMAX  // Add this to disable min/max macros
#include <windows.h>
#include "ViGEm/Client.h"
#include "drdc.h"
#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include <cmath>
#include <iomanip>

volatile sig_atomic_t stop = 0;  // Signal flag

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int signal) {
    stop = 1;  // Set flag to exit the loop
    std::cout << "\nSIGINT received, stopping...\n";
}

int main() {
    // Set up the signal handler for SIGINT
    std::signal(SIGINT, handle_sigint);

    std::cout << "Starting the program...\n";

    // Initialize the ViGEm client
    PVIGEM_CLIENT client = vigem_alloc();
    if (client == nullptr) {
        std::cerr << "Failed to allocate ViGEm client." << std::endl;
        return -1;
    }

    std::cout << "Connecting to ViGEmBus...\n";
    VIGEM_ERROR result = vigem_connect(client);
    if (result != VIGEM_ERROR_NONE) {
        std::cerr << "Failed to connect to ViGEmBus: Error " << result << std::endl;
        vigem_free(client);
        return -1;
    }

    // Create a virtual Xbox 360 controller
    std::cout << "Creating virtual Xbox controller...\n";
    PVIGEM_TARGET target = vigem_target_x360_alloc();
    result = vigem_target_add(client, target);
    if (result != VIGEM_ERROR_NONE) {
        std::cerr << "Failed to add virtual controller to ViGEmBus: Error " << result << std::endl;
        vigem_target_free(target);
        vigem_free(client);
        return -1;
    }

    std::cout << "Virtual Xbox controller created! Press Ctrl+C to stop...\n";

    // Force Dimension setup
    if (drdOpen() < 0) {
        std::cerr << "Error: Failed to open haptic device." << std::endl;
        return -1;
    }

    if (!drdIsSupported()) {
        std::cerr << "Error: Unsupported device type." << std::endl;
        drdClose();
        return -1;
    }

    if (!drdIsInitialized() && drdAutoInit() < 0) {
        std::cerr << "Error: Failed to initialize device." << std::endl;
        drdClose();
        return -1;
    }

    if (drdStart() < 0) {
        std::cerr << "Error: Failed to start robotic regulation." << std::endl;
        drdClose();
        return -1;
    }

    // Center the device in the workspace
    double positionCenter[DHD_MAX_DOF] = {};
    if (drdMoveTo(positionCenter) < 0) {
        std::cerr << "Error: Failed to move the device." << std::endl;
        drdClose();
        return -1;
    }

    drdSetEncIGain(0.0);  // Disable integral term

    // Main loop - runs until SIGINT is received
    while (!stop) {
        // Wait for the next tick of the regulation thread
        drdWaitForTick();

        // Get the force values
        double fx = 0.0, fy = 0.0, fz = 0.0;
        if (dhdGetForce(&fx, &fy, &fz) < 0) {
            std::cerr << "Error: Failed to get force." << std::endl;
            break;
        }

        // Get the position values
        double posX = 0.0, posY = 0.0, posZ = 0.0;
        if (drdGetPositionAndOrientation(&posX, &posY, &posZ, nullptr, nullptr, nullptr, nullptr, nullptr) < 0) {
            std::cerr << "Error: Failed to get position." << std::endl;
            break;
        }

        // Initialize a XUSB report structure
        XUSB_REPORT report;
        ZeroMemory(&report, sizeof(report));

        // Map position values to the left analog stick and left trigger
        report.sThumbLX = static_cast<SHORT>(posX * 32767 / DHD_MAX_DOF);
        report.sThumbLY = static_cast<SHORT>(posY * 32767 / DHD_MAX_DOF);
        report.bLeftTrigger = static_cast<BYTE>((std::min)(255.0, (std::max)(0.0, posZ * 255 / DHD_MAX_DOF)));

        // Map force values to the right analog stick and right trigger
        report.sThumbRX = static_cast<SHORT>(fx * 32767 / DHD_MAX_DOF);
        report.sThumbRY = static_cast<SHORT>(fy * 32767 / DHD_MAX_DOF);
        report.bRightTrigger = static_cast<BYTE>((std::min)(255.0, (std::max)(0.0, fz * 255 / DHD_MAX_DOF)));

        // Send the report to the virtual controller
        result = vigem_target_x360_update(client, target, report);
        if (result != VIGEM_ERROR_NONE) {
            std::cerr << "Failed to update virtual controller: Error " << result << std::endl;
            break;
        }

        // Wait 1 ms (1 kHz loop)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "Stopping and cleaning up...\n";

    // Cleanup when done
    drdClose();
    vigem_target_remove(client, target);
    vigem_target_free(target);
    vigem_free(client);

    return 0;
}
