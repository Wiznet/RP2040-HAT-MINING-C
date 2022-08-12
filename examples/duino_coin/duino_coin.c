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

#include "port_common.h"
#include "pico/multicore.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "w5x00_spi.h"

#include "dhcp.h"
#include "timer.h"

#include "http_transport_interface.h"
#include "timer_interface.h"

#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/compat-1.3.h"
#include "mbedtls/sha1.h"

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>

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
#define US_TIMER_KEEPALIVE      30000000 //30s

#define FAIL_COUNT 5

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_HTTP 0
#define SOCKET_DHCP 1
#define SOCKET_TCP  2

/* Port */
#define PORT_SSL 443

/* HTTP */
#define HTTP_BUF_MAX_SIZE (1024 * 2)

/* Duino coin */
#define DUINO_CHIP_ID           "1234b" 
#define DUINO_USER_NAME         "wiznet-mason"
#define DUINO_MINER_KEY         "1234"
    // Change the part in brackets if you want to set a custom miner name (use Auto to autogenerate, None for no name)
#define DUINO_RIG_IDENTIFIER    "W5100" 
    // Do not change the lines below. These lines are static and dynamic variables
    //  that will be used by the program for counters and measurements.
#define DUINO_HTTP_GET_URL      "https://server.duinocoin.com/getPool"
#define DUINO_MINER_VER         "3.18"                  
#define DUINO_MINER_BANNER      "W5100S-EVB-Pico Miner"
//#define DUINO_MINER_BANNER      "Official ESP8266 Miner"

#define DUINO_MSG_LEN_HASH  41
#define DUINO_MSG_LEN_TOTAL 90

/* Multicore*/
#define USE_DUALCORE
#define HASH_START 0
#define HASH_END   1

#define HASH_FINDING 0
#define HASH_NOT_FIND 1

#define HASH_RATE_DELAY_US 0
#define TEST_PIN 15

/**
 * ----------------------------------------------------------------------------------------------------
 * Structure
 * ----------------------------------------------------------------------------------------------------
 */
 /* Duino */
typedef enum {
    UpdatePool = 0,
    ConServer,
    RecvData,
    GetHash,
    CalHash,
    SendHash,
}Loop_state;

typedef struct Duino_host_t
{
    uint32_t port;   ///< host port
    uint8_t  ip[4];   ///< host ip
}Duino_host;

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/* duino */
static Duino_host g_duino_host; 

