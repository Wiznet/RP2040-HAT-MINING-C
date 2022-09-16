#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#include "Wire.h"
#include "charbuffer.h"
#include "hwSetupUtils.h"
#include "duinoCoinUtils.h"

#define CMD_READ_STATUS     0x00
#define CMD_JOB_ALLOCATE    0x01
#define CMD_READ_RESULT     0x02
#define CMD_RESET           0x03
#define CMD_CFG_HASH_DELAY  0x10

CharBuffer receiveBuffer0, receiveBuffer1, requestBuffer;
CharBuffer configBuffer;

typedef enum _HashCalStatus {
    HASH_IDLE = '0',
    HASH_BUSY = '1',
    HASH_DONE = '2',
} HashCalStatus;

static HashCalStatus HashCalStatusCore0 = HASH_IDLE;
static HashCalStatus HashCalStatusCore1 = HASH_IDLE;

uint8_t TempBufferCore0[256];
uint8_t TempBufferCore1[256];

void software_reset()
{
    watchdog_enable(1, 1);
    while(1);
}

void slaveLoopCore1() {
    while(1){
        if(!receiveBuffer1.isEmpty() && receiveBuffer1.indexOf('\n') != -1){
            if(receiveBuffer1.length() == 256) {
                receiveBuffer1.clear();
            } else {
                memset(TempBufferCore1,0,256);
                HashCalStatusCore1 = HASH_BUSY;
                
                uint8_t lastblockhash[64] = "";
                uint8_t newblockhash[64] = "";
                uint8_t tempDifficulty[8] = "";


                uint32_t difficulty = 0;

                printf("Core1: job received: %s\n", receiveBuffer1.buf());
                receiveBuffer1.readStringUntil(',',(char *)lastblockhash);
                receiveBuffer1.readStringUntil(',',(char *)newblockhash);
                receiveBuffer1.readStringUntil('\n', (char *)tempDifficulty);
                difficulty = atoi((char *)tempDifficulty);

                if(difficulty == 0 || strlen((char *)lastblockhash) == 0 || strlen((char *)newblockhash) == 0){
                    printf("Core1: Something wrong with the data from the master.\n");
                    HashCalStatusCore1 = HASH_IDLE;
                    return;
                }

                printf("Core1: Last block hash is %s\nNew block hash is %s\nDifficulty is %d\n", lastblockhash, newblockhash, difficulty);
                
                uint32_t startTime = time_us_32();

                uint32_t result = calculateHashCore1(lastblockhash, newblockhash, difficulty);

                uint32_t endTime = time_us_32();

                uint32_t elapsed_time = endTime - startTime;//us

                elapsed_time = endTime - startTime;//us
 
                sprintf((char *)TempBufferCore1,"%d, %d\n", result, elapsed_time);

                printf("Core1: The hash result is %s\n",TempBufferCore1);
                HashCalStatusCore1 = HASH_DONE;
                outsideLedOn();
            }
        }
    }
}

//request callback
static void slave_on_request() {
    uint8_t value = requestBuffer.read();
    Wire1.write(value);//send requestBuffer
}

