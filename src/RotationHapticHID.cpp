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
double jointAngle[DHD_MAX_DOF];
double jointVelocity[DHD_MAX_DOF];
double jointAngleTarget[DHD_MAX_DOF];
double jointTorque[DHD_MAX_DOF];
bool   lock[DHD_MAX_DOF];

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int signal) {
    stop = 1;  // Set flag to exit the loop
    std::cout << "\nSIGINT received, stopping...\n";
}

int main() {

    // regulation stiffness
    const double Kj[DHD_MAX_DOF] = { 0.0, 0.0, 0.0,
                                     4.0, 3.0, 1.0,    // locked wrist joint stiffness (in Nm/rad)
                                     0.0,
                                     0.0 };

    // regulation viscosity
    const double Kv[DHD_MAX_DOF] = { 0.0, 0.0, 0.0,
                                     0.04, 0.03, 0.01, // locked wrist joint viscosity (in Nm/(rad/s))
                                     0.0,
                                     0.0 };


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

    // display instructions
    printf("press 'q' to quit\n");
    printf("      '0' to toggle virtual lock on wrist joint 0\n");
    printf("      '1' to toggle virtual lock on wrist joint 1\n");
    printf("      '2' to toggle virtual lock on wrist joint 2\n");
    printf("      'a' to toggle virtual lock on all wrist joints\n\n");


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

        // retrieve wrist joints
        dhdGetJointAngles(jointAngle);
        dhdGetJointVelocities(jointVelocity);

        // compute joint torques as appropriate
        for (int i = 3; i < 6; i++) {
            if (lock[i]) jointTorque[i] = -Kj[i] * (jointAngle[i] - jointAngleTarget[i]) - Kv[i] * jointVelocity[i];
            else         jointTorque[i] = 0.0;
        }

        // apply as appropriate
        int res = dhdSetForceAndWristJointTorquesAndGripperForce(0.0, 0.0, 0.0,
                                                                 jointTorque[3], jointTorque[4], jointTorque[5],
                                                                 0.0);
        if (res < DHD_NO_ERROR) {
            printf("error: cannot set force (%s)\n", dhdErrorGetLastStr());
            stop = 1;
        }

        // user input
        if (dhdKbHit()) {
            switch (dhdKbGet()) {
            case 'q': stop = 1; break;
            case 'a':
                lock[3] = lock[4] = lock[5] = !lock[3];
                if (lock[3]) for (int i = 3; i < 6; i++) jointAngleTarget[i] = jointAngle[i];
                break;
            case '0':
                lock[3] = !lock[3];
                jointAngleTarget[3] = jointAngle[3];
                break;
            case '1':
                lock[4] = !lock[4];
                jointAngleTarget[4] = jointAngle[4];
                break;
            case '2':
                lock[5] = !lock[5];
                jointAngleTarget[5] = jointAngle[5];
                break;
            }
        }

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
