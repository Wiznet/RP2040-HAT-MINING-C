/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "wizchip_conf.h"

#include "port_common.h"
#include "pico/multicore.h"

#include "utils/inc/hwSetupUtils.h"
#include "utils/inc/charbuffer.h"
#include "utils/inc/Wire.h"

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
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

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

#define SOCKET_MAX      20

/* Port */
#define PORT_SSL 443
#define PORT_TARGET 6000

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

#define HASH_START      0
#define HASH_END        1
#define HASH_FINDING    0
#define HASH_NOT_FIND   1
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

static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/* LWIP */
struct netif g_netif;

/* TCP */
static ip_addr_t g_tcp_client_target_ip;
u16_t g_tcp_client_target_port= 0;

static struct tcp_pcb *tcp_client_pcb [SOCKET_MAX];
uint8_t pack [ETHERNET_MTU];

/* Http */
static tlsContext_t g_http_tls_context;

/* duino */
static char rp2040_id[8];

/* duino hash*/
static mbedtls_sha1_context core0_sha1_ctx, core1_sha1_ctx, core0_sha1_ctx_base, core1_sha1_ctx_base;
static char last_block_hash_str[DUINO_MSG_LEN_HASH+1];
static char expected_hash_str[DUINO_MSG_LEN_HASH+1];  
static char send_req_str[128];

static uint32_t difficulty = 0;
static uint8_t core0_hashArray[20];
static uint8_t core1_hashArray[20];
static uint8_t duco_numeric_result_str[128];
static uint8_t expected_hash_arry[20];
static uint32_t duco_numeric_result;

static float hashrate = 0;
static uint32_t hash_cnt = 0;


/* Duino Loop << use?? */
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
static Duino_lcd_info g_duino_lcd_info[LCD_DUINO_INFO_MEM_ARR_SIZE];

int8_t g_lcd_newest_mem_index= 0;
int8_t g_lcd_printed_line_cnt= 1;

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

/*DHCP*/
static void wizchip_dhcp_init(void);
void wizchip_net_info_set(void);

/* LWIP */
static err_t tcp_callback_connected(void *arg, struct tcp_pcb *pcb_new, err_t err);
static err_t tcp_callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_callback_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_callback_poll(void *arg, struct tcp_pcb *tpcb);
static void tcp_callback_error(void *arg, err_t err);
static void app_close_conn(struct tcp_pcb *tpcb); //close function
//static void app_send_data(void); //send function

/* Duino */
static void set_duino_req_msg (void);
void update_pool(void);
void duino_state_loop(void);

/* Multicore */
void core1_entry(void);

/* LCD */
void set_lcd_print_info(int8_t mem_index, int8_t slave_indnex, float elapsed_time_s, float hashrate, uint32_t difficulty, char* result);
void set_lcd_print_log(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */

int main()
{
    /* Initialize */
    int i;
    int ret = 0;
    err_t err;

    set_clock_khz();
    // initialize stdio after the clock change
    stdio_init_all();

    sleep_ms(1000 * 3); // wait for 3 seconds

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
    sleep_ms(10000);
    
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



#if 1 //for test
    /* Create new TCP client */
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
    while(1);
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

static void wizchip_dhcp_init(void)
{
    printf("\n DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}

/*
 * tcp_callback_connected
 * callback when connected, client sends a request to the server
 */
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

/*
 * tcp_callback_sent
 * callback when data sending is finished, control leds
 */
static err_t tcp_callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(tpcb);
    LWIP_UNUSED_ARG(len);

    printf("Sent OK\r\n");

  return ERR_OK;
}

/*
 * tcp_callback_received
 * callback when data is received, validate received data and parse it
 */
static err_t tcp_callback_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    err_t ret_err;

    if (p == NULL) //pbuf is null when session is closed
    {
        app_close_conn(tpcb);
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
    }

  return ret_err;
}

/*
 * app_close_conn
 * close connection & clear callbacks
 */
static void app_close_conn(struct tcp_pcb *tpcb)
{
  /* clear callback functions */
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);

  tcp_close(tpcb);    //close connection
  tpcb = NULL;
}

/*
 *  error callback
 *  call when there's an error, turn on an error led
 */
static void tcp_callback_error(void *arg, err_t err)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);
}

/*
 * poll callback
 * called when lwip is idle, do something such as watchdog reset
 */
