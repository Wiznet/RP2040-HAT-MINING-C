#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "port_common.h"
#include "pico/multicore.h"
#include "utils/inc/hwSetupUtils.h"
#include "utils/inc/charbuffer.h"
#include "utils/inc/Wire.h"

#include "wizchip_conf.h"

#include "timer.h"
#include "dhcp.h"
#include "socket.h"
#include "core_json.h"
#include "httpParser.h"

#include "ILI9340/ILI9340.h"


#ifdef __cplusplus
extern "C" {
#endif
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

/* Timer*/
#define US_TIMER_MAX            0xFFFF0000 //4295000000 //1 hour 11 minutes and 35 seconds.
#define US_TIMER_WAIT_TIMEOUT   5000000 //5s
#define US_TIMER_KEEPALIVE      10000000 //10s
#define FAIL_COUNT 5

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_MACRAW 	0
#define SOCKET_DHCP 	1
#define SOCKET_HTTP 	2

#define SOCKET_MAX 20

/* Port */
#define PORT_SSL 443

/* Duino coin information for the mining*/
// User name : input yours
#define DUINO_USER_NAME         "user name"
// Miner key : input yours
#define DUINO_MINER_KEY         "miner key"
//RIG_Identifier : Anything can be used for here
#define DUINO_RIG_IDENTIFIER    "WIZnet_Miner_" 
//Miner software version
#define DUINO_MINER_BANNER      "W5100S-EVB-Pico Miner v1.0"
//Duino Platform name to be used when requesting hash job
//This changes the hash difficulty.
//ESP8266NH: 3500
//ESP32: 1250
//AVR: 450
#define DUINO_PLATFORM  "ESP8266NH"

/* duino coin server url to get pool information*/
#define DUINO_HTTP_GET_URL      "https://server.duinocoin.com/getPool"

#define DUINO_MSG_LEN_HASH  40
#define DUINO_MSG_LEN_TOTAL 90

#define DUNO_DEBUG_MSG    1
/* Multicore*/
#define USE_DUALCORE
#define HASH_START          0
#define HASH_END            1
#define HASH_FINDING        0
#define HASH_NOT_FIND       1
#define HASH_RATE_DELAY_US  0

//Slave hash delay in us
#define HASH_DELAY  "0\n"

/* Duco miner */
#define BUF_SIZE        256
#define MAX_NUM_SLAVE   10

/* Commands for slaves*/
#define CMD_STATUS      0x00
#define CMD_WRITE_JOB   0x01
#define CMD_READ_RESULT 0x02
#define CMD_RESET       0x55
#define CMD_CONFIG_HASH_DELAY   0x10

/* WatchDog Timer */
#define WATCHDOG_TIMER_MS 60000

typedef enum {
    UpdatePool = 0,
    ConServer,
    Wait_ConServer,
    GetHash,
    WaitRecvHash,
    RecvHash,
    CalHash,
    SendHash,
    WaitResult,
    GetResult
}Loop_state;

 /* Duino client*/
typedef enum _duinoClientState{
    CONNECT = 0,
    CONNECT_WAIT,
    JOB_REQUEST,
    REQUEST_WAIT,
    HASH_CALCULATION,
    CALCULATION_WAIT,
    SEND_RESULT,
    RESULT_WAIT,
    RESULT_UPDATE,
    SLAVE_FAULT,
} duinoClientState;

/* LCD */
typedef struct Duino_lcd_info_t {
    char  slave_index[LCD_STRING_SIZE_SLAVE_INDEX];
    char  result[LCD_STRING_SIZE_RESULT];
    char  hashrate[LCD_STRING_SIZE_HASH_RATE];
    char  elapsed_time_s[LCD_STRING_SIZE_ELAP_TIME];
    char  difficulty[LCD_STRING_SIZE_DIFFICULTY];
} Duino_lcd_info;

/* Default W5100S Network information*/
static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0xff, 0xff, 0x18}, // MAC address
    .ip =  {192, 168, 11, 2},                     // IP address
    .sn =  {255, 255, 255, 0},                    // Subnet Mask
    .gw =  {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_DHCP                       // DHCP enable/disable
};

//buffer size for the duino coin client
#define SOCK_BUF_SIZE 128
//Server response timeout in sec
#define SERVER_RESPONSE_TIMEOUT 20
//Server connect fail counter
#define SERVER_DISCONNECT_COUNTER 10
//Slave calculation timeout in sec
#define SLAVE_CALCULATION_TIMEOUT 30

//duino coin client structure
typedef struct _ducoClient {
    struct tcp_pcb *tcp_client_pcb;
    uint8_t socketBuffer[SOCK_BUF_SIZE];
    uint8_t socketNumber;
    uint8_t slaveAddress;
    uint64_t timeOut;
    duinoClientState state;
    uint8_t lastBlockHash[41];
    uint64_t startTime;
    uint64_t endTime;
    float hashrate;
    int hashResult;
    float elapsedTime;
    uint32_t diff;
    uint8_t retryCounter;   //to slave
    uint8_t disconnCounter; // to server
    int8_t wait_open_time;  // start time+ client number, min
} DucoClient;

