# Haptic HID #  
This repository contains code for compiling a small `c++` application to run as an interface layer between Force Dimension's `sigma.7` robot and the web standard `WebHID` interface. By running this application on a device connected by USB with `sigma.7`, you can receive 3-axis position and force information from the robot in a standard web browser interface using javascript.  

## Requirements

- Windows
- MSYS2 with `gcc`, `ninja` (or `make`), and `cmake`
- Force Dimension SDK (e.g., version 3.17.1)
- ViGEmBus and ViGEmClient libraries

### Installation Steps

#### 1. Install MSYS2 and Required Packages

1. Download and install **MSYS2** from the official website: [MSYS2](https://www.msys2.org/).
2. Open **MSYS2 MinGW 64-bit** terminal.
3. Run the following commands to install the necessary tools and libraries:

    ```bash
    pacman -Syu           # Update the package database and core system
    pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake git
    ```

   This installs GCC, CMake, and other necessary development tools.

#### 2. Install ViGEmBus and ViGEmClient

1. Download and install the **ViGEmBus** driver from: [ViGEmBus Releases](https://github.com/ViGEm/ViGEmBus/releases).
2. Download the **ViGEmClient** SDK from: [ViGEmClient Releases](https://github.com/ViGEm/ViGEmClient/releases).

3. After extracting the ViGEmClient SDK, copy the `include` and `lib` directories to a path such as `C:/MyRepos/CPP/ViGEmClient/`.

#### 3. Install Force Dimension SDK

1. Download and install the **Force Dimension SDK** (e.g., version 3.17.1) from: [Force Dimension](https://www.forcedimension.com/download).
2. After installation, locate the SDK's `include` and `lib` directories, typically located in `C:/Program Files/Force Dimension/sdk-3.17.1/`.

## Compilation

### 1. Set Up the Project

Clone this repository and navigate to the project directory:

```bash
git clone https://github.com/Neuro-Mechatronics-Interfaces/HapticHID.git HapticHID && cd HapticHID
```

### 2. Configure and Build
1. Ensure that the paths in CMakeLists.txt are correctly set for your environment, especially for the ViGEmClient and Force Dimension SDK directories.

2. Create a build directory and run `cmake` to configure the project. Use `make` or `ninja` depending on your `cmake` configuration:

```bash
mkdir build && cd build && cmake .. && make
```
 
```bash
mkdir build && cd build && cmake .. && ninja
``` 

3. Copy the `ViGEmClient.dll` and `drd64.dll` link libraries from `lib` into `build`:  
```bash
cp ../lib/drd64.dll drd64.dll && cp ../lib/ViGEmClient.dll ViGEmClient.dll
```

This will generate the DummyHapticHID.exe and HapticHID.exe executables in the build directory.  

### 3. Copy/Paste DLLs
Both executables require ViGEmClient.dll and drd64.dll at runtime. Copy these .dll files into the build directory where the executables are located:  
* `ViGEmClient.dll` from the ViGEmClient SDK.
* `drd64.dll` from the Force Dimension SDK.

## Usage ##  
Run from a `bash` terminal (e.g. `git bash` or `msys2`; **not Powershell or Windows Terminal**). The terminal should navigate to the `build` folder in your local repository and call the executable from there.  

### Running `DummyHapticHID` ###
To run `DummyHapticHID`, which simulates a virtual XBOX 360 controller (and is therefore useful for testing WebHID when no `sigma.7` is available) execute the following command from a terminal:
```bash
./DummyHapticHID.exe
```

### Running `HapticHID` ###  
To run HapticHID, ensure that your Force Dimension haptic device is connected, and execute the following command:

```bash
./HapticHID.exe
```

This program connects to the haptic device, retrieves force and position data, and maps that data to a virtual Xbox 360 controller.

### Keyboard Shortcuts for Both Programs ###
* Ctrl + C: Stops the program and exits.

### Testing ###  
There is a generic HID test interface available at **[`https://reaction-task.nml.wtf/debug/hid`](https://reaction-task.nml.wtf/debug/hid)**. 
