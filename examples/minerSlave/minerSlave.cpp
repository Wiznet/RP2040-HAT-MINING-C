#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "Wire.h"
#include "charbuffer.h"
#include "hwSetupUtils.h"
#include "duinoCoinUtils.h"

CharBuffer receiveBuffer0, receiveBuffer1, requestBuffer;

typedef enum _HashCalStatus {
    HASH_IDLE = '0',
    HASH_BUSY = '1',
    HASH_DONE = '2',
} HashCalStatus;

static HashCalStatus HashCalStatusCore0 = HASH_IDLE;
static HashCalStatus HashCalStatusCore1 = HASH_IDLE;

uint8_t TempBufferCore0[256];
uint8_t TempBufferCore1[256];

void slaveLoopCore1() {
    while(1){
        if(!receiveBuffer1.isEmpty() && receiveBuffer1.indexOf('\n') != -1){
            if(receiveBuffer1.length() == 256) {
                receiveBuffer1.clear();
            } else {
                memset(TempBufferCore1,0,256);
                HashCalStatusCore1 = HASH_BUSY;
                
                // uint8_t jobnumber[32] = "";// this should be number between 0 to 99
                uint8_t lastblockhash[64] = "";
                uint8_t newblockhash[64] = "";
                uint8_t tempDifficulty[8] = "";


                uint32_t difficulty = 0;
                // int jobNumber = -1;

                printf("slaveloopCore1:!!! %s", receiveBuffer1.buf());
                // receiveBuffer.readStringUntil(',',(char *)jobnumber);
                receiveBuffer1.readStringUntil(',',(char *)lastblockhash);
                receiveBuffer1.readStringUntil(',',(char *)newblockhash);
                receiveBuffer1.readStringUntil('\n', (char *)tempDifficulty);
                difficulty = atoi((char *)tempDifficulty);
                // jobNumber = atoi((char *)jobnumber);

                if(difficulty == 0 || strlen((char *)lastblockhash) == 0 || strlen((char *)newblockhash) == 0){
                    printf("From Core1: Something wrong with the data from the master.\n");
                    HashCalStatusCore1 = HASH_IDLE;
                    return;
                }

                printf("From Core1: Last block hash is %s\nNew block hash is %s\nDifficulty is %d\n", lastblockhash, newblockhash, difficulty);
                
                uint32_t startTime = time_us_32();
                //while hash calcuion on core 1, it's not able to assign a hash job to core 1.
                uint32_t result = calculateHashCore1(lastblockhash, newblockhash, difficulty);

                uint32_t endTime = time_us_32();

                uint32_t elapsed_time = endTime - startTime;//us

                elapsed_time = endTime - startTime;//us
                // printf("Core1: endTime: %d, startTime: %d\n",time_us_64(),endTime);
                //hash rate needs to be calculated.
                //Update Buffer
                //만들자,
                sprintf((char *)TempBufferCore1,"%d, %d\n", result, elapsed_time);
                // sprintf((char *)TempBufferCore0,"%s,%s,%s\n", jobnumber, result, elapsed_time);
                // uint32_t result = 0;
                printf("Form Core1: The hash result is %s\n",TempBufferCore1);
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
    hard_assert(Wire1.available());
    command = (uint8_t)Wire1.read();//first byte is command.
    // printf("command:%02x\n",command);
    switch (command){
    case 0x00: //read if it's available?
        /* code */
        while (Wire1.available()) { //clear the receive buffer
            Wire1.read();
        }
        requestBuffer.clear();
        requestBuffer.write(HashCalStatusCore0);//Core 0 status
        requestBuffer.write(HashCalStatusCore1);//Core 1 status
        // requestBuffer.write('\n');
        break;
    
    case 0x01: //job allocation
        coreNumber = Wire1.read();
        // printf("coreNumber:%c",coreNumber);
        if(coreNumber == '0'){
            while (Wire1.available()){
                receiveBuffer0.write((uint8_t)Wire1.read());
            }
        } else {
            while (Wire1.available()){
                receiveBuffer1.write((uint8_t)Wire1.read());
            }
        }
        
        // if(receiveBuffer.read() == '1'){
        //     Core1Trigger = 1;
        // } else {
        //     Core0Trigger = 1;
        // }
        // printf("Core1Trigger: %d, %d\n",Core1Trigger, receiveBuffer.indexOf('\n'));
        break;

    case 0x02: //Read result
        /* code */
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
            // printf("requestBuffer:%s",requestBuffer.buf());//여기는 잘 나옴.
            HashCalStatusCore1 = HASH_IDLE;
            outsideLedOff();
        }
        else {}//do nothing!}
        break;

        case 0x03: // Read Done
        while (Wire1.available()) { //clear i2c receive buffer
            Wire1.read();
        }
        //do nothing yet...
        //todo
        break;

    default:
        while (Wire1.available()) { //clear the receive buffer
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
            
            // uint8_t jobnumber[32] = "";// this should be number between 0 to 99
            uint8_t lastblockhash[64] = "";
            uint8_t newblockhash[64] = "";
            uint8_t tempDifficulty[8] = "";

            // int jobNumber = -1;
            uint32_t difficulty = 0;

            printf("slaveloopCore0:!!! %s",receiveBuffer0.buf());
            // receiveBuffer.readStringUntil(',',(char *)jobnumber);
            receiveBuffer0.readStringUntil(',',(char *)lastblockhash);
            receiveBuffer0.readStringUntil(',',(char *)newblockhash);
            receiveBuffer0.readStringUntil('\n', (char *)tempDifficulty);

            difficulty = atoi((char *)tempDifficulty);
            // jobNumber = atoi((char *)jobnumber);

            if(difficulty == 0 || strlen((char *)lastblockhash) == 0 || strlen((char *)newblockhash) == 0){
                printf("From Core0: Something wrong with the data from the master.\n");
                HashCalStatusCore0 = HASH_IDLE;
                return;
            }

            printf("From Core0: Last block hash is %s\nNew block hash is %s\nDifficulty is %d\n", lastblockhash, newblockhash, difficulty);
            
            uint32_t startTime = time_us_32();
            //while hash calcuion on core 0, it's not able to assign a hash job to core 1.
            uint32_t result = calculateHashCore0(lastblockhash, newblockhash, difficulty);

            uint32_t endTime = time_us_32();

            uint32_t elapsed_time = endTime - startTime;//us
            // printf("Core0: endTime: %d, startTime: %d\n", endTime, startTime);
            //hash rate needs to be calculated.
            //Update Buffer
            //만들자,
            sprintf((char *)TempBufferCore0,"%d,%d\n", result, elapsed_time);
            // sprintf((char *)TempBufferCore0,"%s,%s,%s\n", jobnumber, result, elapsed_time);
            // uint32_t result = 0;
            printf("Form Core0: The hash result is %s\n",TempBufferCore0);
            HashCalStatusCore0 = HASH_DONE;
            onBoardLedOn();
        }
}

int main() {
    // bi_decl(bi_program_description("Binary f Por the slave of the duino coin mining example. WIZnet Co,.Ltd"));
    // bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED")); 

    stdio_init_all();
    
    hardwareInit();

    //initial delay
    sleep_ms(100);

    //i2c1 setup
    Wire1.onReceive(slave_on_receive);
    Wire1.onRequest(slave_on_request);
    // // in this implementation, the user is responsible for initializing the I2C instance and GPIO
    // // pins before calling Wire::begin()
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