int8_t g_tcp_connect_retry = SERVER_DISCONNECT_COUNTER; 
static DucoClient ducoClient[SOCKET_MAX];

static uint8_t * g_ethernet_buf;

/* LWIP */
struct netif g_netif;

/* TCP */
static ip_addr_t g_tcp_client_target_ip;
u16_t g_tcp_client_target_port= 0;

uint8_t pack [ETHERNET_MTU];
uint8_t pack2 [ETHERNET_MTU];

/* Http */
static tlsContext_t g_http_tls_context;

/* duino hash*/
static char send_req_str[128];

/* For I2C Slave*/
uint8_t g_duino_slave[10];
uint8_t g_slave_job[BUF_SIZE];
uint32_t g_slave_count;

// I2C Buffer
CharBuffer receiveBuffer, jobBuffer;

/* LCD */
static Duino_lcd_info g_duino_lcd_info[LCD_DUINO_INFO_MEM_ARR_SIZE];

int8_t g_lcd_newest_mem_index= 0;
int8_t g_lcd_printed_line_cnt= 1;

int8_t g_start_min = -1;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* I2C Master*/
uint32_t writeCmd(uint8_t slaveAddress, uint8_t cmd, uint8_t * follow, uint32_t len);
uint32_t readData(uint8_t slaveAddress, uint32_t len);
uint32_t busScan(uint8_t * slaveAddressArray);
bool reserved_addr(uint8_t addr);

/* Duino*/
void update_pool(void);
void set_duino_req_msg (void);
void duino_server_state_loop(void);
void duino_master_init(void);
void duino_master_state_loop (void); 
static void set_duino_res_msg(uint8_t * socketBuffer, uint32_t duco_numeric_result, float hashrate,uint8_t slaveAddress, uint8_t slaveNumber);
/*DHCP*/
void wizchip_net_info_set(void);
void wizchip_dhcp_init(void);

/* LWIP */
int tcp_find_socket(tcp_pcb *tpcb);
static err_t tcp_callback_connected(void *arg, struct tcp_pcb *pcb_new, err_t err);
static err_t tcp_callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_callback_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_callback_poll(void *arg, struct tcp_pcb *tpcb);
static void tcp_callback_error(void *arg, err_t err);
static void tcp_client_close(struct tcp_pcb *tpcb);

/* Multicore */
void core1_entry(void);

/* RTC Timer */
void duino_print_rtc_log (datetime_t t);
void duino_set_start_time (void);

/* LCD */
void set_lcd_print_info(int8_t mem_index, int8_t slave_indnex, float elapsed_time_s, float hashrate, uint32_t difficulty, char* result);
void set_lcd_print_log(void);


int main() 
{

    int ret;
    err_t err;
    
   // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2022,
            .month = 01,
            .day   = 01,
            .dotw  = 0, // 0 is Sunday, so 5 is Friday
            .hour  = 1,
            .min   = 1,
            .sec   = 01
    };

    // Start the RTC

    set_clock_khz();
    stdio_init_all();
 
    wizchip_delay_ms(1000 * 3); // wait for 3 seconds
    g_ethernet_buf = (uint8_t*)malloc(2048);
    
    hardwareInit();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();
    wizchip_1ms_timer_initialize(repeating_timer_callback);

    ILI9340_init();
    wizchip_net_info_set();
    //get the pool ip
    update_pool();

    free(g_ethernet_buf);
    //Change the mode for the W5100S to MACRAW
    wizchip_lwip_init(SOCKET_MACRAW, &g_netif, &g_net_info);
    set_duino_req_msg();
#ifdef USE_DUALCORE
    //Core1 launch for the lwip tcp/ip stack
    multicore_launch_core1(core1_entry);
#endif
    duino_master_init();
    duino_set_start_time();

    rtc_init();
    rtc_set_datetime(&t);
    watchdog_enable(WATCHDOG_TIMER_MS, true);

    while(1)
    {
        duino_master_state_loop();

        DHCP_run();
        rtc_get_datetime(&t);
        duino_print_rtc_log(t);
        watchdog_update();
    } 
}

/* DHCP */
static void wizchip_dhcp_assign(void){
    getIPfromDHCP(g_net_info.ip);
    getGWfromDHCP(g_net_info.gw);
    getSNfromDHCP(g_net_info.sn);
    getDNSfromDHCP(g_net_info.dns);

    g_net_info.dhcp = NETINFO_DHCP;

    /* Network initialize */
    network_initialize(g_net_info);

    print_network_information(g_net_info);
    printf(" DHCP leased time : %ld seconds\n", getDHCPLeasetime());
}