static err_t tcp_callback_poll(void *arg, struct tcp_pcb *tpcb)
{
  return ERR_OK;
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
static void set_duino_req_msg (void)
{
    memset(send_req_str, 0x00, 128);
    sprintf(send_req_str, "JOB,%s,LOW,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);

}

static void set_duino_res_msg(uint32_t duco_numeric_result, float hashrate)
{
    memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE );
    sprintf((char*)g_ethernet_buf, "%d,%.2f,%s %s,%s,DUCOID%s\n", duco_numeric_result , hashrate       , DUINO_MINER_BANNER, DUINO_MINER_VER
                                                          , DUINO_RIG_IDENTIFIER, DUINO_CHIP_ID);
    //printf("[Hoon] len = %d, %s\r\n", strlen(g_ethernet_buf), g_ethernet_buf);
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

void core1_entry(void)
{
    printf("core1_entry\r\n");
    while(1)
    {
        wizchip_lwip_process(SOCKET_MACRAW, &g_netif, pack);
    }
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

    printf("bodylen = %d, pBody = \r\n%.*s\r\n", bodyLen, bodyLen, pBody);

    result = JSON_Validate( pBody, bodyLen );
    if( result == JSONSuccess )
    {
        JSON_Search( ( char * )pBody, bodyLen, "ip", sizeof("ip")-1, &pOutValue, (size_t *)&outValueLength);
        sprintf(s_ip,"%.*s", outValueLength, pOutValue);
        inet_addr_((uint8_t*)s_ip, ip);

        IP4_ADDR(&g_tcp_client_target_ip, ip[0], ip[1], ip[2], ip[3]); //  <<replace host ip;
     
        JSON_Search( ( char * )pBody, bodyLen, "port", sizeof("port")-1, &pOutValue, (size_t *)&outValueLength);
        g_tcp_client_target_port= (u16_t) strtoul( pOutValue, NULL, 10 );
        
        printf("\r\n > [host addr: %d.%d.%d.%d / %d]\r\n", ip[0], ip[1], ip[2], ip[3], g_tcp_client_target_port );
        //set_duino_host_addr_info(ip, &g_tcp_client_target_port);
    }
    else
    {
        printf("\r\nThe json document is invalid!!: %d\r\n", result);
        while(1);
    }

}

void connect_to_server(uint8_t socket_num, uint32_t close_flag)
{
#if 0         
    if (g_netif.ip_addr.addr != 0)
    {
        /* Create new TCP client 1 */
        if (tcp_client_pcb == NULL)
        {
            tcp_client_pcb = tcp_new();
            if (tcp_client_pcb != NULL)
            {
                err_t err;

                err = tcp_connect(tcp_client_pcb, &g_tcp_client_target_ip, g_tcp_client_target_port, tcp_callback_connected);
                if (err == ERR_OK)
                {
                    //recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, 0);
                    //printf("Connected to the server. Server version: %s\r\n", g_ethernet_buf);                  
                    printf("Connected to the server. Server version\r\n");                  
                }
                else
                {
                    printf(" connect failed : %d\n", err);
                    /* deallocate the pcb */
                    memp_free(MEMP_TCP_PCB, tcp_client_pcb);
                }
            }
        }
        
    }  
    else
    {
        printf(" netif ip is null \n");
    }
 //        
    int ret, size;
    
    if (close_flag)
      close(socket_num);

       /* connect to server */
    ret = socket(socket_num, Sn_MR_TCP, g_duino_host.port, 0);
    if (ret != socket_num)
    {
        printf(" failed\n  ! socket returned %d\n\n", ret);
    }

    /* send msg: request hash data */
    ret= connect(socket_num, g_duino_host.ip,  g_duino_host.port);
    if (ret == SOCK_FATAL)
    {
        printf(" failed\n  ! connect returned %d\n\n", ret);
        while(1);
    }
    
    recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, 0);
    printf("Connected to the server. Server version: %s\r\n", g_ethernet_buf);
#endif    
}

int recv_data_from_server(uint8_t socket_num, uint8_t *recv_buf, uint32_t buf_size, uint32_t timeout)
{
    int size;
    uint32_t start_time;
    uint32_t end_time;

    start_time = time_us_32();

    while(1)
    {
#if 0        
        if (getSn_RX_RSR(socket_num) > 0)
        {
            memset(recv_buf, 0x00, buf_size);
            size = recv(socket_num, recv_buf, buf_size);
            break;
        }
#endif        

        if (timeout)
        {
            end_time = time_us_32();
            if (end_time - start_time > timeout)
                return -1;
        }
    }
    return size;
}

int get_hash_data_from_server(uint8_t socket_num)
{
    int ret;
#if 0    
    printf("Asking for a new job for user: %s\r\n", DUINO_USER_NAME);
    printf("Send Data = %s, len = %d\r\n", send_req_str, strlen(send_req_str));
    printf("send socket_num = %d\r\n", socket_num);

    ret = send(socket_num, send_req_str, strlen(send_req_str));
    //printf("send_ret = %d\r\n", ret);
    ret = recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, US_TIMER_WAIT_TIMEOUT);
    //printf("recv ret = %d, data = %s\r\n", ret, g_ethernet_buf);
#endif
    //tcp_write(tcp_client_pcb, send_req_str, 128, 0);
    ret = recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, US_TIMER_WAIT_TIMEOUT);

    return ret;
}


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
    str_to_array((uint8_t*)expected_hash_str, expected_hash_arry);
    hash_start_flag = HASH_START;
    /* run hash */

    //printf("Core0 Hash Start\r\n");
    mbedtls_sha1_init(&core0_sha1_ctx_base);

    if( ( ret = mbedtls_sha1_starts_ret( &core0_sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
        
    if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx_base,(unsigned char*) last_block_hash_str, strlen(last_block_hash_str) ) != 0 )
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

        if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx, duco_numeric_result_str, strlen((char *)duco_numeric_result_str) ) != 0 )
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
            //printf("Core0 Find Hash %d\r\n", duco_numeric_result);
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
    send(socket_num, g_ethernet_buf, strlen((char*)g_ethernet_buf));
}