/* Network */
static wiz_NetInfo g_net_info =
{
    .mac = {0x00, 0x08, 0xDC, 0x5C, 0x7D, 0x50}, // MAC address
    .ip =  {192, 168, 11, 2},                     // IP address
    .sn =  {255, 255, 255, 0},                    // Subnet Mask
    .gw =  {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_DHCP                       // DHCP enable/disable
};

static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/* Http */
static tlsContext_t g_http_tls_context;

/* duino hash*/
static mbedtls_sha1_context core0_sha1_ctx, core1_sha1_ctx, core0_sha1_ctx_base, core1_sha1_ctx_base;
static char last_block_hash_str[DUINO_MSG_LEN_HASH];
static char expected_hash_str[DUINO_MSG_LEN_HASH];  
static char send_req_str[128];

static uint32_t difficulty = 0;
static uint8_t core0_hashArray[20];
static uint8_t core1_hashArray[20];
static uint8_t duco_numeric_result_str[128];
static uint8_t expected_hash_arry[20];
static uint32_t duco_numeric_result;

static float hashrate = 0;

/* Multicore*/
volatile static uint8_t hash_start_flag = HASH_END;
volatile static uint8_t core1_hash_find = HASH_FINDING;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/*DHCP*/
static void wizchip_dhcp_init(void);
static void wizchip_dhcp_assign(void);
static void wizchip_dhcp_conflict(void);

/* Clock */
static void set_clock_khz(void);

/* duino */
static uint8_t str_to_array(uint8_t * str, uint8_t * hex);

static void set_duino_req_msg (void);
static void set_duino_res_msg (uint32_t duco_numeric_result, float hashrate);
static int  get_duino_hash_data (char* data, char* separator, char* last_block_hash, char* expected_hash, uint32_t* difficulty);
extern void set_duino_host_addr_info (uint8_t* addr, uint32_t* port);

/* Multicore */
void core1_entry(void);

/* Duino */
void update_pool(void);
void connect_to_server(uint8_t socket_num, uint32_t close_flag);
int recv_data_from_server(uint8_t socket_num, uint8_t *recv_buf, uint32_t buf_size, uint32_t timeout);
int get_hash_data_from_server(uint8_t socket_num);
float calulate_hash(uint8_t socket_num);
void send_hash_result_to_server(uint8_t socket_num);



/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */

int main()
{
    Loop_state loop_state = UpdatePool;
    int ret;
    uint8_t socket_num = SOCKET_TCP;
    uint8_t close_flag = 0;
    uint8_t send_flag = 0;
    uint32_t hash_cnt = 0;
    uint32_t fail_count = 0;
    float elapsed_time_s = 0;

    /* Initialize */
    set_clock_khz();

    stdio_init_all();

#ifdef USE_DUALCORE
    multicore_launch_core1(core1_entry);
#endif

    wizchip_delay_ms(1000 * 3); // wait for 3 seconds

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();
    wizchip_1ms_timer_initialize(repeating_timer_callback);

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

    gpio_init(TEST_PIN);
    gpio_set_dir(TEST_PIN, GPIO_OUT);
    gpio_put(TEST_PIN, 0);

    
    set_duino_req_msg();
    /* run duino hash loop */
    while(1)
    {
        switch (loop_state){
            case UpdatePool :
                update_pool();
                loop_state = ConServer;
                break;

            case ConServer :
                connect_to_server(socket_num, close_flag);
                close_flag = 0;
#if 0
                if (send_flag)
                {
                    loop_state = SendHash;
                    send_flag = 0;
                }
                else
#endif
                loop_state = GetHash;
                break;
                
            case GetHash :
                ret = get_hash_data_from_server(socket_num);
                if (ret < 0)
                {
                    printf("get_hash_data_from_server error ret = %d\r\n\r\n", ret);
                }
                
                ret = get_duino_hash_data(g_ethernet_buf, ",", last_block_hash_str, expected_hash_str, &difficulty);
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
                    printf("difficulty: %d\r\n", difficulty);
                    printf("last_block_hash_str = %s\r\n", last_block_hash_str);
                    printf("expected_hash_str = %s\r\n", expected_hash_str);
                    fail_count = 0;
                    loop_state = CalHash;
                }
#if 0
                ret = disconnect(socket_num);
                printf("disconnect(socket_num) = 0x%X\r\n", ret);
                close(socket_num);
#endif
                break;

            case CalHash :
                elapsed_time_s = calulate_hash(socket_num);
                loop_state = SendHash;
                break;

            case SendHash :
                ret = getSn_SR(socket_num);
                printf("getSn_SR(socket_num) ret = 0x%0X\r\n", ret);
                if (ret == SOCK_ESTABLISHED)
                {
                    if(getSn_IR(socket_num) & Sn_IR_CON)	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
                    {
                        printf("getSn_IR(socket_num) & Sn_IR_CON\r\n"); 
			                  setSn_IR(socket_num, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
                    }
                }
                else
                {
                    printf("getSn_SR(socket_num) != SOCK_ESTABLISHED\r\n");
                    close_flag = 1;
                    send_flag = 1;
                    loop_state = ConServer;
                    break;
                }
            
                send_hash_result_to_server(socket_num);
                ret = recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, US_TIMER_WAIT_TIMEOUT);
                printf("SendHash Recv = %s\r\n", g_ethernet_buf);
                if (ret < 0)
                {
                    printf("failed send hash data to server\r\n");
                    close_flag = 1;
                    loop_state = ConServer;
                }
                else
                    loop_state = GetHash;

                if (0 == strncmp(g_ethernet_buf, "GOOD", 4)) {
                    printf("\r\n GOOD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);                
                }
                else if (0 == strncmp(g_ethernet_buf, "BAD", 3)) {
                    printf("\r\n BAD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  ", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);
                    printf("\r\n   >> %.*s\r\n\r\n", 25, g_ethernet_buf);
                }
                hash_cnt++;
                break;
        }
        DHCP_run();;
    }/* run duino loop */ 
    
    printf("Duino failed...\r\n");
    while(1);
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* DHCP */
static void wizchip_dhcp_init(void)
{
    printf(" DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}

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

/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

static uint8_t str_to_array(uint8_t * str, uint8_t * hex)
{
	uint8_t i;
	uint8_t hexcnt = 0;
	uint8_t hn, ln;
	uint8_t str_tmp[2];
	uint8_t len = strlen((char *)str);
	
	if((len < 2)  ||  (*str == 0)) return 0;

	for(i = 0; i < len; i+=2)
	{
		//Convert each character to uppercase
		str_tmp[0] = (uint8_t)toupper(str[i]);
		str_tmp[1] = (uint8_t)toupper(str[i+1]);

		hn = str_tmp[0] > '9' ? (str_tmp[0] - 'A' + 10) : (str_tmp[0] - '0');
		ln = str_tmp[1] > '9' ? (str_tmp[1] - 'A' + 10) : (str_tmp[1] - '0');

		hex[hexcnt++] = (hn << 4) | ln;
	}

	return 1;
}

static int get_duino_hash_data (char* data, char* separator, char* last_block_hash, char* expected_hash, uint32_t* difficulty)
{
    bool ret= 0;

    int idx_cnt1= 0; 
    int idx_cnt2= 0;
    uint32_t temp_num= 0;

    char temp_data[DUINO_MSG_LEN_TOTAL];
    char temp_out1[DUINO_MSG_LEN_HASH];
    char temp_out2[DUINO_MSG_LEN_HASH];

    if (data[0] == NULL)
      return -1;

    memset(temp_data, 0x00, DUINO_MSG_LEN_TOTAL);
    memset(temp_out1, 0x00, DUINO_MSG_LEN_HASH);
    memset(temp_out2, 0x00, DUINO_MSG_LEN_HASH);

    memcpy(temp_data, data, DUINO_MSG_LEN_TOTAL);

    while (idx_cnt1 <= DUINO_MSG_LEN_TOTAL) 
    {
        if (temp_data[idx_cnt1] == *separator) break;
        if (idx_cnt1 == DUINO_MSG_LEN_HASH)    return 1;
  
        temp_out1[idx_cnt1]= temp_data[idx_cnt1];
        idx_cnt1++;
    }
    memcpy(last_block_hash, temp_out1, idx_cnt1);

    idx_cnt1++; // jump ,
    while (idx_cnt1 <= DUINO_MSG_LEN_TOTAL) 
    {
        if (temp_data[idx_cnt1] == *separator) break;
        if (idx_cnt1 == DUINO_MSG_LEN_TOTAL)   return 2;
        
        temp_out2[idx_cnt2]= temp_data[idx_cnt1];
        idx_cnt1++; 
        idx_cnt2++;
    }    
    memcpy(expected_hash, temp_out2, idx_cnt2);

    idx_cnt1++; // jump ,
    temp_num= atol(&temp_data[idx_cnt1]) ;
    if (temp_num==0) return 3;

    *difficulty= temp_num*100+1;

    return 0;
}

/* connect to duino server */
static void set_duino_req_msg (void)
{
    memset(send_req_str, 0x00, 128);
    //sprintf(send_req_str, "JOB,%s,RP2040,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
    //sprintf(send_req_str, "JOB,%s,ESP32,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
    //sprintf(send_req_str, "JOB,%s,Arduino,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
    sprintf(send_req_str, "JOB,%s,LOW,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
    //sprintf(send_req_str, "JOB,%s,Raspberry Pi Pico,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
    //sprintf(send_req_str, "JOB,%s,ESP8266,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
}

static void set_duino_res_msg(uint32_t duco_numeric_result, float hashrate)
{
    memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE );
    sprintf(g_ethernet_buf, "%d,%.2f,%s %s,%s,DUCOID%s\n", duco_numeric_result , hashrate       , DUINO_MINER_BANNER, DUINO_MINER_VER
                                                          , DUINO_RIG_IDENTIFIER, DUINO_CHIP_ID);
    printf("[Hoon] len = %d, %s\r\n", strlen(g_ethernet_buf), g_ethernet_buf);
}

void set_duino_host_addr_info (uint8_t* addr, uint32_t* port)
{
    memcpy(&g_duino_host.ip[0], addr, 4);
    g_duino_host.port= *port;
    printf("\r\n > host addr: %d.%d.%d.%d/ %d \r\n", g_duino_host.ip[0], g_duino_host.ip[1]
                                                   , g_duino_host.ip[2], g_duino_host.ip[3], g_duino_host.port );
}

void core1_entry(void)
{
    int ret;
    uint32_t start_time = 0;
    uint32_t end_time = 0;
    uint32_t hash_cnt = 1;
    uint32_t hash_number;
    unsigned long elapsed_time = 0; 
    float elapsed_time_s = 0;

    printf("core1_entry\r\n");
    while(1)
    {
        if (hash_start_flag == HASH_START)
        {
            printf("core1 hash_start_flag == HASH_START\r\n");
            /* run hash */
            //start_time = time_us_32();
            core1_hash_find = HASH_FINDING;
            mbedtls_sha1_init(&core1_sha1_ctx_base);
            
            if( ( ret = mbedtls_sha1_starts_ret( &core1_sha1_ctx_base ) ) != 0 )
                printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
                
            if ( ret = mbedtls_sha1_update_ret(&core1_sha1_ctx_base, last_block_hash_str, strlen(last_block_hash_str) ) != 0 )
                printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);
            
            str_to_array(expected_hash_str, expected_hash_arry);
            
            //for (hash_number = core1_difficulty_start; hash_number < difficulty; hash_number++)
            for (hash_number = 1; hash_number < difficulty; hash_number+=2)
            {
                memcpy(&core1_sha1_ctx, &core1_sha1_ctx_base, sizeof(mbedtls_sha1_context));
                memset(duco_numeric_result_str, 0x00, 128);
                sprintf(duco_numeric_result_str, "%d", hash_number);
            
                if ( ret = mbedtls_sha1_update_ret(&core1_sha1_ctx, duco_numeric_result_str, strlen(duco_numeric_result_str) ) != 0 )
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
            printf("core1 calu end hash_number = %d\r\n", hash_number);
            
            if (hash_number >= difficulty)
            {
              printf("Not Find Core1 wait hash_number = %d\r\n", hash_number);
              core1_hash_find = HASH_NOT_FIND;
              while(hash_start_flag != HASH_END);
            }
       }
       
    }
}

void update_pool(void)
{
    g_http_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_NONE; // not used client certificate
    g_http_tls_context.clica_option = 0;                        // not used Root CA verify

    memset(g_ethernet_buf, 0x00, ETHERNET_BUF_MAX_SIZE);
    http_get(SOCKET_HTTP, g_ethernet_buf, DUINO_HTTP_GET_URL, &g_http_tls_context);
    //set IP
}

void connect_to_server(uint8_t socket_num, uint32_t close_flag)
{
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
    
}

int recv_data_from_server(uint8_t socket_num, uint8_t *recv_buf, uint32_t buf_size, uint32_t timeout)
{
    int size;
    uint32_t start_time;
    uint32_t end_time;

    start_time = time_us_32();

    while(1)
    {
        if (getSn_RX_RSR(socket_num) > 0)
        {
            memset(recv_buf, 0x00, buf_size);
            size = recv(socket_num, recv_buf, buf_size);
            break;
        }

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
    
    printf("Asking for a new job for user: %s\r\n", DUINO_USER_NAME);
    printf("Send Data = %s, len = %d\r\n", send_req_str, strlen(send_req_str));
    printf("send socket_num = %d\r\n", socket_num);
    ret = send(socket_num, send_req_str, strlen(send_req_str));
    printf("send_ret = %d\r\n", ret);
    ret = recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, US_TIMER_WAIT_TIMEOUT);
    printf("recv ret = %d, data = %s\r\n", ret, g_ethernet_buf);

    return ret;
}

#if 0
double calulate_hash(uint8_t socket_num)
{
    int ret;
    uint32_t hash_number;    
    uint32_t start_time = 0;
    uint32_t end_time = 0;

    uint32_t keepalive_start_time = 0;
    uint32_t keepalive_end_time = 0;
    
    uint64_t elapsed_time = 0; 
    double elapsed_time_s = 0;
    
    start_time = time_us_32();
    str_to_array(expected_hash_str, expected_hash_arry);
    hash_start_flag = HASH_START;
    /* run hash */

    printf("Core0 Hash Start\r\n");
    mbedtls_sha1_init(&core0_sha1_ctx_base);

    if( ( ret = mbedtls_sha1_starts_ret( &core0_sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
        
    if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx_base, last_block_hash_str, strlen(last_block_hash_str) ) != 0 )
        printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);
        
    keepalive_start_time = time_us_32();
    for (hash_number = 0; hash_number < difficulty; hash_number+=2)
    {
        memcpy(&core0_sha1_ctx, &core0_sha1_ctx_base, sizeof(mbedtls_sha1_context));
        memset(duco_numeric_result_str, 0x00, 128);
        sprintf(duco_numeric_result_str, "%d", hash_number);

        if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx, duco_numeric_result_str, strlen(duco_numeric_result_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        if ( ret = mbedtls_sha1_finish_ret(&core0_sha1_ctx, core0_hashArray) != 0 )
            printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

        keepalive_end_time = time_us_32();

        if ((keepalive_end_time - keepalive_start_time) > US_TIMER_KEEPALIVE)
        {
            ret = setsockopt(socket_num, SO_KEEPALIVESEND, 0);
            printf("keepalive ret = %d\r\n", ret);
            keepalive_start_time = time_us_32();
        }

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
//    if (start_time > end_time) end_time = end_time + (US_TIMER_MAX-start_time);

    elapsed_time   = end_time- start_time;
    elapsed_time_s = (elapsed_time * .000001f);

    hashrate = (duco_numeric_result / elapsed_time_s);
    printf("hashrate:%.2fKH/s (%.2fs)\r\n", hashrate/1000, elapsed_time_s);

    return elapsed_time_s;
}

#else

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
    //gpio_put(TEST_PIN, 1);
    str_to_array(expected_hash_str, expected_hash_arry);
    hash_start_flag = HASH_START;
    /* run hash */

    printf("Core0 Hash Start\r\n");
    mbedtls_sha1_init(&core0_sha1_ctx_base);

    if( ( ret = mbedtls_sha1_starts_ret( &core0_sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
        
    if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx_base, last_block_hash_str, strlen(last_block_hash_str) ) != 0 )
        printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);
        
    keepalive_start_time = time_us_32();
#ifdef USE_DUALCORE
    for (hash_number = 0; hash_number < difficulty; hash_number+=2)
#else
    for (hash_number = 0; hash_number < difficulty; hash_number++)
#endif
    {
        //gpio_put(TEST_PIN, !gpio_get(TEST_PIN));

#if HASH_RATE_DELAY_US
        sleep_us(HASH_RATE_DELAY_US);
#endif

        //if (recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, 1) > 0)
            //printf("!!!recv = %s\r\n", g_ethernet_buf);
        memcpy(&core0_sha1_ctx, &core0_sha1_ctx_base, sizeof(mbedtls_sha1_context));
        memset(duco_numeric_result_str, 0x00, 128);
        sprintf(duco_numeric_result_str, "%d", hash_number);

        if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx, duco_numeric_result_str, strlen(duco_numeric_result_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        if ( ret = mbedtls_sha1_finish_ret(&core0_sha1_ctx, core0_hashArray) != 0 )
            printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

        keepalive_end_time = time_us_32();

#if 0
        if ((keepalive_end_time - keepalive_start_time) > US_TIMER_KEEPALIVE)
        {
            ret = send(socket_num, "PING", 4);
            printf("send PING ret = %d\r\n", ret);
            recv_data_from_server(socket_num, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE, 1000);
            printf("recv PING = %s\r\n", g_ethernet_buf);
            keepalive_start_time = time_us_32();
#if 0
            ret = setsockopt(socket_num, SO_KEEPALIVESEND, 0);
            printf("keepalive ret = %d\r\n", ret);
            keepalive_start_time = time_us_32();
#endif
        }
#endif

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
    //gpio_put(TEST_PIN, 0);
    elapsed_time   = end_time- start_time;
    elapsed_time_s = (elapsed_time * .000001f);

    hashrate = (duco_numeric_result / elapsed_time_s);
    printf("hashrate:%.2fKH/s (%.2fs)\r\n", hashrate/1000, elapsed_time_s);

    return elapsed_time_s;
}
#endif

void send_hash_result_to_server(uint8_t socket_num)
{
    /* send msg: response hash data */
    set_duino_res_msg(duco_numeric_result, hashrate);
    send(socket_num, g_ethernet_buf, strlen(g_ethernet_buf));
}


