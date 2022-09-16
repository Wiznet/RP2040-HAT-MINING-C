#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "port_common.h"
#include "pico/multicore.h"

#include "wizchip_conf.h"


#include "core_json.h"
#include "socket.h"
#include "dhcp.h"


#include "utils/inc/hwSetupUtils.h"
#include "utils/inc/charbuffer.h"
#include "utils/inc/Wire.h"
//#include "timer_interface.h"
#include "timer.h"

//#include "http_transport_interface.h"
#include "httpParser.h"

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
#define DUINO_CHIP_ID           "1234b" 
#define DUINO_USER_NAME         "wiznet_test"
#define DUINO_MINER_KEY         "1234"
    // Change the part in brackets if you want to set a custom miner name (use Auto to autogenerate, None for no name)
#define DUINO_RIG_IDENTIFIER    "W5100" 
    // Do not change the lines below. These lines are static and dynamic variables
    //  that will be used by the program for counters and measurements.
#define DUINO_HTTP_GET_URL      "https://server.duinocoin.com/getPool"
#define DUINO_MINER_VER         "3.18"                  
#define DUINO_MINER_BANNER      "W5100S-EVB-Pico Miner"

#define DUINO_MSG_LEN_HASH  40
#define DUINO_MSG_LEN_TOTAL 90

/* Multicore*/
#define USE_DUALCORE
#define HASH_START 0
#define HASH_END   1

#define HASH_FINDING 0
#define HASH_NOT_FIND 1
#define HASH_RATE_DELAY_US 0

/* Slave */
#define BUF_SIZE 256

#define CMD_STATUS 0x00
#define CMD_WRITE_JOB 0x01
#define CMD_READ_RESULT 0x02

#define MAX_NUM_SLAVE 10

/**
 * ----------------------------------------------------------------------------------------------------
 * Structure
 * ----------------------------------------------------------------------------------------------------
 */

 /* Duino */
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

#if 0
/* LCD */
typedef struct Duino_lcd_info_t
{
    char  slave_index[LCD_STRING_SIZE_SLAVE_INDEX];
    char  result[LCD_STRING_SIZE_RESULT];
    char  hashrate[LCD_STRING_SIZE_HASH_RATE];
    char  elapsed_time_s[LCD_STRING_SIZE_ELAP_TIME];
    char  difficulty[LCD_STRING_SIZE_DIFFICULTY];
}Duino_lcd_info;
#endif

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

static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/* LWIP */
struct netif g_netif;

/* TCP */
static ip_addr_t g_tcp_client_target_ip;
u16_t g_tcp_client_target_port= 0;

//static struct tcp_pcb *tcp_client_pcb; //= NULL;
static struct tcp_pcb *tcp_client_pcb [SOCKET_MAX]; //= NULL;
uint8_t pack [ETHERNET_MTU];

/* Http */
static tlsContext_t g_http_tls_context;

/* duino hash*/
static mbedtls_sha1_context core0_sha1_ctx, core1_sha1_ctx, core0_sha1_ctx_base, core1_sha1_ctx_base;
static unsigned char last_block_hash_str[DUINO_MSG_LEN_HASH+1];
static unsigned char expected_hash_str[DUINO_MSG_LEN_HASH+1];  
static char send_req_str[128];

static uint32_t difficulty = 0;
static uint8_t core0_hashArray[20];
static uint8_t core1_hashArray[20];
static uint8_t duco_numeric_result_str[128];
static uint8_t expected_hash_arry[20];
static uint32_t duco_numeric_result;

static float hashrate = 0;

static uint32_t hash_cnt = 0;

/* Duino Loop*/
Loop_state loop_state = UpdatePool;
uint8_t close_flag = 0;
uint8_t send_flag = 0;

uint32_t fail_count = 0;
float elapsed_time_s = 0;

/* Multicore*/
volatile static uint8_t hash_start_flag = HASH_END;
volatile static uint8_t core1_hash_find = HASH_FINDING;

/* Slave */
uint8_t ducoSlave[10];
CharBuffer receiveBuffer;

/* LCD */
#if 0
static Duino_host g_duino_host; 
static Duino_lcd_info g_duino_lcd_info[LCD_DUINO_INFO_MEM_ARR_SIZE];