static void wizchip_dhcp_conflict(void){
    printf(" Conflict IP from DHCP\n");

    // halt or reset or any...
    while (1); // this example is halt.
}

void wizchip_dhcp_init(void){
    printf("\n DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}

/* I2C master bus scan */
uint32_t busScan(uint8_t * slaveAddressArray){
    uint8_t slaveCount = 0;
    int8_t ret = 0;
    uint8_t dummy;
    for(int addr = 0 ; addr < (1 << 7); ++addr){
        if (reserved_addr(addr)) {
            ret = PICO_ERROR_GENERIC;
        }
        else{
            ret = i2c_read_blocking(i2c1, addr, &dummy, 1, false);
        }

        if(ret > 0){
            slaveAddressArray[slaveCount++] = addr;
        }

        if(slaveCount > 10){
            printf("maximum number of slaves are 10\n");
            return 10;
        }
    }
    return slaveCount;
}

//Some I2C Addresses are resereved for the special uses
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

//I2C write command
uint32_t writeCmd(uint8_t slaveAddress, uint8_t cmd, uint8_t * follow, uint32_t len){
    uint32_t count;
    Wire1.beginTransmission(slaveAddress);
    //write command
    Wire1.write(cmd);
    //write follow
    if(follow)
        Wire1.write((const uint8_t *)follow, len);
    count = Wire1.endTransmission(true);
    return count;
}

//I2C read data
uint32_t readData(uint8_t slaveAddress, uint32_t len){
    uint32_t count;
    uint8_t status;
    count = Wire1.requestFrom(slaveAddress,len,true);// Status: Core0 | Core1 | '\n'
    while(Wire1.available()){
        receiveBuffer.write((uint8_t)Wire1.read());
    }
    return receiveBuffer.length();
}

//I2C master init
//i2c bus scan -> store slave's information -> set the slave's hash delay
void duino_master_init (void) {
    Wire1.begin();
    
    printf("I2C Bus scanning..\n");
    g_slave_count= busScan(g_duino_slave);       
    printf("%d of slaves are detected.\n",g_slave_count);
    
    for(int i = 0 ; i < g_slave_count ; i++)
        printf("Detected slave address[%d]: %d\n",i, g_duino_slave[i]);
    
    for(int i = 0 ; i < g_slave_count ; i++){
        writeCmd(g_duino_slave[i],CMD_CONFIG_HASH_DELAY,(uint8_t *)HASH_DELAY, sizeof(HASH_DELAY));
        sleep_ms(10);
    }
    printf("Slave hash delay has been set to %s,%d\n",HASH_DELAY,sizeof(HASH_DELAY));
}

uint64_t timeOut(uint32_t sec){
    return time_us_64() + sec*1000000;
}

void duino_set_start_time (void) {

    if (watchdog_caused_reboot()) {
        printf(" >> Rebooted by Watchdog! restart wait 3 min\n");
        for (int i=0 ; i<SOCKET_MAX; i++)
            ducoClient[i].wait_open_time=((i/2) +3);
    }
    else{
        for (int i=0 ; i<SOCKET_MAX; i++)
            ducoClient[i].wait_open_time=i/2;
    }
}

void duino_print_rtc_log (datetime_t t) {
    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];

    if (t.min != g_start_min) {
        g_start_min=t.min;
        datetime_to_str(datetime_str, sizeof(datetime_buf), &t);
        printf("\r----------------  RTC Time (min): %s  ----------------\r\n", datetime_str);
    }

}

void init_client_open_timeout (uint8_t clinet_num) {
    ducoClient[clinet_num].wait_open_time= -1;
}

void set_client_connect_timeout (int8_t clinet_num, int add_min) {
    int8_t min;
    if (add_min<0) 
        min= g_start_min + clinet_num;
    else 
        min= g_start_min + add_min;

    if (min>= 60) min-=60;
    printf(" >set open timer%d: %d->%d \n", clinet_num, g_start_min, min);
    ducoClient[clinet_num].wait_open_time= min;
    return;
}

void duino_master_state_loop (void) {//i2c whlie
    uint8_t tempBuffer[64] = "";
    uint8_t core[2];

    uint8_t slaveNumber = 0;
    uint8_t coreNumber = 0;
    err_t err;

    bool success;

    for(int clientNo = 0; clientNo < g_slave_count * 2; clientNo++) {//0 ~ 19
        slaveNumber = clientNo/2;// Slave Number
        coreNumber = clientNo%2;// 0 or 1 => core0 or core1
        switch (ducoClient[clientNo].state)
        {
        case CONNECT:
            if(ducoClient[clientNo].wait_open_time != g_start_min && ducoClient[clientNo].wait_open_time != -1){
                break;
            }

            if(ducoClient[clientNo].tcp_client_pcb != NULL)
                tcp_client_close(ducoClient[clientNo].tcp_client_pcb);
            ducoClient[clientNo].tcp_client_pcb = tcp_new();

            if (ducoClient[clientNo].tcp_client_pcb == NULL){                    
                g_tcp_connect_retry--;  
                if (g_tcp_connect_retry< 0){   
                    printf("create socket fail: %d, wait watchdog reboot. reset after (%d ms)\n", clientNo, WATCHDOG_TIMER_MS);          
                    for(int i=0; i<SOCKET_MAX; i++){
                        tcp_close(ducoClient[i].tcp_client_pcb); // close connection
                    }
                    while (1); //reset by watchdog
                }   
                break;
            }
            
            printf("Client %d: CONNECT\n", clientNo);
            ducoClient[clientNo].elapsedTime = 0.0;
            ducoClient[clientNo].hashrate    = 0.0;
            ducoClient[clientNo].hashResult  = 0;
            
            err = tcp_connect(ducoClient[clientNo].tcp_client_pcb, &g_tcp_client_target_ip, g_tcp_client_target_port, tcp_callback_connected);
            if(err == ERR_OK){
                init_client_open_timeout(clientNo);
                g_tcp_connect_retry = SERVER_DISCONNECT_COUNTER; //connect to server is ok, reflash counter
                ducoClient[clientNo].timeOut = timeOut(SERVER_RESPONSE_TIMEOUT);//10 second
                ducoClient[clientNo].state = CONNECT_WAIT;
            } 
            else{//connect fail             
                g_tcp_connect_retry --;
                if (g_tcp_connect_retry< 0){   
                    printf("create socket fail: %d, wait watchdog reboot. reset after (%d ms)\n", clientNo, WATCHDOG_TIMER_MS);          
                    for(int i=0; i<SOCKET_MAX; i++){
                        tcp_close(ducoClient[i].tcp_client_pcb); // close connection
                    }
                    while (1); //reset by watchdog
                }   
                tcp_client_close(ducoClient[clientNo].tcp_client_pcb);//close socket and
                set_client_connect_timeout(clientNo, 2); //retry after 2 min
                printf("connect fail: %d, connect time out: ->%d/ retry cnt: %d\n", clientNo, ducoClient[clientNo].wait_open_time, g_tcp_connect_retry); 
                ducoClient[clientNo].state = CONNECT;//try to connect again
            }
 
            break;
        case CONNECT_WAIT:
            if(ducoClient[clientNo].timeOut < time_us_64()){//timeout
                printf("Client %d: CONNECT_WAIT timeout.\n", clientNo);
                tcp_client_close(ducoClient[clientNo].tcp_client_pcb);//close socket and 
                set_client_connect_timeout(clientNo, 1); //retry after (clientNo) min
                ducoClient[clientNo].state = CONNECT;//try to connect again
            }   
            break;
        case JOB_REQUEST:
            printf("Client %d: JOB_REQUEST\n", clientNo);
            //check status
            writeCmd(g_duino_slave[slaveNumber],CMD_STATUS,NULL,0);//read status command
            sleep_ms(10);
            readData(g_duino_slave[slaveNumber], 128);//read status data
            
            core[0] = receiveBuffer.read();
            core[1] = receiveBuffer.read();
            receiveBuffer.clear();

            if(core[coreNumber] == '0'){// IDLE
#ifdef DUNO_DEBUG_MSG            
                printf("Job request string: %s\n",send_req_str);
#endif                
                err = tcp_write((tcp_pcb *)ducoClient[clientNo].tcp_client_pcb, send_req_str, strlen(send_req_str), 0);
                if(err != ERR_OK){              
                    printf("tcp_write failed: %d\n",clientNo);
                    tcp_client_close(ducoClient[clientNo].tcp_client_pcb);//close socket and       
                    set_client_connect_timeout(clientNo, 1); //retry after 1 min 
                    ducoClient[clientNo].state = CONNECT;
                    break;
                }
                ducoClient[clientNo].timeOut = timeOut(SERVER_RESPONSE_TIMEOUT);//10 second
                ducoClient[clientNo].state = REQUEST_WAIT;
            } 
            else if (core[coreNumber] == '1'){//BUSY
                printf("Client %d: busy. Socket state: %d\n",clientNo, ducoClient[clientNo].tcp_client_pcb->state);
                //do nothing
            } 
            else if(core[coreNumber] == '2'){//DONE
                printf("Client %d: hash done. Socket state: %d\n",clientNo, ducoClient[clientNo].tcp_client_pcb->state);
                ducoClient[clientNo].state = SEND_RESULT;//read out
            } 
            else { // SOMETHING WRONG!
                if(ducoClient[clientNo].retryCounter < 10){
                    printf("Client %d: went wrong %02x retry %d.\n",clientNo,core[coreNumber],ducoClient[clientNo].retryCounter);
                    ducoClient[clientNo].state = JOB_REQUEST;//retry
                    ducoClient[clientNo].retryCounter++;
                } 
                else {
                    printf("Client %d: slave %d error. SLAVE_FAULT!! \n", clientNo, slaveNumber);
                    ducoClient[clientNo].state = SLAVE_FAULT;//retry
                }
            }
            break;
        case REQUEST_WAIT:
            ducoClient[clientNo].disconnCounter = 2;
            if(ducoClient[clientNo].timeOut < time_us_64()){//time out
                printf("Client %d: REQUEST_WAIT timeout, retry cnt:%d\n", clientNo, ducoClient[clientNo].retryCounter);
                if (ducoClient[clientNo].disconnCounter) {
                    ducoClient[clientNo].disconnCounter --;
                    ducoClient[clientNo].state = JOB_REQUEST;
                }                    
                else {
                    ducoClient[clientNo].disconnCounter = 2;
                    set_client_connect_timeout(clientNo, 1); //retry after client/2 min 
                    ducoClient[clientNo].state = CONNECT;
                }
            }   
            break;
        case HASH_CALCULATION:{
            ducoClient[clientNo].startTime = time_us_64();
            printf("Client %d: HASH_CALCULATION\n",clientNo);
            sleep_ms(10);
            g_slave_job[0] = (uint8_t)coreNumber;
            memset(tempBuffer,0,64);
            //To get the hash difficulty from the job message
            jobBuffer.setBuffer((char*)ducoClient[clientNo].socketBuffer,strlen((char*)ducoClient[clientNo].socketBuffer));
            jobBuffer.readStringUntil(',',(char*)tempBuffer);//Last Block Hash => dummy
            if(strcmp((char*)tempBuffer, "BAD") == 0) { //if server response is "BAD" or the same block hash received.
                printf("Client %d: wrong job string. Retry to get the job.\n", clientNo);
                ducoClient[clientNo].state = JOB_REQUEST;//send job request again
                break;
            }
            memcpy(ducoClient[clientNo].lastBlockHash, tempBuffer, strlen((char*)tempBuffer));
        
            memset(tempBuffer,0,64);
            jobBuffer.readStringUntil(',',(char*)tempBuffer);//New Block Hash => dummy
            //last block hash update

            //Read the hash difficulty
            memset(tempBuffer,0,64);
            jobBuffer.readStringUntil('\n',(char*)tempBuffer);
            ducoClient[clientNo].diff = atoi((char*)tempBuffer);

            //add the core number in front of the job string for the slave hash calculation.
            sprintf((char*)g_slave_job, "%d%s", coreNumber, (char*)ducoClient[clientNo].socketBuffer);
            //send the write hash command and the job string
            writeCmd(g_duino_slave[slaveNumber],CMD_WRITE_JOB, (uint8_t *)g_slave_job, strlen((char*)g_slave_job));
#ifdef DUNO_DEBUG_MSG
            printf("Client %d: job allocated => %s\n",clientNo, g_slave_job);
#endif            
            ducoClient[clientNo].state = CALCULATION_WAIT;
            ducoClient[clientNo].timeOut = timeOut(SLAVE_CALCULATION_TIMEOUT);
            break;
        }
        case CALCULATION_WAIT:
            //check status
            writeCmd(g_duino_slave[slaveNumber],CMD_STATUS,NULL,0);//send command : status read
            sleep_ms(10);
            readData(g_duino_slave[slaveNumber], 128);//read data: status
            
            core[0] = receiveBuffer.read();
            core[1] = receiveBuffer.read();
            receiveBuffer.clear();

            if(core[coreNumber] == '2'){//HASH DONE
                ducoClient[clientNo].state = SEND_RESULT;
                break;
            }
            //if timeout
            if(ducoClient[clientNo].timeOut < time_us_64()) {//time out
                printf("Client %d: CALCULATION_WAIT timeout\n", clientNo);
                ducoClient[clientNo].timeOut = timeOut(SLAVE_CALCULATION_TIMEOUT);//timer update
            }
            break;
        case SEND_RESULT:
            ducoClient[clientNo].retryCounter = 0;
            printf("Client %d: SEND_RESULT\n",clientNo);
            //send command: read result
            writeCmd(g_duino_slave[slaveNumber],CMD_READ_RESULT,&coreNumber,1);
            sleep_ms(10);
            //read data: hash result
            readData(g_duino_slave[slaveNumber],128);
#ifdef DUNO_DEBUG_MSG            
            printf("Client %d: result => %s\n", clientNo, receiveBuffer.buf());
#endif
            //read the hash result from the i2c buffer
            memset(tempBuffer,0x00,64);
            receiveBuffer.readStringUntil(',',(char *)tempBuffer);
            ducoClient[clientNo].hashResult = atoi((char *)tempBuffer);

            //read the elapsed time from the i2c buffer
            //but this is meanningless
            memset(tempBuffer,0x00,64);
            receiveBuffer.readStringUntil('\n',(char *)tempBuffer);
            ducoClient[clientNo].elapsedTime = atoi((char *)tempBuffer) / 1000000.0;
            ducoClient[clientNo].hashrate = ducoClient[clientNo].hashResult/ducoClient[clientNo].elapsedTime;
            
            //clear the i2c receive buffer
            receiveBuffer.clear();

            if(ducoClient[clientNo].hashResult == 0){
                if(ducoClient[clientNo].retryCounter == 2){//retry to get the hash result twice
                    printf("Client %d: wrong result\n",clientNo);
                    ducoClient[clientNo].retryCounter = 0;
                    set_client_connect_timeout(clientNo, 1); //retry after 1 min 
                    ducoClient[clientNo].state = CONNECT; //something went wrong(i2c, lwip)
                    break;
                }
                ducoClient[clientNo].retryCounter++;
                break;
            }
            ducoClient[clientNo].retryCounter = 0;
            ducoClient[clientNo].endTime = time_us_64();
            ducoClient[clientNo].elapsedTime = (ducoClient[clientNo].endTime - ducoClient[clientNo].startTime) / 1000000.0;
            ducoClient[clientNo].hashrate = ducoClient[clientNo].hashResult/ducoClient[clientNo].elapsedTime;

            memset(ducoClient[clientNo].socketBuffer,0,128);
            set_duino_res_msg(( uint8_t *)ducoClient[clientNo].socketBuffer, ducoClient[clientNo].hashResult, ducoClient[clientNo].hashrate, g_duino_slave[slaveNumber], clientNo);
#ifdef DUNO_DEBUG_MSG            
            printf("Client %d: message '%s' will be sent.\n",clientNo, ducoClient[clientNo].socketBuffer);
#endif            
            if(ducoClient[clientNo].tcp_client_pcb->state != ESTABLISHED) {//if socket state is not in ESTABLISHED
                ducoClient[clientNo].state = CONNECT;//try to connect again
                break;
            }
            err = tcp_write((tcp_pcb *)ducoClient[clientNo].tcp_client_pcb, ( uint8_t *)ducoClient[clientNo].socketBuffer, strlen((char *)ducoClient[clientNo].socketBuffer), 0); //Getting hash job
            if(err < 0) {
                printf("SEND_RESULT write Error:%02x\n",err);
                tcp_client_close(ducoClient[clientNo].tcp_client_pcb);//close socket and 
                set_client_connect_timeout(clientNo, 1); //retry after 
                ducoClient[clientNo].state = CONNECT;
                break;
            }
            ducoClient[clientNo].timeOut = timeOut(SERVER_RESPONSE_TIMEOUT);//10 second
            ducoClient[clientNo].state = RESULT_WAIT;
            break;
        case RESULT_WAIT:
            if(ducoClient[clientNo].timeOut < time_us_64()){//time out
                printf("Client %d: RESULT_WAIT timeout.\n", clientNo);
                set_client_connect_timeout(clientNo, 1); //retry after client/1 min 
                ducoClient[clientNo].state = CONNECT;//try to reconnect again
            }  
            break;
        case RESULT_UPDATE:
            ducoClient[clientNo].state = JOB_REQUEST;
            printf("Client %d: %s",clientNo, ducoClient[clientNo].socketBuffer);
            if (0 == strncmp((char*)ducoClient[clientNo].socketBuffer, "GOOD", 4))  {
#ifdef DUNO_DEBUG_MSG                
                printf("Client %d: GOOD, %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", clientNo, ducoClient[clientNo].hashResult, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].elapsedTime);
#endif                
                set_lcd_print_info(g_lcd_newest_mem_index, clientNo, ducoClient[clientNo].elapsedTime, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].diff, "GOOD");
                set_lcd_print_log();
            }
            else if (0 == strncmp((char*)ducoClient[clientNo].socketBuffer, "BAD", 3)) {
#ifdef DUNO_DEBUG_MSG                
                printf("Client %d: BAD, %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", clientNo, ducoClient[clientNo].hashResult, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].elapsedTime);                    
#endif                
                set_lcd_print_info(g_lcd_newest_mem_index, clientNo, ducoClient[clientNo].elapsedTime, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].diff, "BAD");
                set_lcd_print_log();
            }
            break;
        
        case SLAVE_FAULT:
            printf("Fault.\n");
            break;

        default:
            break;
        }
    }
}