static void slave_on_receive(int count) {
    
    uint8_t command;
    uint8_t coreNumber;
    uint8_t hashDelayUs[8];
    hard_assert(Wire1.available());
    command = (uint8_t)Wire1.read();//first byte is command.
    switch (command){
case CMD_READ_STATUS://Command: read status
        /* code */
        while (Wire1.available()) { //clear the receive buffer
            Wire1.read();
        }
        requestBuffer.clear();
        requestBuffer.write(HashCalStatusCore0);//Core 0 status
        requestBuffer.write(HashCalStatusCore1);//Core 1 status
        // requestBuffer.write('\n');
        break;
    
    case CMD_JOB_ALLOCATE: //Command: job allocation
        coreNumber = Wire1.read();
        printf("coreNumber:%c\n",coreNumber);
        if(coreNumber == '0'){
            while (Wire1.available()){
                receiveBuffer0.write((uint8_t)Wire1.read());
            }
        } else {
            while (Wire1.available()){
                receiveBuffer1.write((uint8_t)Wire1.read());
            }
        }
        break;

    case CMD_READ_RESULT: //Command: Read result
        while (Wire1.available()) { //clear i2c receive buffer
            coreNumber = Wire1.read();
        }
        // printf("coreNumber: %d\n",coreNumber);
        if(coreNumber == 0){
            requestBuffer.setBuffer((char *)TempBufferCore0,256);//hash result + socket number
            HashCalStatusCore0 = HASH_IDLE;
            onBoardLedOff();
        } else if(coreNumber == 1){
            requestBuffer.setBuffer((char *)TempBufferCore1,256);//hash result + socket number
            HashCalStatusCore1 = HASH_IDLE;
            outsideLedOff();
        }
        else {}//do nothing!
        break;

    case CMD_RESET: //Command: reset
        software_reset();
        break;

    case CMD_CFG_HASH_DELAY: //Command: configure the hash delay
        while (Wire1.available()){
            configBuffer.write((uint8_t)Wire1.read());
        }
        configBuffer.readStringUntil('\n',(char*)hashDelayUs);
        setHashDelay(atoi((char*)hashDelayUs));
        break;

    default:
        while (Wire1.available()) { //clear receive buffer
            Wire1.read();
        }
        break;
    }
}

static void slaveLoop() {
    if(!receiveBuffer0.isEmpty() && receiveBuffer0.indexOf('\n') != -1)
    {
            if(receiveBuffer0.length() == 256) {
                receiveBuffer0.clear();
            }
            memset(TempBufferCore0,0,256);
            HashCalStatusCore0 = HASH_BUSY;
            
            uint8_t lastblockhash[64] = "";
            uint8_t newblockhash[64] = "";
            uint8_t tempDifficulty[8] = "";

            uint32_t difficulty = 0;

            printf("Core 0: Job received %s\n",receiveBuffer0.buf());
            receiveBuffer0.readStringUntil(',',(char *)lastblockhash);
            receiveBuffer0.readStringUntil(',',(char *)newblockhash);
            receiveBuffer0.readStringUntil('\n', (char *)tempDifficulty);

            difficulty = atoi((char *)tempDifficulty);

            if(difficulty == 0  || strlen((char *)lastblockhash) == 0 || strlen((char *)newblockhash) == 0){
                printf("Core0: Something wrong with the data from the master.\n");
                onBoardLedOff();
                HashCalStatusCore0 = HASH_IDLE;
                return;
            }

            printf("Core0: Last block hash is %s\nNew block hash is %s\nDifficulty is %d\n", lastblockhash, newblockhash, difficulty);
            
            uint32_t startTime = time_us_32();
            
            uint32_t result = calculateHashCore0(lastblockhash, newblockhash, difficulty);

            uint32_t endTime = time_us_32();

            uint32_t elapsed_time = endTime - startTime;//us

            sprintf((char *)TempBufferCore0,"%d, %d\n", result, elapsed_time);

            printf("Core0: The hash result is %s\n",TempBufferCore0);
            HashCalStatusCore0 = HASH_DONE;
            onBoardLedOn();

        }
}

int main() {

    stdio_init_all();
    
    hardwareInit();

    //initial delay
    sleep_ms(100);

    //i2c1 setup
    Wire1.onReceive(slave_on_receive);
    Wire1.onRequest(slave_on_request);

    uint8_t slaveAddress = getAddress();
    printf("I2C Slave starts with address %d\n",slaveAddress);
    Wire1.begin(slaveAddress);//i2c1 slave start

    multicore_launch_core1(slaveLoopCore1);

    sleep_ms(100);
    onBoardLedOff();
    outsideLedOff();
    while(1){
        slaveLoop();
    }
}