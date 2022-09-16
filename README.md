# Getting Started with WIZnet RP2040 DUINO COIN Mining stack example



![WIZnet W5100S-PICO-Mining Rig](https://user-images.githubusercontent.com/88420728/190546075-b883623d-fd64-4496-9c9d-638037f8c070.png)

<p align="center">WIZnet W5100S-PICO-Mining Rig</p>

### Introduction
This is the example project for multi slave duino coin miner rig using WIZnet W5100S and RP2040. Total 10 Slaves, 20 cores can be used for the hash calculation but you can increase the number of slaves by modifying the schematics and software.

![W5100S-PICO-MINING RIG BLOCK DIAGRAM](https://user-images.githubusercontent.com/88420728/190548602-a0b316a0-9bfb-43d8-9276-cf3f1570f630.png)

<p align="center">W5100S-PICO-MINING RIG BLOCK DIAGRAM</p>

**Restrictions**: There is a limit on the number of miners. To get rid of this limitation, you will need to purchase additional slots for mining machines from the duino wallet site. Or "Too many miners" warning will be shown and the mining efficiency drops significantly.

### Getting started
0. Preparation

   - Duino Coin wallet: https://wallet.duinocoin.com/
   - W5100S-PICO-Mining rig or hardware with the same schematic
     - schematic URL:
   - Micro USB cable
   - Internet router
   - Ethernet cable

1. Download

   Clone the WIZnet RP2040 DUINO COIN Mining stack example code using the Git command on your environment

   ```bash
   > git clone https://github.com/Wiznet/RP2040-HAT-MINING-C.git
   ```

2. Input your Duino information

   Open _examples/minerMaster/minerMaster.cpp_ and inpur your user name and miner key

   ```c
   /* Duino coin information for the mining*/
   // User name : input yours
   #define  DUINO_USER_NAME  "user name"
   // Miner key : input yours
   #define  DUINO_MINER_KEY  "miner key"
   ```

3. Build
   
   Create build folder
   ```bash
   > cd RP2040-HAT-MINING-C
   > mkdir build
   > cd build
   ```
   CMake and make
   ```bash
   > cmake ..
   > make -j8
   ```
   ###### option "-j8" means that 8 cores will be used to make
   
4. Flash program

   After the build, two binary files are created in the following folder.
   - Master binary: _build/examples/minerMaster/minerMaster.uf2_
   - Slave binary: _build/examples/minerSlave/minerSlave.uf2_
   
   Binary file can be programed using BOOTSEL button on the RP2040 board. If the micro usb cable is connected to the PC while the button is pressed, it is recognized as an external storage device. Programming is done by simply copying the binary file to the external storage device.
   
   <p align="center"><img src="https://user-images.githubusercontent.com/88420728/190547939-6e2761eb-3fbc-41d3-9602-7aa06aa549d7.gif" align="center" /></p>
   <p align="center">RPI-PICO Flash Programming</p>

5. Test

   Put the all programmed slaves to the slots and connect the miner rig to the internet router using ethernet cable. Supply power using the usb cable or POE. A few minutes after, the mining status is displayed through the TFT LCD on the board.

   ![Mining Status](https://user-images.githubusercontent.com/88420728/190548374-5f6d5f03-ae30-4751-aceb-a1fc714333fd.png)
   <p align="center">Mining Status</p>

   The mining status can be checked from the duino coin wallet web site too.