/*Core 1 loop function for the lwip tcp/ip stack*/
void core1_entry(void){
    printf("core1_entry: wizchip_lwip_process\r\n");
    while(1)
        wizchip_lwip_process(SOCKET_MACRAW, &g_netif, pack2);
}

/* connect to duino server */
void set_duino_req_msg (void){
    memset(send_req_str, 0x00, 128);
    sprintf(send_req_str, "JOB,%s,%s,%s\n\0", DUINO_USER_NAME, DUINO_PLATFORM, DUINO_MINER_KEY);
}

static void set_duino_res_msg(uint8_t * socketBuffer, uint32_t duco_numeric_result, float hashrate, uint8_t slaveAddress, uint8_t slaveNumber){
    memset(socketBuffer, 0x00, SOCK_BUF_SIZE );
    // sprintf((char*)socketBuffer, "%d,%0.2f,%s,%s%02d_%02d,DUCOID%02d%02d\n", duco_numeric_result, hashrate, DUINO_MINER_BANNER, DUINO_RIG_IDENTIFIER,slaveAddress,slaveNumber,slaveAddress,slaveNumber);
    // the hash rate doesn't need to be sent.
    sprintf((char*)socketBuffer, "%d,,%s,%s%02d_%02d,DUCOID%02d%02d\n", duco_numeric_result, /*hashrate,*/ DUINO_MINER_BANNER, DUINO_RIG_IDENTIFIER,slaveAddress,slaveNumber,slaveAddress,slaveNumber);
}