int8_t g_lcd_newest_mem_index= 0;
int8_t g_lcd_printed_line_cnt= 1;
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

//Duino coin server functions
uint32_t readJob(uint8_t * job, uint8_t socketNumber);
uint8_t sendResult(uint8_t * result);

uint32_t writeCmd(uint8_t slaveAddress, uint8_t cmd, uint8_t * follow, uint32_t len);
uint32_t readData(uint8_t slaveAddress, uint32_t len);
uint32_t busScan(uint8_t * slaveAddressArray);
bool reserved_addr(uint8_t addr);

void wizchip_net_info_set(void);

/*DHCP*/
void wizchip_dhcp_init(void);

/* LWIP */
static err_t tcp_callback_connected(void *arg, struct tcp_pcb *pcb_new, err_t err);
static err_t tcp_callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_callback_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_callback_poll(void *arg, struct tcp_pcb *tpcb);
static void tcp_callback_error(void *arg, err_t err);
static void tcp_client_close(struct tcp_pcb *tpcb);

/* Multicore */
void core1_entry(void);


void update_pool(void);
void set_duino_req_msg (void);
void duino_state_loop(void);

int main() 
{

    int ret;
    err_t err;
    /* Initialize */

    set_clock_khz();
    stdio_init_all();

    wizchip_delay_ms(1000 * 3); // wait for 3 seconds
    
    wizchip_i2c_init();
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();
    wizchip_1ms_timer_initialize(repeating_timer_callback);

    //rp2040_get_board_id();
    //ILI9340_initialize();
    wizchip_net_info_set();

    update_pool();
    wizchip_lwip_init(SOCKET_MACRAW, &g_netif, &g_net_info);
    set_duino_req_msg();
#ifdef USE_DUALCORE
    multicore_launch_core1(core1_entry);
#endif
    //initial delay
    sleep_ms(1000);
//=========================== Slave Test =======================================//  
#if 0

    
    Wire1.begin();

    printf("Master starts\n");

    uint8_t job[BUF_SIZE];
    printf("I2C Bus scanning..\n");

    uint32_t slaveCount = busScan(ducoSlave);

    printf("%d of slaves are detected.\n",slaveCount);
        
    for(int i = 0 ; i < slaveCount ; i++)
        printf("Detected slave address[%d]: %d\n",i, ducoSlave[i]);
#endif
    int i;
    while(1)
    { 
       duino_state_loop();
           /* Create new TCP client */
#if 0           
    for (i = 0; i < SOCKET_MAX; i++)
    {
        if (tcp_client_pcb[i]->state != CLOSED)
        {
            /* Deallocate the pcb */
            memp_free(MEMP_TCP_PCB, tcp_client_pcb[i]);
        }

        tcp_client_pcb[i] = tcp_new();

        if (tcp_client_pcb[i] != NULL)
        {
            err = tcp_connect(tcp_client_pcb[i],
                              &g_tcp_client_target_ip, g_tcp_client_target_port,
                              tcp_callback_connected);

            if (err == ERR_OK)
            {
                printf(" Connect [%d.%d.%d.%d:%d]\n",
                            ip4_addr1(&tcp_client_pcb[i]->remote_ip),
                            ip4_addr2(&tcp_client_pcb[i]->remote_ip),
                            ip4_addr3(&tcp_client_pcb[i]->remote_ip),
                            ip4_addr4(&tcp_client_pcb[i]->remote_ip),
                            tcp_client_pcb[i]->remote_port);                            
                err= tcp_write(tcp_client_pcb[i], &send_req_str, sizeof(send_req_str)-1, 0);
                //recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, 0);
                //printf("Connected to the server. Server version: %s\r\n", g_ethernet_buf);                  
                printf("Connected to the server. socket index :%d\r\n", i);
            }
            else
            {
                printf(" Connect failed i = %d err = %d\r\n", i, err);
                /* Deallocate the pcb */
                memp_free(MEMP_TCP_PCB, tcp_client_pcb[i]);
            }
        }

        sleep_ms(1000); // wait for 1 second
    }
#endif

#if 0               
        for(int i = 0; i < slaveCount; i++){
            writeCmd(ducoSlave[i],CMD_STATUS,NULL,0);//read status command
            readData(ducoSlave[i], 128);//read status data
            uint8_t core0 = receiveBuffer.read();
            uint8_t core1 = receiveBuffer.read();
            receiveBuffer.clear();

            if(core0 == '0' || core1 =='0'){//HASH_IDLE
                //get the job from the duino coin server
                uint8_t socketNumber = rand()%99;
                uint32_t length = readJob(job, socketNumber);
                //send the write job command and the job string
                printf("Job allocated => %s\n",job);
                writeCmd(ducoSlave[i],CMD_WRITE_JOB, (uint8_t *)job, length);
            } else if (core0 == '2' || core1 == '2'){//HASH_DONE
                //send the read result command
                writeCmd(ducoSlave[i],CMD_READ_RESULT,NULL,0);
                //read results
                readData(ducoSlave[i],128);

                //do something with the result

                //send result to the duino coin server
                sendResult((uint8_t*)receiveBuffer.buf());
                //clear receive Buffer
                receiveBuffer.clear();
            }
        }

#endif        
        onBoardLedToggle();
    } // loop while
}