void rp2040_get_board_id(void)
{
    pico_get_unique_board_id_string(rp2040_id, 8);
}

void set_lcd_print_info(int8_t mem_index, int8_t slave_indnex, float elapsed_time_s, float hashrate, uint32_t difficulty, char* result)
{
    if (mem_index>LCD_DUINO_INFO_MEM_ARR_SIZE || mem_index<0) 
    {
        printf("\r\n invalid memory index :%d", mem_index);
        return ;
    }
    if (slave_indnex<=0) 
    {
        printf("\r\n invalid slave index :%d", slave_indnex);
        return ;
    }
    if (difficulty<=0)
    {
        printf("\r\n invalid difficulty :%d", difficulty);
        return ;
    }    
    if (elapsed_time_s<=0)
    {
        printf("\r\n invalid elapsed time :%.2f", elapsed_time_s);
        return ;
    }    
    if (hashrate<=0)
    {
        printf("\r\n invalid memory index :%.2f", hashrate/1000);
        return ;
    }    
    if (0 !=strcmp(result,"GOOD") && 0 !=strcmp(result,"BAD "))   
        {
        printf("\r\n invalid result :%s", result);
        return ;
    }
    
    memset(&g_duino_lcd_info[mem_index], 0x00, sizeof(Duino_lcd_info));

    itoa(slave_indnex, g_duino_lcd_info[mem_index].slave_index, 10);
    itoa(difficulty  , g_duino_lcd_info[mem_index].difficulty , 10);
    sprintf(g_duino_lcd_info[mem_index].elapsed_time_s, "%.2f", elapsed_time_s);
    sprintf(g_duino_lcd_info[mem_index].hashrate,"%.2f", hashrate);
    strncpy(g_duino_lcd_info[mem_index].result, result, LCD_STRING_SIZE_RESULT);

}

void set_lcd_print_log(void)
{
    int8_t start_mem_cnt;
    char    temp[2]; //replace slave index
    
    if (g_lcd_newest_mem_index == LCD_DUINO_INFO_MEM_CNT_MAX|| g_lcd_printed_line_cnt < LCD_DUINO_INFO_PRINT_CNT_MAX )
        start_mem_cnt=0;
    else
        start_mem_cnt= g_lcd_newest_mem_index+1;
    
    for(int8_t i=0; g_lcd_printed_line_cnt > i; i++)
    { 
        itoa(start_mem_cnt, temp, 10);
        ILI9340_Write_Mining_State( i, temp //g_duino_lcd_info[start_mem_cnt].slave_index
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