//set the W5100S network information
void wizchip_net_info_set(void){
    int ret;
    if (g_net_info.dhcp == NETINFO_DHCP){
        wizchip_dhcp_init();

        while (1){
            ret = DHCP_run();
            if (ret == DHCP_IP_LEASED)
                break;
            wizchip_delay_ms(1000);
        }
    }
    else {
        network_initialize(g_net_info);
        print_network_information(g_net_info);
    }
}


/* Lwip */
int tcp_find_socket(tcp_pcb *tpcb){
    int cnt;
    
    for (cnt=0; cnt < SOCKET_MAX; cnt++){
        if (ducoClient[cnt].tcp_client_pcb == tpcb)
            break;
    }
    if (cnt == SOCKET_MAX) 
        cnt = -1;
    
    return cnt;
}
     
static err_t tcp_callback_connected(void *arg, struct tcp_pcb *pcb_new, err_t err){
    LWIP_UNUSED_ARG(arg);

    if (err != ERR_OK) //error when connect to the server
        return err;
    
    tcp_setprio(pcb_new, TCP_PRIO_NORMAL); //set priority for the client pcb

    tcp_arg (pcb_new, 0); //no argument is used
    tcp_err (pcb_new, tcp_callback_error); //register error callback
    tcp_sent(pcb_new, tcp_callback_sent); //register send callback
    tcp_recv(pcb_new, tcp_callback_received);  //register receive callback
    tcp_poll(pcb_new, tcp_callback_poll, 0); //register poll callback

    return ERR_OK;
}

