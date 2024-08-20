#include <windows.h>
#include "ViGEm/Client.h"
#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>

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

    // Initial values for position and force
    SHORT posX = 0, posY = 0;
    SHORT forceX = 0, forceY = 0;
    BYTE posZ = 0, forceZ = 0;

    // Direction for incrementing or decrementing values
    SHORT posStep = 1000, forceStep = 1000;
    BYTE triggerStep = 5;

    // Main loop - runs until SIGINT is received
    while (!stop) {
        // Initialize a XUSB report structure
        XUSB_REPORT report;
        ZeroMemory(&report, sizeof(report));

        // Update position values (X, Y, Z) - Simulate analog stick movement
        posX += posStep;
        posY += posStep;
        posZ += triggerStep;

        // Update force values (X, Y, Z) - Simulate force feedback movement
        forceX += forceStep;
        forceY += forceStep;
        forceZ += triggerStep;

        // Ensure the values remain within valid ranges for sticks and triggers
        if (posX > 32767 || posX < -32767) posStep = -posStep;
        if (posY > 32767 || posY < -32767) posStep = -posStep;
        if (posZ > 255 || posZ < 0) triggerStep = -triggerStep;

        if (forceX > 32767 || forceX < -32767) forceStep = -forceStep;
        if (forceY > 32767 || forceY < -32767) forceStep = -forceStep;
        if (forceZ > 255 || forceZ < 0) triggerStep = -triggerStep;

        // Map position values to the left analog stick and left trigger
        report.sThumbLX = posX;
        report.sThumbLY = posY;
        report.bLeftTrigger = posZ;

        // Map force values to the right analog stick and right trigger
        report.sThumbRX = forceX;
        report.sThumbRY = forceY;
        report.bRightTrigger = forceZ;

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
    vigem_target_remove(client, target);
    vigem_target_free(target);
    vigem_free(client);

    return 0;
}
