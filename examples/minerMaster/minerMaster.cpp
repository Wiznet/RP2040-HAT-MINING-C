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

/* Duino coin */
#define DUINO_USER_NAME         "wiznet-mason"
#define DUINO_MINER_KEY         "1234"
    // Change the part in brackets if you want to set a custom miner name (use Auto to autogenerate, None for no name)
#define DUINO_RIG_IDENTIFIER    "WIZnet_Miner_" 
    // Do not change the lines below. These lines are static and dynamic variables
    //  that will be used by the program for counters and measurements.
#define DUINO_HTTP_GET_URL      "https://server.duinocoin.com/getPool"
#define DUINO_MINER_VER         "3.3"                  
#define DUINO_MINER_BANNER      "W5100S-EVB-Pico Miner"

#define DUINO_MSG_LEN_HASH  40
#define DUINO_MSG_LEN_TOTAL 90

#define DUINO_PLATFORM  "ESP8266NH"

/* Multicore*/
#define USE_DUALCORE
#define HASH_START          0
#define HASH_END            1
#define HASH_FINDING        0
#define HASH_NOT_FIND       1
#define HASH_RATE_DELAY_US  0

#define HASH_DELAY  "0\n"

/* Duco miner */
#define BUF_SIZE        256
#define MAX_NUM_SLAVE   10
#define CMD_STATUS      0x00
#define CMD_WRITE_JOB   0x01
#define CMD_READ_RESULT 0x02
#define CMD_RESET       0x55
#define CMD_CONFIG_HASH_DELAY   0x10



/**
 * ----------------------------------------------------------------------------------------------------
 * Structure
 * ----------------------------------------------------------------------------------------------------
 */


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
typedef struct Duino_lcd_info_t
{
    char  slave_index[LCD_STRING_SIZE_SLAVE_INDEX];
    char  result[LCD_STRING_SIZE_RESULT];
    char  hashrate[LCD_STRING_SIZE_HASH_RATE];
    char  elapsed_time_s[LCD_STRING_SIZE_ELAP_TIME];
    char  difficulty[LCD_STRING_SIZE_DIFFICULTY];
}Duino_lcd_info;

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */


/* Network */
static wiz_NetInfo g_net_info =
{
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip =  {192, 168, 11, 2},                     // IP address
    .sn =  {255, 255, 255, 0},                    // Subnet Mask
    .gw =  {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_DHCP                       // DHCP enable/disable
};

#define SOCK_BUF_SIZE 128

#define SERVER_RESPONSE_TIMEOUT 20
#define SLAVE_CALCULATION_TIMEOUT 50

typedef struct _ducoClient {
    struct tcp_pcb *tcp_client_pcb;
    uint8_t socketBuffer[SOCK_BUF_SIZE];
    uint8_t socketNumber;
    uint8_t slaveAddress;
    uint64_t timeOut;
    duinoClientState state;
    uint8_t lastBlockHash[41];
    float hashrate;
    int hashResult;
    float elapsedTime;
    uint32_t diff;
    uint8_t retryCounter;
} DucoClient;


static DucoClient ducoClient[SOCKET_MAX];


static uint8_t * g_ethernet_buf;

/* LWIP */
struct netif g_netif;

/* TCP */
static ip_addr_t g_tcp_client_target_ip;
u16_t g_tcp_client_target_port= 0;

//static struct tcp_pcb *tcp_client_pcb; //= NULL;
// static struct tcp_pcb *tcp_client_pcb [SOCKET_MAX]; //= NULL;
uint8_t pack [ETHERNET_MTU];

/* Http */
static tlsContext_t g_http_tls_context;

/* duino hash*/
static mbedtls_sha1_context core0_sha1_ctx, core0_sha1_ctx_base;
static unsigned char last_block_hash_str[DUINO_MSG_LEN_HASH+1];
static unsigned char expected_hash_str[DUINO_MSG_LEN_HASH+1];  
static char send_req_str[128];

static uint32_t difficulty = 0;
static uint8_t core0_hashArray[20];
static uint8_t duco_numeric_result_str[128];
static uint8_t expected_hash_arry[20];
static uint32_t duco_numeric_result;

static float hashrate = 0;
static uint32_t hash_cnt = 0;

/* Duino Loop*/
Loop_state loop_state = UpdatePool;
uint8_t close_flag = 0;
//uint8_t send_flag = 0;

uint32_t fail_count = 0;
float elapsed_time_s = 0;

/* Multicore*/
volatile static uint8_t hash_start_flag = HASH_END;
volatile static uint8_t core1_hash_find = HASH_FINDING;

/*I2C Master-Slave*/
uint8_t g_slave_job[BUF_SIZE];
uint32_t g_slave_count;
uint8_t g_duino_slave[10];
CharBuffer receiveBuffer, jobBuffer;

/* LCD */
static Duino_lcd_info g_duino_lcd_info[LCD_DUINO_INFO_MEM_ARR_SIZE];

int8_t g_lcd_newest_mem_index= 0;
int8_t g_lcd_printed_line_cnt= 1;


/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* I2C Master*/
uint32_t readJob(uint8_t * job, uint8_t * slaveJob, uint8_t socketNumber);
uint8_t sendResult(uint8_t * result);

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
bool connect_to_server(tcp_pcb *);
static void set_duino_res_msg(uint8_t * socketBuffer, uint32_t duco_numeric_result, float hashrate, uint8_t slaveNumber);
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

/* LCD */
void set_lcd_print_info(int8_t mem_index, int8_t slave_indnex, float elapsed_time_s, float hashrate, uint32_t difficulty, char* result);
void set_lcd_print_log(void);


int main() 
{

    int ret;
    err_t err;
    /* Initialize */

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

    update_pool();//getting pool ip
    free(g_ethernet_buf);
    wizchip_lwip_init(SOCKET_MACRAW, &g_netif, &g_net_info);
    set_duino_req_msg();
#ifdef USE_DUALCORE
    multicore_launch_core1(core1_entry);
#endif
    duino_master_init();
    //initial delay
    // sleep_ms(1000);
    while(1)
    { 
        // duino_server_state_loop();
        duino_master_state_loop();

    } 
}


/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* DHCP */
static void wizchip_dhcp_assign(void)
{
    getIPfromDHCP(g_net_info.ip);
    getGWfromDHCP(g_net_info.gw);
    getSNfromDHCP(g_net_info.sn);
    getDNSfromDHCP(g_net_info.dns);

    g_net_info.dhcp = NETINFO_DHCP;

    /* Network initialize */
    network_initialize(g_net_info); // apply from DHCP

    print_network_information(g_net_info);
    printf(" DHCP leased time : %ld seconds\n", getDHCPLeasetime());
}

static void wizchip_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\n");

    // halt or reset or any...
    while (1); // this example is halt.
}