static err_t tcp_callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len){
    uint8_t socketNumber = tcp_find_socket(tpcb);

    LWIP_UNUSED_ARG(arg);//?
    LWIP_UNUSED_ARG(tpcb);//?
    LWIP_UNUSED_ARG(len);//?

    printf("Client %d: Send OK\r\n",socketNumber);

  return ERR_OK;
}

static err_t tcp_callback_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err){
    int i;
    err_t ret_err;
    int socket_number;
    socket_number = tcp_find_socket(tpcb);

    if (p == NULL) {//pbuf is null when session is closed
        tcp_client_close(tpcb);
        printf("Client %d: socket closed => 0x%x, 0x%x\n",socket_number, tpcb, ducoClient[socket_number].tcp_client_pcb);
        ret_err = ERR_OK;
    }
    else if (err != ERR_OK) {//ERR_ABRT is returned when called tcp_abort
        printf("Client %d: socket aborted => 0x%x, 0x%x\n",socket_number, tpcb, ducoClient[socket_number].tcp_client_pcb);
        tcp_recved(tpcb, p->tot_len); //advertise window size
        tcp_client_close(tpcb);
        pbuf_free(p); //free pbuf

        ducoClient[socket_number].state= CONNECT;
        ret_err = err;
    }
    else {//receiving data    
        if (socket_number<0){
            printf("invalid socket number\n");
            pbuf_free(p);
            return err;
        }
        tcp_recved(tpcb, p->tot_len); 
        memset(ducoClient[socket_number].socketBuffer,0,128);
        memcpy((void *)ducoClient[socket_number].socketBuffer, p->payload, p->len);

        switch (ducoClient[socket_number].state){
            case CONNECT_WAIT:
                printf("Client %d: Connected.\n", socket_number);
                ducoClient[socket_number].state= JOB_REQUEST;
                break;
            case REQUEST_WAIT:
                printf("Client %d: Job received.\n", socket_number);
                ducoClient[socket_number].state= HASH_CALCULATION;
                break;
            case RESULT_WAIT:
                printf("Client %d: Result received.\n", socket_number);
                ducoClient[socket_number].state= RESULT_UPDATE;
                break;
        }
        pbuf_free(p); //free pbuf
        ret_err = ERR_OK;
    }
    return ret_err;
}