/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */


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


/* DHCP */
void wizchip_dhcp_init(void)
{
    printf("\n DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}



uint32_t busScan(uint8_t * slaveAddressArray){
    uint8_t slaveCount = 0;
    int8_t ret = 0;
    uint8_t dummy;
    for(int addr = 100 ; addr < (1 << 7); ++addr){
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
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

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

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
    //should do something with counter
}

uint32_t readData(uint8_t slaveAddress, uint32_t len){
    uint32_t count;
    uint8_t status;
    count = Wire1.requestFrom(slaveAddress,len,false);// Status: Core0 | Core1 | '\n'
    while(Wire1.available()){
        receiveBuffer.write((uint8_t)Wire1.read());
    }
    return receiveBuffer.length();
}

uint32_t readJob(uint8_t * job, uint8_t socketNumber){
    //소켓번호 할당해야함.

    char dummyMessage[] = "4db808021dcee957da8b5f32e2a46a0ca0c6914f,24e948f4ff5a6984fa7b933c0cfa620fdde6c38b,25000\n";
    
    sprintf((char*)job, "%02d,%s", socketNumber, dummyMessage);

    return strlen((char*)job);
}

uint8_t sendResult(uint8_t * result){
    printf("Result: %s\n",result);
    return 0;
}


void core1_entry(void)
{

    printf("core1_entry\r\n");
    while(1)
    {
        wizchip_lwip_process(SOCKET_MACRAW, &g_netif, pack);
    }
#if 0    
    int ret;
    uint32_t start_time = 0;
    uint32_t end_time = 0;
    uint32_t hash_cnt = 1;
    uint32_t hash_number;
    unsigned long elapsed_time = 0; 
    float elapsed_time_s = 0;

   // printf("core1_entry\r\n");
    while(1)
    {
        if (hash_start_flag == HASH_START)
        {
            //printf("core1 hash_start_flag == HASH_START\r\n");
            /* run hash */
            core1_hash_find = HASH_FINDING;
            mbedtls_sha1_init(&core1_sha1_ctx_base);
            
            if( ( ret = mbedtls_sha1_starts_ret( &core1_sha1_ctx_base ) ) != 0 )
                printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
                
            if ( ret = mbedtls_sha1_update_ret(&core1_sha1_ctx_base, last_block_hash_str, strlen((const char*)last_block_hash_str) ) != 0 )
                printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);
            
            str_to_array(expected_hash_str, expected_hash_arry);
            
            for (hash_number = 1; hash_number < difficulty; hash_number+=2)
            {
                memcpy(&core1_sha1_ctx, &core1_sha1_ctx_base, sizeof(mbedtls_sha1_context));
                memset(duco_numeric_result_str, 0x00, 128);
                sprintf((char*)duco_numeric_result_str, "%d", hash_number);
            
                if ( ret = mbedtls_sha1_update_ret(&core1_sha1_ctx, duco_numeric_result_str, strlen((const char*)duco_numeric_result_str) ) != 0 )
                    printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);
            
                if ( ret = mbedtls_sha1_finish_ret(&core1_sha1_ctx, core1_hashArray) != 0 )
                    printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);            
            
                if(hash_start_flag == HASH_END)
                  break;
                
                if(!(memcmp(expected_hash_arry, core1_hashArray, 20)))
                {
                    duco_numeric_result = hash_number;
                    printf("Core1 Find Hash %d\r\n", duco_numeric_result);
                    hash_start_flag = HASH_END;
                    break;
                }
            }
            mbedtls_sha1_free(&core1_sha1_ctx_base);
            //printf("core1 calu end hash_number = %d\r\n", hash_number);
            
            if (hash_number >= difficulty)
            {
              printf("Not Find Core1 wait hash_number = %d\r\n", hash_number);
              core1_hash_find = HASH_NOT_FIND;
              while(hash_start_flag != HASH_END);
            }
       }
       
    }
#endif
}