void wizchip_dhcp_init(void)
{
    printf("\n DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}

/* I2C Master */
uint32_t busScan(uint8_t * slaveAddressArray){
    uint8_t g_slave_count = 0;
    int8_t ret = 0;
    uint8_t dummy;
    for(int addr = 0 ; addr < (1 << 7); ++addr){
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else{
            ret = i2c_read_blocking(i2c1, addr, &dummy, 1, false);
        }
        if(ret > 0){
            slaveAddressArray[g_slave_count++] = addr;
        }
        if(g_slave_count > 10){
            printf("maximum number of slaves are 10\n");
            return 10;
        }
    }
    return g_slave_count;
}

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

uint32_t writeCmd(uint8_t slaveAddress, uint8_t cmd, uint8_t * follow, uint32_t len){
    uint32_t count;
    Wire1.beginTransmission(slaveAddress);
    //write command
    Wire1.write(cmd);
    //write follow
    // sleep_ms(10);
    if(follow)
        Wire1.write((const uint8_t *)follow, len);
    count = Wire1.endTransmission(true);
    return count;
    //should do something with counter
}

uint32_t readData(uint8_t slaveAddress, uint32_t len){
    uint32_t count;
    uint8_t status;
    count = Wire1.requestFrom(slaveAddress,len,true);// Status: Core0 | Core1 | '\n'
    while(Wire1.available()){
        receiveBuffer.write((uint8_t)Wire1.read());
    }
    return receiveBuffer.length();
}

uint32_t readJob(uint8_t * job, uint8_t * slaveJob, uint8_t socketNumber){
    //소켓번호 할당해야함.

    // char dummyMessage[] = "4db808021dcee957da8b5f32e2a46a0ca0c6914f,24e948f4ff5a6984fa7b933c0cfa620fdde6c38b,25000\n";
    
    sprintf((char*)slaveJob, "%02d,%s", socketNumber, job);

    return strlen((char*)slaveJob);
}

uint8_t sendResult(uint8_t * result){
    printf("Result: %s\n",result);
    return 0;
}

void duino_master_init (void)
{
    Wire1.begin();
    
    printf("I2C Bus scanning..\n");
    g_slave_count= busScan(g_duino_slave);       
    printf("%d of slaves are detected.\n",g_slave_count);
    
    for(int i = 0 ; i < g_slave_count ; i++)
        printf("Detected slave address[%d]: %d\n",i, g_duino_slave[i]);
    
    for(int i = 0 ; i < g_slave_count ; i++){
        writeCmd(g_duino_slave[i],CMD_CONFIG_HASH_DELAY,(uint8_t *)HASH_DELAY, sizeof(HASH_DELAY));
        sleep_ms(100);
    }
    printf("Slave hash delay has been set to %s,%d\n",HASH_DELAY,sizeof(HASH_DELAY));
}

uint64_t timeOut(uint32_t sec){
    return time_us_64() + sec*1000000;
}

void duino_master_state_loop (void) //i2c whlie
{
    uint8_t tempBuffer[64] = "";
    uint8_t core[2];
    uint8_t slaveNumber = 0;
    uint8_t coreNumber = 0;
    err_t err;

    bool success;

    for(int clientNo = 0; clientNo < g_slave_count * 2; clientNo++)//0 ~ 9
    {
        slaveNumber = clientNo/2;// Slave Number
        coreNumber = clientNo%2;// 0 or 1 => core0 or core1
        switch (ducoClient[clientNo].state)
        {
        case CONNECT:
            printf("Client %d: CONNECT\n", clientNo);
            ducoClient[clientNo].elapsedTime = 0.0;
            ducoClient[clientNo].hashrate = 0.0;
            ducoClient[clientNo].hashResult = 0;

            if (ducoClient[clientNo].tcp_client_pcb == NULL)
            {
                ducoClient[clientNo].tcp_client_pcb = tcp_new();
            }
            if(ducoClient[clientNo].tcp_client_pcb->state != 0) tcp_client_close(ducoClient[clientNo].tcp_client_pcb);
            err = tcp_connect(ducoClient[clientNo].tcp_client_pcb, &g_tcp_client_target_ip, g_tcp_client_target_port, tcp_callback_connected);
            if(err == ERR_OK)
            {
                ducoClient[clientNo].timeOut = timeOut(SERVER_RESPONSE_TIMEOUT);//10 second
                ducoClient[clientNo].state = CONNECT_WAIT;
            } else {
                tcp_client_close(ducoClient[clientNo].tcp_client_pcb);//close socket and
                ducoClient[clientNo].state = CONNECT;//try to connect again
            }
            break;
        case CONNECT_WAIT:
            if(ducoClient[clientNo].timeOut < time_us_64()){//timeout
                printf("Client %d: CONNECT_WAIT timeout.\n", clientNo);
                tcp_client_close(ducoClient[clientNo].tcp_client_pcb);//close socket and 
                ducoClient[clientNo].state = CONNECT;//try to connect again
            }   
            break;
        case JOB_REQUEST:
            printf("Client %d: JOB_REQUEST\n", clientNo);
            
            //check status
            writeCmd(g_duino_slave[slaveNumber],CMD_STATUS,NULL,0);//read status command
            sleep_ms(100);
            readData(g_duino_slave[slaveNumber], 128);//read status data
            
            core[0] = receiveBuffer.read();
            core[1] = receiveBuffer.read();
            receiveBuffer.clear();

            // printf("core[0]:%c, core[1]:%c\n",core[0],core[1]);
            if(core[coreNumber] == '0'){// IDLE
                printf("Job request string: %s",send_req_str);
                err = tcp_write((tcp_pcb *)ducoClient[clientNo].tcp_client_pcb, send_req_str, sizeof(send_req_str)-1, 0);
                if(err != ERR_OK){
                    ducoClient[clientNo].state = CONNECT;
                    break;
                }
                ducoClient[clientNo].timeOut = timeOut(SERVER_RESPONSE_TIMEOUT);//10 second
                ducoClient[clientNo].state = REQUEST_WAIT;
            } else if (core[coreNumber] == '1'){//BUSY
                printf("Client %d: busy. Socket state: %d\n",clientNo, ducoClient[clientNo].tcp_client_pcb->state);
                //do nothing
                ducoClient[clientNo].state = JOB_REQUEST;
            } else if(core[coreNumber] == '2'){//DONE
                printf("Client %d: hash done. Socket state: %d\n",clientNo, ducoClient[clientNo].tcp_client_pcb->state);
                ducoClient[clientNo].state = SEND_RESULT;//read out
            } else { // SOMETHING WRONG!
                if(ducoClient[clientNo].retryCounter < 10)
                {
                    printf("Client %d: went wrong %02x retry %d.\n",clientNo,core[coreNumber],ducoClient[clientNo].retryCounter);
                    ducoClient[clientNo].state = JOB_REQUEST;//retry
                    ducoClient[clientNo].retryCounter++;
                } else {
                    printf("Client %d: slave %d error. SLAVE_FAULT!! \n", clientNo, slaveNumber);
                    ducoClient[clientNo].state = SLAVE_FAULT;//retry
                }
            }
            break;
        case REQUEST_WAIT:
            ducoClient[clientNo].retryCounter = 0;
            if(ducoClient[clientNo].timeOut < time_us_64()){//time out
                printf("Client %d: REQUEST_WAIT timeout\n", clientNo);
                ducoClient[clientNo].state = JOB_REQUEST;
            }   
            break;
        case HASH_CALCULATION:{
            printf("Client %d: HASH_CALCULATION\n",clientNo);
            sleep_ms(100);
            g_slave_job[0] = (uint8_t)coreNumber;
            memset(tempBuffer,0,64);
            //To get the hash difficulty from the job message
            jobBuffer.setBuffer((char*)ducoClient[clientNo].socketBuffer,strlen((char*)ducoClient[clientNo].socketBuffer));
            jobBuffer.readStringUntil(',',(char*)tempBuffer);//Last Block Hash => dummy
            if(strcmp((char*)tempBuffer, "BAD") == 0){ //if server response is "BAD" or the same block hash received.
                printf("Client %d: wrong job string. Retry to get the job.\n", clientNo);
                ducoClient[clientNo].state = JOB_REQUEST;//send job request again
                break;
            }
            uint8_t temp = 0;
            for(temp = 0 ; temp < g_slave_count * 2; temp++){//
                if(strcmp((char*)ducoClient[temp].lastBlockHash,(char*)tempBuffer) == 0){
                    printf("the same last block hash\n");
                    break;
                }
            }
            if(temp < g_slave_count * 2){
                ducoClient[clientNo].state = JOB_REQUEST;
                break;
            }
            // printf("Client %d: last block update [%s] [%s].\n", clientNo, tempBuffer, last_block_hash_str);
            memcpy(ducoClient[clientNo].lastBlockHash, tempBuffer, strlen((char*)tempBuffer));
        
            memset(tempBuffer,0,64);
            jobBuffer.readStringUntil(',',(char*)tempBuffer);//New Block Hash => dummy
            //last block hash update

            //Read the hash difficulty
            memset(tempBuffer,0,64);
            jobBuffer.readStringUntil('\n',(char*)tempBuffer);
            ducoClient[clientNo].diff = atoi((char*)tempBuffer);

            // if(ducoClient[clientNo].diff > 2000)
            // {
            //     printf("Client %d: Hash difficulty is too high %d\n",clientNo, ducoClient[clientNo].diff);
            //     tcp_client_close(ducoClient[clientNo].tcp_client_pcb);//close the socket
            //     ducoClient[clientNo].state = CONNECT;//and connect again
            //     break;
            // }
            //add the core number in front of the job string for the slave hash calculation.
            sprintf((char*)g_slave_job, "%d%s", coreNumber, (char*)ducoClient[clientNo].socketBuffer);
            //send the write hash command and the job string
            writeCmd(g_duino_slave[slaveNumber],CMD_WRITE_JOB, (uint8_t *)g_slave_job, strlen((char*)g_slave_job));
            
            printf("Client %d: job allocated => %s",clientNo, g_slave_job);
            ducoClient[clientNo].state = CALCULATION_WAIT;
            ducoClient[clientNo].timeOut = timeOut(SLAVE_CALCULATION_TIMEOUT);
            break;
        }
        case CALCULATION_WAIT:
            //check status
            
            writeCmd(g_duino_slave[slaveNumber],CMD_STATUS,NULL,0);//read status command
            sleep_ms(100);
            readData(g_duino_slave[slaveNumber], 128);//read status data
            
            core[0] = receiveBuffer.read();
            core[1] = receiveBuffer.read();

            receiveBuffer.clear();

            if(core[coreNumber] == '2'){//HASH DONE
                ducoClient[clientNo].state = SEND_RESULT;
                break;
            }
            break;
        case SEND_RESULT:
            ducoClient[clientNo].retryCounter = 0;
            printf("Client %d: SEND_RESULT\n",clientNo);
            // sleep_ms(100);
            //send read result command
            writeCmd(g_duino_slave[slaveNumber],CMD_READ_RESULT,&coreNumber,1);
            sleep_ms(100);
            //read results
            readData(g_duino_slave[slaveNumber],128);
            
            printf("Client %d: result => %s", clientNo, receiveBuffer.buf());

            memset(tempBuffer,0x00,64);
            receiveBuffer.readStringUntil(',',(char *)tempBuffer);
            ducoClient[clientNo].hashResult = atoi((char *)tempBuffer);

            memset(tempBuffer,0x00,64);
            receiveBuffer.readStringUntil('\n',(char *)tempBuffer);
            ducoClient[clientNo].elapsedTime = atoi((char *)tempBuffer) / 1000000.0;
            
            ducoClient[clientNo].hashrate = ducoClient[clientNo].hashResult/ducoClient[clientNo].elapsedTime;

            if(ducoClient[clientNo].hashResult == 0){
                if(ducoClient[clientNo].retryCounter == 2){//retry twice
                    ducoClient[clientNo].retryCounter = 0;
                    printf("Client %d: wrong result\n",clientNo);
                    ducoClient[clientNo].state = CONNECT; // went wrong
                    break;
                }
                ducoClient[clientNo].retryCounter++;
                break;
            }
            ducoClient[clientNo].retryCounter = 0;
            memset(ducoClient[clientNo].socketBuffer,0,128);
            set_duino_res_msg(( uint8_t *)ducoClient[clientNo].socketBuffer, ducoClient[clientNo].hashResult, ducoClient[clientNo].hashrate, clientNo);
            printf("Client %d: message '%s' will be sent.\n",clientNo, ducoClient[clientNo].socketBuffer);
            if(ducoClient[clientNo].tcp_client_pcb->state != ESTABLISHED){//if socket state is not in ESTABLISHED
                ducoClient[clientNo].state = CONNECT;//try to connect again
                break;
            }
            err = tcp_write((tcp_pcb *)ducoClient[clientNo].tcp_client_pcb, ( uint8_t *)ducoClient[clientNo].socketBuffer, sizeof(ducoClient[clientNo].socketBuffer) - 1, 0); //Getting hash job
            if(err < 0) {
                printf("SEND_RESULT Error:%02x",err);
                ducoClient[clientNo].state = CONNECT;
                break;
            }
            ducoClient[clientNo].timeOut = timeOut(SERVER_RESPONSE_TIMEOUT);//10 second
            ducoClient[clientNo].state = RESULT_WAIT;
            break;
        case RESULT_WAIT:
            if(ducoClient[clientNo].timeOut < time_us_64()){//time out
                printf("Client %d: RESULT_WAIT timeout.\n", clientNo);
                // tcp_client_close(ducoClient[i].tcp_client_pcb);
                ducoClient[clientNo].state = CONNECT;//try to reconnect again
            }  
            break;
        case RESULT_UPDATE:
            ducoClient[clientNo].state = JOB_REQUEST;
            printf("Client %d: RESULT_UPDATE\n",clientNo);
            printf("Client %d: %s",clientNo, ducoClient[clientNo].socketBuffer);
            if (0 == strncmp((char*)ducoClient[clientNo].socketBuffer, "GOOD", 4)) 
            {
                printf("Client %d: GOOD, %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", clientNo, ducoClient[clientNo].hashResult, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].elapsedTime);
                set_lcd_print_info(g_lcd_newest_mem_index, clientNo, ducoClient[clientNo].elapsedTime, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].diff, "GOOD"); //to be..insert socket number           
                set_lcd_print_log();
            }
            else if (0 == strncmp((char*)ducoClient[clientNo].socketBuffer, "BAD", 3)) 
            {
                printf("Client %d: BAD, %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", clientNo, ducoClient[clientNo].hashResult, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].elapsedTime);                    
                // set_lcd_print_info(g_lcd_newest_mem_index, clientNo, ducoClient[clientNo].elapsedTime, ducoClient[clientNo].hashrate/1000, ducoClient[clientNo].diff, "BAD"); //to be..insert socket number
                // set_lcd_print_log();
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

/* */
void core1_entry(void)
{
    printf("core1_entry: wizchip_lwip_process\r\n");
    while(1)
    {
        wizchip_lwip_process(SOCKET_MACRAW, &g_netif, pack);
    }
}

/* connect to duino server */
void set_duino_req_msg (void)
{
    memset(send_req_str, 0x00, 128);
    sprintf(send_req_str, "JOB,%s,%s,%s\n", DUINO_USER_NAME, DUINO_PLATFORM, DUINO_MINER_KEY);
}

static void set_duino_res_msg(uint8_t * socketBuffer, uint32_t duco_numeric_result, float hashrate, uint8_t slaveNumber)
{
    memset(socketBuffer, 0x00, SOCK_BUF_SIZE );
    // printf("[Hoon] len = %d, %s\r\n", 1024, (char*)socketBuffer);

    // sprintf((char*)socketBuffer, "%d,%.2f,%s%02d\n", duco_numeric_result , hashrate, DUINO_RIG_IDENTIFIER, slaveNumber);
    //miner 3.3 + information
    // sprintf((char*)socketBuffer, "%d,%.2f,%s %s,%s%02d\n", duco_numeric_result , hashrate, DUINO_MINER_BANNER, DUINO_MINER_VER, DUINO_RIG_IDENTIFIER,slaveNumber);
    //no hash rate information
    sprintf((char*)socketBuffer, "%d,,%s%02d\n", duco_numeric_result, DUINO_RIG_IDENTIFIER,slaveNumber);
}

// move to utils 
void wizchip_net_info_set(void)
{
    int ret;
    if (g_net_info.dhcp == NETINFO_DHCP) // DHCP
    {
        wizchip_dhcp_init();

        while (1)
        {
            ret = DHCP_run();

            if (ret == DHCP_IP_LEASED)
            {
                break;
            }
            wizchip_delay_ms(1000);
        }
    }
    else // static
    {
        network_initialize(g_net_info);
        print_network_information(g_net_info);
    }
}


/* Lwip */
int tcp_find_socket(tcp_pcb *tpcb)
{
    int cnt;
    
    for (cnt=0; cnt < SOCKET_MAX; cnt++)
    {
        if (ducoClient[cnt].tcp_client_pcb == tpcb){
            break;
        }
    }
    if (cnt == SOCKET_MAX) 
        cnt = -1;
    
    return cnt;
}
     
static err_t tcp_callback_connected(void *arg, struct tcp_pcb *pcb_new, err_t err)
{
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

static err_t tcp_callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    uint8_t socketNumber = tcp_find_socket(tpcb);
    LWIP_UNUSED_ARG(arg);//?
    LWIP_UNUSED_ARG(tpcb);//?
    LWIP_UNUSED_ARG(len);//?

    printf("Client %d: Send OK\r\n",socketNumber);

  return ERR_OK;
}

static err_t tcp_callback_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    int i;
    err_t ret_err;
    int socket_number;
    socket_number = tcp_find_socket(tpcb);

    if (p == NULL) //pbuf is null when session is closed
    {
        tcp_client_close(tpcb);
        printf("Client %d: socket closed => 0x%x, 0x%x\n",socket_number, tpcb, ducoClient[socket_number].tcp_client_pcb);
        ret_err = ERR_OK;
    }
    else if (err != ERR_OK) //ERR_ABRT is returned when called tcp_abort
    {
        tcp_recved(tpcb, p->tot_len); //advertise window size

        pbuf_free(p); //free pbuf
        ret_err = err;
    }
    else //receiving data
    {
        if (socket_number<0)
        {
            printf("invalid socket number\n");
            pbuf_free(p);
            return err;
        }

        tcp_recved(tpcb, p->tot_len); 
        memset(ducoClient[socket_number].socketBuffer,0,128);
        memcpy((void *)ducoClient[socket_number].socketBuffer, p->payload, p->len);

        // printf("recv data(%d): %s, state: %d\r\n", socket_number, ducoClient[socket_number].socketBuffer,ducoClient[socket_number].state);
        // printf("ducoClient State: %d, %d\n",socket_number,);
        switch (ducoClient[socket_number].state)
        {
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


static void tcp_callback_error(void *arg, err_t err)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);
}


static err_t tcp_callback_poll(void *arg, struct tcp_pcb *tpcb)
{
  return ERR_OK;
}

static void tcp_client_close(struct tcp_pcb *tpcb)
{
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

void update_pool(void)
{
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
    if( result == JSONSuccess )
    {
        JSON_Search( ( char * )pBody, bodyLen, "ip", sizeof("ip")-1, &pOutValue, (size_t *)&outValueLength);
        sprintf((char*)s_ip,"%.*s", outValueLength, pOutValue);
        inet_addr_((uint8_t*)s_ip, ip);
        IP4_ADDR(&g_tcp_client_target_ip, ip[0], ip[1], ip[2], ip[3]); //  <<replace host ip;
     
        JSON_Search( ( char * )pBody, bodyLen, "port", sizeof("port")-1, &pOutValue, (size_t *)&outValueLength);
        g_tcp_client_target_port= (u16_t) strtoul( pOutValue, NULL, 10 );
        
        printf("\r\n > [host addr: %d.%d.%d.%d / %d]\r\n", ip[0], ip[1], ip[2], ip[3], g_tcp_client_target_port );
        memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE );
    }
    else
    {
        printf("\r\nThe json document is invalid!!: %d", result);
    }

}

bool connect_to_server(tcp_pcb *tcp_clint_pcb)
{
    err_t err;        
    
    if (g_netif.ip_addr.addr == 0)
        return false;

    // printf("ducoClient: 0x%x\n",tcp_clint_pcb);
    // if (tcp_clint_pcb == NULL)
    // {
    //     tcp_clint_pcb = tcp_new();
        // printf("ducoClient: 0x%x, 0x%x\n",tcp_clint_pcb, &ducoClient[0].tcp_client_pcb);
        // if (tcp_clint_pcb != NULL)
        // {
            // if(ducoClient->tcp_client_pcb->state != CLOSED)
            //     tcp_client_close(ducoClient->tcp_client_pcb);
            
            err = tcp_connect(tcp_clint_pcb, &g_tcp_client_target_ip, g_tcp_client_target_port, tcp_callback_connected);

            if (err != ERR_OK)
            {     
                // printf("connect failed : %d\n", err);
                /* deallocate the pcb */
                tcp_client_close(tcp_clint_pcb);
                // memp_free(MEMP_TCP_PCB, tcp_clint_pcb);
                return false;
            } else {
                // printf("connected,%d\n",err);
                return true;
            }
        // } else {
        //     printf("connection failed\n");
        //     return false;   
        // }
    // }
}

void send_hash_result_to_server(uint8_t socket_num)
{
    /* send msg: response hash data */
    // set_duino_res_msg(duco_numeric_result, hashrate);
    printf("eth buf = %s", g_ethernet_buf);
    // tcp_write(tcp_client_pcb[socket_num], g_ethernet_buf, sizeof(g_ethernet_buf)-1, 0);
}

void set_lcd_print_info(int8_t mem_index, int8_t slave_indnex, float elapsed_time_s, float hashrate, uint32_t difficulty, char* result)
{
   
    memset(&g_duino_lcd_info[mem_index], 0x00, sizeof(Duino_lcd_info));

    itoa(slave_indnex, g_duino_lcd_info[mem_index].slave_index, 10);
    itoa(difficulty  , g_duino_lcd_info[mem_index].difficulty , 10);
    sprintf(g_duino_lcd_info[mem_index].elapsed_time_s, "%.2f", elapsed_time_s);
    sprintf(g_duino_lcd_info[mem_index].hashrate,"%.2f", hashrate);
    memcpy (g_duino_lcd_info[mem_index].result, result, LCD_STRING_SIZE_RESULT);

}

void set_lcd_print_log(void)
{
    int8_t start_mem_cnt;
    
    if (g_lcd_newest_mem_index == LCD_DUINO_INFO_MEM_CNT_MAX|| g_lcd_printed_line_cnt < LCD_DUINO_INFO_PRINT_CNT_MAX )
        start_mem_cnt=0;
    else
        start_mem_cnt= g_lcd_newest_mem_index+1;
    // ILI9340_ClearScreen();
    for(int8_t i=0; g_lcd_printed_line_cnt > i; i++)
    { 
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