static void tcp_callback_error(void *arg, err_t err){
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);
}


static err_t tcp_callback_poll(void *arg, struct tcp_pcb *tpcb){
  return ERR_OK;
}

static void tcp_client_close(struct tcp_pcb *tpcb){
    uint8_t socketNumber = tcp_find_socket(tpcb);
    /* Clear callback functions */
    tcp_arg(ducoClient[socketNumber].tcp_client_pcb, NULL);
    tcp_sent(ducoClient[socketNumber].tcp_client_pcb, NULL);
    tcp_recv(ducoClient[socketNumber].tcp_client_pcb, NULL);
    tcp_err(ducoClient[socketNumber].tcp_client_pcb, NULL);
    tcp_poll(ducoClient[socketNumber].tcp_client_pcb, NULL, 0);
    tcp_close(ducoClient[socketNumber].tcp_client_pcb); // close connection
    ducoClient[socketNumber].tcp_client_pcb = NULL;
}

void update_pool(void){
    char* pBody=NULL;
    size_t bodyLen;

    uint8_t ip[4]={0,0,0,0};
    char s_ip[20];
    char * pOutValue= NULL;
    uint32_t outValueLength= 0U;
    JSONStatus_t result = JSONSuccess;

    g_http_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_NONE; // not used client certificate
    g_http_tls_context.clica_option = 0;                        // not used Root CA verify

    memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE);

    http_get(SOCKET_HTTP, g_ethernet_buf, DUINO_HTTP_GET_URL, &g_http_tls_context, &pBody, &bodyLen);

    result = JSON_Validate( pBody, bodyLen );
    if( result == JSONSuccess ){
        JSON_Search( ( char * )pBody, bodyLen, "ip", sizeof("ip")-1, &pOutValue, (size_t *)&outValueLength);
        sprintf((char*)s_ip,"%.*s", outValueLength, pOutValue);
        inet_addr_((uint8_t*)s_ip, ip);
        IP4_ADDR(&g_tcp_client_target_ip, ip[0], ip[1], ip[2], ip[3]); //  <<replace host ip;
     
        JSON_Search( ( char * )pBody, bodyLen, "port", sizeof("port")-1, &pOutValue, (size_t *)&outValueLength);
        g_tcp_client_target_port= (u16_t) strtoul( pOutValue, NULL, 10 );
        
        printf("\r\n > [host addr: %d.%d.%d.%d / %d]\r\n", ip[0], ip[1], ip[2], ip[3], g_tcp_client_target_port );
        memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE );
    }
    else{
        printf("\r\nThe json document is invalid!!: %d", result);
    }
}