static uint8_t get_duino_hash_data (char* data, char* separator, char* last_block_hash, char* expected_hash, uint32_t* difficulty)
{
    uint32_t temp_num= 0;
    char* cursor=NULL, ret;

    memcpy(last_block_hash, data, DUINO_MSG_LEN_HASH);

    cursor= strchr(data, *separator)+1;
    memcpy(expected_hash, cursor, DUINO_MSG_LEN_HASH);

    cursor= strchr(cursor, *separator)+1;
    temp_num= atol(cursor);
    *difficulty= temp_num*100+1;

    return 0;
}

/* connect to duino server */
void set_duino_req_msg (void)
{
    memset(send_req_str, 0x00, 128);
    sprintf(send_req_str, "JOB,%s,LOW,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);

}

static void set_duino_res_msg(uint32_t duco_numeric_result, float hashrate)
{
    memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE );
    sprintf((char*)g_ethernet_buf, "%d,%.2f,%s %s,%s,DUCOID%s\n", duco_numeric_result , hashrate       , DUINO_MINER_BANNER, DUINO_MINER_VER
                                                         , DUINO_RIG_IDENTIFIER, DUINO_CHIP_ID);
    printf("[Hoon] len = %d, %s\r\n", 1024, (char*)g_ethernet_buf);
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
static err_t tcp_callback_connected(void *arg, struct tcp_pcb *pcb_new, err_t err)
{
    LWIP_UNUSED_ARG(arg);

    if (err != ERR_OK) //error when connect to the server
    {
        return err;
    }

    tcp_setprio(pcb_new, TCP_PRIO_NORMAL); //set priority for the client pcb

    tcp_arg(pcb_new, 0); //no argument is used
    tcp_sent(pcb_new, tcp_callback_sent); //register send callback
    tcp_recv(pcb_new, tcp_callback_received);  //register receive callback
    tcp_err(pcb_new, tcp_callback_error); //register error callback
    tcp_poll(pcb_new, tcp_callback_poll, 0); //register poll callback

    return ERR_OK;
}

static err_t tcp_callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(tpcb);
    LWIP_UNUSED_ARG(len);

    printf("Sent OK\r\n");

  return ERR_OK;
}

