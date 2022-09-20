
## This project uses the dual core of RP2040 to operate one RP2040 chip as two miners.

# How to Test DUINO COIN Example


## Step 1: Prepare hardware

If you are using W5100S-EVB-Pico or W5500-EVB-Pico, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico or W5500-EVB-Pico ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable.


## Step 2: Prepare software

The following serial terminal program is required for DUINOCOIN example test, download and install from below link.

- [**Tera Term**][link-tera_term]


## Step 3: Prepare Duino Coin Web wallet

Create an account by accessing the Duino Coin Main Web from the link below.

- [**DUINO_COIN**][link-duino_coin_main]

Register a wallet on the network-> Web Wallet
![][link-duino_coin_web_main]


## Step 4: Setup Duino Example

To test the DUINO COIN example, minor settings shall be done in code.

1. Setup SPI port and pin in 'w5x00_spi.h' in 'RP2040-HAT-MINING-C/port/ioLibrary_Driver/' directory.

   Set the SPI interface you use.

```cpp
/* SPI */
#define SPI_PORT spi0

#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_RST 20
```

   If you want to test with the DUINO COIN example using SPI DMA, uncommnet USE_SPI_DMA.

```cpp
/* Use SPI DMA */
//#define USE_SPI_DMA // if you want to use SPI DMA, uncomment.
```


2. Setup network configuration such as IP in 'duino_coin.c', which is the DUINO COIN example in 'RP2040-HAT-MINING-C/examples/duino
_coin/' directory.

   We are going to use Static IP. However, If you want to use 'DHCP' set the IP and other network settings to suit your network environment.

```cpp
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};
```

1. Setup duino coin user information such as USER_NAME, MINING_KEY in 'duino_coin.c', which is the DUINO COIN example in 'RP2040-HAT-MINING-C/examples/duino
_coin/' directory.

 ```cpp
/* duino coin */
	#define DUINO_USER_NAME         "user name"
	#define DUINO_MINER_KEY         "miner key"
};
```
- 'DUINO_CHIP_ID' is a chip id or any value if no chip id
- You can check user name and maining key on the main page of the Duino Web Wallet. 
- [**WEB_WALLET**][link-duino_coin_wallet]
![][link-duino_coin_web_minimng_info]

4. Duino coin State Flow
   State flow of the DUINO COIN is as follows.
 
![][link-duino_coin_state_flow]



## Step 5: Build

1. After completing the DUINO COIN example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'duino_coin.uf2' is generated in 'RP2040-HAT-MINING-C/build/examples/duino_coin/'.



## Step 6: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'duino_coin.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the DUINO COIN example works normally, you can see the Host information got from the Duino URL

![][link-connect_to_doino_coin_url]

6. If the connection with the Duino Coin server is successful, the mining process will work

![][link-duino_coin_hash_process]

7. Also, the web site dashboard will show mining status 
 - Sometimes it takes more than 5 minutes for mining to be applied to the web
 - [GOOD share #N: ...]: Success Mining
 - [BAD share #N: ... >>BAD,Incorrect difficulty]: Adjust difficulty from server. normal operation soon. 
 - [BAD share #N: ... >>BAD,Incorrect result]: Failed calulate.
 
![][link-duino_coin_web_process]

![][link-duino_coin_web_process_2]

<!--
Link
-->
[link-tera_term]: https://osdn.net/projects/ttssh2/releases/

[link-duino_coin_main]: https://duinocoin.com/getting-started.html
[link-duino_coin_wallet]: https://wallet.duinocoin.com/

[link-connect_to_doino_coin_url]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/connect_to_doino_coin_url.png
[link-duino_coin_hash_process]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/duino_coin_hash_process.png
[link-duino_coin_state_flow]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/duino_coin_state_flow.png
[link-duino_coin_web_main]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/duino_coin_web_main.png
[link-duino_coin_web_minimng_info]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/duino_coin_web_minimng_info.png
[link-duino_coin_web_process]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/duino_coin_web_process.png
[link-duino_coin_web_process_2]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/duino_coin_web_process_2.png


[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/Wiznet/RP2040-HAT-MINING-C/blob/main/static/images/duino_coin/connect_to_serial_com_port.png