void set_lcd_print_info(int8_t mem_index, int8_t slave_indnex, float elapsed_time_s, float hashrate, uint32_t difficulty, char* result){
   
    memset(&g_duino_lcd_info[mem_index], 0x00, sizeof(Duino_lcd_info));

    itoa(slave_indnex, g_duino_lcd_info[mem_index].slave_index, 10);
    itoa(difficulty  , g_duino_lcd_info[mem_index].difficulty , 10);
    sprintf(g_duino_lcd_info[mem_index].elapsed_time_s, "%.2f", elapsed_time_s);
    sprintf(g_duino_lcd_info[mem_index].hashrate,"%.2f", hashrate);
    memcpy (g_duino_lcd_info[mem_index].result, result, LCD_STRING_SIZE_RESULT);

}

void set_lcd_print_log(void){
    int8_t start_mem_cnt;
    
    if (g_lcd_newest_mem_index == LCD_DUINO_INFO_MEM_CNT_MAX|| g_lcd_printed_line_cnt < LCD_DUINO_INFO_PRINT_CNT_MAX )
        start_mem_cnt=0;
    else
        start_mem_cnt= g_lcd_newest_mem_index+1;
    // ILI9340_ClearScreen();
    for(int8_t i=0; g_lcd_printed_line_cnt > i; i++) { 
        ILI9340_Write_Mining_State( i, g_duino_lcd_info[start_mem_cnt].slave_index
                                     , g_duino_lcd_info[start_mem_cnt].result       , g_duino_lcd_info[start_mem_cnt].difficulty
                                     , g_duino_lcd_info[start_mem_cnt].hashrate     , g_duino_lcd_info[start_mem_cnt].elapsed_time_s);

        if (start_mem_cnt == LCD_DUINO_INFO_MEM_CNT_MAX)
            start_mem_cnt=0;
        else 
            start_mem_cnt++;
    }
    g_lcd_newest_mem_index= start_mem_cnt;
    
    if(g_lcd_printed_line_cnt < LCD_DUINO_INFO_PRINT_CNT_MAX )
        g_lcd_printed_line_cnt++;
}

#ifdef __cplusplus
}
#endif