static err_t tcp_callback_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    int i;
    err_t ret_err;

    if (p == NULL) //pbuf is null when session is closed
    {
        tcp_client_close(tpcb);
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
        int i;

        for (i=0; i <=SOCKET_MAX; i++)
        {
            if (tcp_client_pcb[i]== tpcb) break;
        }

        if (i == SOCKET_MAX) 
            printf("\n no matched tpcb > ");        
        else            
            printf("\n pcd%d > ", i);        
        
        tcp_recved(tpcb, p->tot_len); //advertise window size
        printf("recv data: %.*s\r\n", p->len, p->payload);

        //tcp_write(tpcb, p->payload, p->len, 0);
        pbuf_free(p); //free pbuf
        ret_err = ERR_OK;        
#if 0        
        tcp_recved(tpcb, p->tot_len); 
        memcpy(&g_ethernet_buf, p->payload, p->len);

        printf("recv data: %s\r\n", g_ethernet_buf);
        switch (loop_state)
        {
            case Wait_ConServer:
                loop_state= GetHash;
                break;
            case WaitRecvHash:
                loop_state= RecvHash;
                break;
            case WaitResult:
                loop_state= GetResult;
                break;
        }

        pbuf_free(p); //free pbuf
        ret_err = ERR_OK;
#endif        
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
    /* Clear callback functions */
    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);

    tcp_close(tpcb); // close connection
    tpcb = NULL;
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
        //set_duino_host_addr_info(ip, &g_tcp_client_target_port);
        memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE );
    }
    else
    {
        printf("\r\nThe json document is invalid!!: %d", result);
    }

}
#if 0
void connect_to_server(uint8_t socket_num, uint32_t close_flag)
{
    err_t err;        
    
    if (g_netif.ip_addr.addr == 0)
        return;

    if (tcp_client_pcb == NULL)
    {
        tcp_client_pcb = tcp_new();
        if (tcp_client_pcb != NULL)
        {
            err = tcp_connect(tcp_client_pcb, &g_tcp_client_target_ip, g_tcp_client_target_port, tcp_callback_connected);
            if (err != ERR_OK)
            {     
                printf(" connect failed : %d\n", err);
                /* deallocate the pcb */
                memp_free(MEMP_TCP_PCB, tcp_client_pcb);
            }
        }
    }
}
#endif
float calulate_hash(uint8_t socket_num)   //1 core full speed is about 36us per once hashing
{
    int ret;
    uint32_t hash_number;    
    uint32_t start_time = 0;
    uint32_t end_time = 0;

    uint32_t keepalive_start_time = 0;
    uint32_t keepalive_end_time = 0;
    
    unsigned long elapsed_time = 0; 
    float elapsed_time_s = 0;
    
    start_time = time_us_32();  
    str_to_array((unsigned char*)expected_hash_str, expected_hash_arry);
    hash_start_flag = HASH_START;
    /* run hash */

    printf("Core0 Hash Start\r\n");
    mbedtls_sha1_init(&core0_sha1_ctx_base);

    if( ( ret = mbedtls_sha1_starts_ret( &core0_sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
        
    if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx_base, (unsigned char*)last_block_hash_str, strlen((char*)last_block_hash_str) ) != 0 )
        printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);
        
    keepalive_start_time = time_us_32();
#ifdef USE_DUALCORE
    for (hash_number = 0; hash_number < difficulty; hash_number+=2)
#else
    for (hash_number = 0; hash_number < difficulty; hash_number++)
#endif
    {
#if HASH_RATE_DELAY_US
        sleep_us(HASH_RATE_DELAY_US);
#endif
        memcpy(&core0_sha1_ctx, &core0_sha1_ctx_base, sizeof(mbedtls_sha1_context));
        memset(duco_numeric_result_str, 0x00, 128);
        sprintf((char*)duco_numeric_result_str, "%d", hash_number);

        if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx, duco_numeric_result_str, strlen((char*)duco_numeric_result_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        if ( ret = mbedtls_sha1_finish_ret(&core0_sha1_ctx, core0_hashArray) != 0 )
            printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

        keepalive_end_time = time_us_32();

        if(hash_start_flag == HASH_END)
          break;
        
        if(!(memcmp(expected_hash_arry, core0_hashArray, 20)))
        {
            hash_start_flag = HASH_END;
            duco_numeric_result = hash_number;
            printf("Core0 Find Hash %d\r\n", duco_numeric_result);
            break;
        }
    }
    if (hash_number >= difficulty)
    {
      printf("Not Find Core0 wait hash_number = %d\r\n", hash_number);
      while(1)
      {
          if (hash_start_flag == HASH_END)
          {
              printf("Find Hash from Core1\r\n");
              break;
          }
          
          if (core1_hash_find == HASH_NOT_FIND)
          {
              printf("Not find Hash Core1 too\r\n");
              hash_start_flag = HASH_END;
              break;
          }
      }

    }
    mbedtls_sha1_free(&core0_sha1_ctx_base);

    end_time = time_us_32();
    elapsed_time   = end_time- start_time;
    elapsed_time_s = (elapsed_time * .000001f);

    hashrate = (duco_numeric_result / elapsed_time_s);
    printf("hashrate:%.2fKH/s (%.2fs)\r\n", hashrate/1000, elapsed_time_s);

    return elapsed_time_s;
}

void send_hash_result_to_server(uint8_t socket_num)
{
    /* send msg: response hash data */
    set_duino_res_msg(duco_numeric_result, hashrate);
    printf("eth buf = %s", g_ethernet_buf);
//    tcp_write(tcp_client_pcb, g_ethernet_buf, sizeof(g_ethernet_buf)-1, 0);
}

void duino_state_loop(void)
{
    uint8_t ret=0;
    switch (loop_state)
    {
        case UpdatePool :
            //update_pool();
            //wizchip_lwip_init(SOCKET_MACRAW, &g_netif, &g_net_info);
            loop_state = ConServer;
            break;

        case ConServer :
//            connect_to_server(SOCKET_MACRAW, close_flag);
            close_flag = 0;
            loop_state = Wait_ConServer;
            break;
            
        case GetHash :
//            tcp_write(tcp_client_pcb, send_req_str, sizeof(send_req_str)-1, 0);
            loop_state = WaitRecvHash;
            break;

        case RecvHash:                         
            ret = get_duino_hash_data((char*)g_ethernet_buf, ",", (char*)last_block_hash_str,(char*) expected_hash_str, &difficulty);
            printf("get_duino_hash_data ret = %d\r\n\r\n", ret);
            if (ret != 0)
            {
                loop_state = ConServer;
                close_flag = 1;
                fail_count++;
                if (fail_count == FAIL_COUNT)
                {
                    printf("fail_count == 5\r\n");
                    fail_count = 0;
                    loop_state = UpdatePool;
                }
            }
            else
            {
#if 1                   
                printf("difficulty: %d\r\n", difficulty);
                printf("last_block_hash_str = %s\r\n", last_block_hash_str);
                printf("expected_hash_str = %s\r\n", expected_hash_str);
#endif
                fail_count = 0;
                loop_state = CalHash;
            }
            break;

        case CalHash :
            elapsed_time_s = calulate_hash(SOCKET_MACRAW);
            loop_state = SendHash;
            break;

        case SendHash :
            send_hash_result_to_server(0);
            loop_state= WaitResult;
            break;
            
        case GetResult:
            if (0 == strncmp((char*)g_ethernet_buf, "GOOD", 4)) 
            {
                printf("\r\n GOOD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);
                //set_lcd_print_info(g_lcd_newest_mem_index, 1, elapsed_time_s, hashrate/1000, difficulty, "GOOD");            
            }
            else if (0 == strncmp((char*)g_ethernet_buf, "BAD", 3)) 
            {
                printf("\r\n BAD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  ", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);                      
                //set_lcd_print_info(g_lcd_newest_mem_index, 1, elapsed_time_s, hashrate/1000, difficulty, "BAD ");
            }
            //set_lcd_print_log();
            loop_state = GetHash;
            break;
    }
}

#if 0
void lwip_loop(void)
{
    err_t err;
    wizchip_lwip_process(SOCKET_MACRAW, &g_netif, pack);

    if (g_netif.ip_addr.addr == 0)
            return;
    
    for (int i=0; i< SOCKET_SIZE_MAX; i++)
    {
        if (tcp_client_pcb[i] == NULL)
        {
            tcp_client_pcb[i] = tcp_new();
            if (tcp_client_pcb[i] != NULL)
            {

                err= tcp_connect(tcp_client_pcb[i], &g_tcp_client_target_ip, g_tcp_client_target_port+i, tcp_callback_connected);
                if (err == ERR_OK)
                {
            
                    err= tcp_write(tcp_client_pcb[i], &send_req_str, sizeof(send_req_str)-1, 0);
                    printf("Connected to the server. socket index :%d\r\n", i);                  
                }
                else
                {
                    printf(" connect failed(%d) : %d\n", i, err);
                    /* deallocate the pcb */
                    tcp_client_close(tcp_client_pcb[i]);
                    memp_free(MEMP_TCP_PCB, tcp_client_pcb[i]);
                }
            }
        }
    }

}
#endif

#ifdef __cplusplus
}
#endif