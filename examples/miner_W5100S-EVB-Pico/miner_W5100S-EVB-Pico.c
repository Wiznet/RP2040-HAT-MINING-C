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
#include "pico/unique_id.h"

#include "core_json.h"
#include "httpParser.h"
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
#define SOCKET_CORE0  2
#define SOCKET_CORE1  3

/* Port */
#define PORT_SSL 443

/* Duino coin information for the mining*/
// User name : input yours
#define DUINO_USER_NAME         "user name"
// Miner key : input yours
#define DUINO_MINER_KEY         "miner key"
//RIG_Identifier : Anything can be used for here
#define DUINO_RIG_IDENTIFIER    "RP2040_Core" 

//Miner software version
#define DUINO_MINER_BANNER      "W5100S-EVB-Pico Miner v1.0"
//Duino Platform name to be used when requesting hash job
//This changes the hash difficulty.
//ESP8266NH: 3500
//ESP32: 1250
//AVR: 450
#define DUINO_PLATFORM          "ESP8266NH"
#define DUINO_MINER_VER         "3.3"   
 
/* duino coin server url to get pool information*/
#define DUINO_HTTP_GET_URL      "https://server.duinocoin.com/getPool"

#define DUINO_HASH_ARRAY_LEN 20
#define DUINO_MSG_LEN_HASH  40

/**
 * ----------------------------------------------------------------------------------------------------
 * Structure
 * ----------------------------------------------------------------------------------------------------
 */
 /* Duino */
typedef enum {
    ConServer = 0,
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

static uint8_t g_ethernet_buf_core0[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

static uint8_t g_ethernet_buf_core1[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/* Http */
static tlsContext_t g_http_tls_context;

/* duino */
static Duino_host g_duino_host; 
static char rp2040_id[8];
static char send_req_str[128];

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

/* Duino */
static uint8_t str_to_array(uint8_t * str, uint8_t * hex);
static void set_duino_req_msg (void);
static void set_duino_res_msg(uint8_t socket_num, uint32_t duco_numeric_result, float hashrate, uint8_t *res_buf, uint32_t res_buf_size);
static uint8_t get_duino_hash_data (char* data, char* separator, char* last_block_hash, char* expected_hash, uint32_t* difficulty);
void set_duino_host_addr_info (uint8_t* addr, uint32_t* port);
void update_pool(void);
void connect_to_server(uint8_t socket_num, uint32_t close_flag);
int recv_data_from_server(uint8_t socket_num, uint8_t *recv_buf, uint32_t buf_size, uint32_t timeout);
int get_hash_data_from_server(uint8_t socket_num, uint8_t *recv_buf, uint32_t recv_buf_size);
float calculate_hash_core0(uint8_t socket_num, uint32_t difficulty, uint8_t *last_block_hash_str, \
                    uint8_t *expected_hash_str, uint32_t *duco_numeric_result, float *hashrate);

float calculate_hash_core1(uint8_t socket_num, uint32_t difficulty, uint8_t *last_block_hash_str, \
                    uint8_t *expected_hash_str, uint32_t *duco_numeric_result, float *hashrate);                    
void send_hash_result_to_server(uint8_t socket_num, uint32_t duco_numeric_result, float hashrate, uint8_t *send_buf, uint32_t send_buf_size);

/* Multicore */
void core1_entry(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */

int main()
{
    int ret;
    Loop_state loop_state = ConServer;
    uint8_t socket_num = SOCKET_CORE0;
    uint8_t close_flag = 0;
    uint8_t send_flag = 0;
    uint32_t hash_cnt = 0;
    uint32_t fail_count = 0;
    uint32_t total_time_start, total_time_end;
    float elapsed_time_s = 0;
    float hashrate;

    uint32_t difficulty;
    uint32_t duco_numeric_result;
    char last_block_hash_str[DUINO_MSG_LEN_HASH+1];
    char expected_hash_str[DUINO_MSG_LEN_HASH+1];

    /* Initialize */
    set_clock_khz();
    stdio_init_all();

    wizchip_delay_ms(1000 * 3); // wait for 3 seconds
    printf("Hello Mining\r\n");

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();
    wizchip_1ms_timer_initialize(repeating_timer_callback);
    
    rp2040_get_board_id();

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
    set_duino_req_msg();
    update_pool();

    multicore_launch_core1(core1_entry);
    
    //while(1);
    while(1)
    {
        switch (loop_state){
            case ConServer :
                connect_to_server(socket_num, close_flag);
                close_flag = 0;
                loop_state = GetHash;
                break;
                
            case GetHash :
                ret = get_hash_data_from_server(socket_num, g_ethernet_buf_core0, ETHERNET_BUF_MAX_SIZE);
                if (ret < 0)
                {
                    printf("get_hash_data_from_server error ret = %d\r\n\r\n", ret);
                }
                total_time_start = time_us_32();

                ret = get_duino_hash_data(g_ethernet_buf_core0, ",", last_block_hash_str, expected_hash_str, &difficulty);
                printf("[core0] get_duino_hash_data ret = %d\r\n\r\n", ret);
                if (ret != 0)
                {
                    loop_state = ConServer;
                    close_flag = 1;
                    fail_count++;
                    if (fail_count == FAIL_COUNT)
                    {
                        printf("fail_count == 5 reset device\r\n");
                        sleep_ms(100);
                        NVIC_SystemReset();
                    }
                }
                else
                {
#if 1                    
                    printf("[core0] difficulty: %d\r\n", difficulty);
                    printf("[core0] last_block_hash_str = %s\r\n", last_block_hash_str);
                    printf("[core0] expected_hash_str = %s\r\n", expected_hash_str);
#endif
                    fail_count = 0;
                    loop_state = CalHash;
                }
                break;

            case CalHash :
                elapsed_time_s = calculate_hash_core0(socket_num, difficulty, last_block_hash_str, expected_hash_str, &duco_numeric_result, &hashrate);
                loop_state = SendHash;
                break;

            case SendHash :
                ret = getSn_SR(socket_num);
                printf("[core0] getSn_SR(socket_num) ret = 0x%0X\r\n", ret);
                if (ret == SOCK_ESTABLISHED)
                {
                    if(getSn_IR(socket_num) & Sn_IR_CON)	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
                    {
                        printf("[core0] getSn_IR(socket_num) & Sn_IR_CON\r\n"); 
			                  setSn_IR(socket_num, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
                    }
                }
                else
                {
                    printf("[core0] getSn_SR(socket_num) != SOCK_ESTABLISHED\r\n");
                    close_flag = 1;
                    send_flag = 1;
                    loop_state = ConServer;
                    break;
                }

                send_hash_result_to_server(socket_num, duco_numeric_result, hashrate, g_ethernet_buf_core0, ETHERNET_BUF_MAX_SIZE);
                total_time_end = time_us_32();

                ret = recv_data_from_server(socket_num, g_ethernet_buf_core0, ETHERNET_BUF_MAX_SIZE, US_TIMER_WAIT_TIMEOUT);
                printf("[core0] SendHash Recv = %s\r\n", g_ethernet_buf_core0);
                if (ret < 0)
                {
                    printf("[core0] failed send hash data to server\r\n");
                    close_flag = 1;
                    loop_state = ConServer;
                }
                else
                {
                    if (0 == strncmp(g_ethernet_buf_core0, "GOOD", 4)) {
                        printf("\r\n[core0] GOOD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);
                    }
                    else if (0 == strncmp(g_ethernet_buf_core0, "BAD", 3)) {
                        printf("\r\n[core0] BAD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  ", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);                      
                    }
                    loop_state = GetHash;
                }
                printf("[core0] get hash to send result time = %dms\r\n", (total_time_end - total_time_start) / 1000);
                hash_cnt++;
                break;
        }
        DHCP_run();
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

    DHCP_init(SOCKET_DHCP, g_ethernet_buf_core0);

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

static uint8_t get_duino_hash_data (char* data, char* separator, char* last_block_hash, char* expected_hash, uint32_t* difficulty)
{
    uint32_t temp_num= 0;
    char* cursor=NULL, ret;

    memset(last_block_hash, 0x00, DUINO_MSG_LEN_HASH+1);
    memset(expected_hash, 0x00, DUINO_MSG_LEN_HASH+1);

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
    sprintf(send_req_str, "JOB,%s,ESP8266NH,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
    printf("%s\r\n", send_req_str);
}

static void set_duino_res_msg(uint8_t socket_num, uint32_t duco_numeric_result, float hashrate, uint8_t *res_buf, uint32_t res_buf_size)
{
    uint8_t core_num;
    if (socket_num == SOCKET_CORE0)
        core_num = 0;
    else if (socket_num == SOCKET_CORE1)
        core_num = 1;

    memset(res_buf, 0x00, res_buf_size );
    //sprintf(g_ethernet_buf, "%d,%.2f\n", duco_numeric_result , hashrate);
    sprintf(res_buf, "%d,%.2f,%s %s,%s%d,DUCOID%s\n", duco_numeric_result , hashrate, DUINO_MINER_BANNER, DUINO_MINER_VER
                                                    , DUINO_RIG_IDENTIFIER, core_num, rp2040_id);
    printf("len = %d, %s\r\n", strlen(res_buf), res_buf);
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
    Loop_state loop_state = ConServer;
    uint8_t socket_num = SOCKET_CORE1;
    uint8_t close_flag = 0;
    uint8_t send_flag = 0;
    uint32_t hash_cnt = 0;
    uint32_t fail_count = 0;
    uint32_t total_time_start, total_time_end;
    float elapsed_time_s = 0;
    float hashrate;

    uint32_t difficulty;
    uint32_t duco_numeric_result;
    char last_block_hash_str[DUINO_MSG_LEN_HASH+1];
    char expected_hash_str[DUINO_MSG_LEN_HASH+1];

    printf("Core1 Start\r\n");;

    /* run duino hash loop */
    while(1)
    {
        switch (loop_state){
            case ConServer :
                connect_to_server(socket_num, close_flag);
                close_flag = 0;
                loop_state = GetHash;
                break;

            case GetHash :
                ret = get_hash_data_from_server(socket_num, g_ethernet_buf_core1, ETHERNET_BUF_MAX_SIZE);
                if (ret < 0)
                {
                    printf("get_hash_data_from_server error ret = %d\r\n\r\n", ret);
                }
                total_time_start = time_us_32();

                ret = get_duino_hash_data(g_ethernet_buf_core1, ",", last_block_hash_str, expected_hash_str, &difficulty);
                printf("[core1] get_duino_hash_data ret = %d\r\n\r\n", ret);
                if (ret != 0)
                {
                    loop_state = ConServer;
                    close_flag = 1;
                    fail_count++;
                    if (fail_count == FAIL_COUNT)
                    {
                        printf("fail_count == 5 reset device\r\n");
                        sleep_ms(100);
                        NVIC_SystemReset();
                    }
                }
                else
                {
                    printf("[core1] difficulty: %d\r\n", difficulty);
                    printf("[core1] last_block_hash_str = %s\r\n", last_block_hash_str);
                    printf("[core1] expected_hash_str = %s\r\n", expected_hash_str);
                    fail_count = 0;
                    loop_state = CalHash;
                }
                break;

            case CalHash :
                elapsed_time_s = calculate_hash_core1(socket_num, difficulty, last_block_hash_str, expected_hash_str, &duco_numeric_result, &hashrate);
                loop_state = SendHash;
                break;

            case SendHash :
                ret = getSn_SR(socket_num);
                printf("[core1] getSn_SR(socket_num) ret = 0x%0X\r\n", ret);
                if (ret == SOCK_ESTABLISHED)
                {
                    if(getSn_IR(socket_num) & Sn_IR_CON)	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
                    {
                        printf("[core1] getSn_IR(socket_num) & Sn_IR_CON\r\n"); 
			                  setSn_IR(socket_num, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
                    }
                }
                else
                {
                    printf("[core1] getSn_SR(socket_num) != SOCK_ESTABLISHED\r\n");
                    close_flag = 1;
                    send_flag = 1;
                    loop_state = ConServer;
                    break;
                }

                send_hash_result_to_server(socket_num, duco_numeric_result, hashrate, g_ethernet_buf_core1, ETHERNET_BUF_MAX_SIZE);
                total_time_end = time_us_32();

                ret = recv_data_from_server(socket_num, g_ethernet_buf_core1, ETHERNET_BUF_MAX_SIZE, US_TIMER_WAIT_TIMEOUT);
                printf("[core1] SendHash Recv = %s\r\n", g_ethernet_buf_core1);
                if (ret < 0)
                {
                    printf("[core1] failed send hash data to server\r\n");
                    close_flag = 1;
                    loop_state = ConServer;
                }
                else
                {
                    if (0 == strncmp(g_ethernet_buf_core1, "GOOD", 4)) {
                        printf("\r\n[core1] GOOD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  \r\n\r\n", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);
                    }
                    else if (0 == strncmp(g_ethernet_buf_core1, "BAD", 3)) {
                        printf("\r\n[core1] BAD share #%d: %d, hashrate:%.2fKH/s (%.2fs)  ", hash_cnt, duco_numeric_result, hashrate/1000, elapsed_time_s);                      
                    }
                    loop_state = GetHash;
                }
                printf("[core1] get hash to send result time = %dms\r\n", (total_time_end - total_time_start) / 1000);
                hash_cnt++;
                break;
        }
    }/* run duino loop */ 
    
    printf("Duino failed...\r\n");
    while(1);
}

void update_pool(void)
{
    char* pBody = NULL;
    size_t bodyLen;
    uint32_t port = 0;

    uint8_t ip[4] = {0,0,0,0};
    char s_ip[20];
    char * pOutValue = NULL;
    uint32_t outValueLength = 0U;
    JSONStatus_t result = JSONSuccess;

    g_http_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_NONE; // not used client certificate
    g_http_tls_context.clica_option = 0;                        // not used Root CA verify

    memset(g_ethernet_buf_core0, 0x00, ETHERNET_BUF_MAX_SIZE);
    http_get(SOCKET_HTTP, g_ethernet_buf_core0, DUINO_HTTP_GET_URL, &g_http_tls_context, &pBody, &bodyLen);

    result = JSON_Validate( pBody, bodyLen );
    if( result == JSONSuccess )
    {
        JSON_Search( ( char * )pBody, bodyLen, "ip", sizeof("ip")-1, &pOutValue, (size_t *)&outValueLength);
        sprintf(s_ip,"%.*s", outValueLength, pOutValue);
        printf("\r\n Host ip  :%.*s ", outValueLength, pOutValue);
        inet_addr_(s_ip, ip);

        JSON_Search( ( char * )pBody, bodyLen, "port", sizeof("port")-1, &pOutValue, (size_t *)&outValueLength);
        port = ( uint32_t ) strtoul( pOutValue, NULL, 10 );
        printf("\r\n Port     :%d ", port);

        set_duino_host_addr_info(ip, &port);
    }
    else
    {
        printf("\r\nThe json document is invalid!!: %d", result);
    }

}

void connect_to_server(uint8_t socket_num, uint32_t close_flag)
{
    int ret, size;
    uint8_t server_ver[16];

    printf("socket_num = %d\r\n", socket_num);

    if (close_flag)
      close(socket_num);

       /* connect to server */
    ret = socket(socket_num, Sn_MR_TCP, 0, 0);
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
    
    recv_data_from_server(socket_num, server_ver, 16, 0);
    printf("Connected to the server. Server version: %s\r\n", server_ver);
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

int get_hash_data_from_server(uint8_t socket_num, uint8_t *recv_buf, uint32_t recv_buf_size)
{
    int ret;

    ret = send(socket_num, send_req_str, strlen(send_req_str));
    printf("Send Data = %s, len = %d\r\n", send_req_str, strlen(send_req_str));
    ret = recv_data_from_server(socket_num, recv_buf, recv_buf_size, US_TIMER_WAIT_TIMEOUT);

    return ret;
}


float calculate_hash_core0(uint8_t socket_num, uint32_t difficulty, uint8_t *last_block_hash_str, \
                    uint8_t *expected_hash_str, uint32_t *duco_numeric_result, float *hashrate)   //1 core full speed is about 36us per once hashing
{
    int ret;
    uint32_t hash_number;    
    uint32_t start_time = 0;
    uint32_t end_time = 0;
    unsigned long elapsed_time = 0; 
    float elapsed_time_s = 0;
    
    uint8_t expected_hash_arry[DUINO_HASH_ARRAY_LEN];
    uint8_t hashArray[DUINO_HASH_ARRAY_LEN];
    uint8_t duco_numeric_result_str[128];
    mbedtls_sha1_context sha1_ctx_base, sha1_ctx;

    start_time = time_us_32();  
    str_to_array(expected_hash_str, expected_hash_arry);

    /* run hash */

    mbedtls_sha1_init(&sha1_ctx_base);

    if( ( ret = mbedtls_sha1_starts_ret( &sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
        
    if ( ret = mbedtls_sha1_update_ret(&sha1_ctx_base, last_block_hash_str, strlen(last_block_hash_str) ) != 0 )
        printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);
        
    for (hash_number = 0; hash_number < difficulty; hash_number++)
    {
        memcpy(&sha1_ctx, &sha1_ctx_base, sizeof(mbedtls_sha1_context));
        memset(duco_numeric_result_str, 0x00, 128);
        sprintf(duco_numeric_result_str, "%d", hash_number);

        if ( ret = mbedtls_sha1_update_ret(&sha1_ctx, duco_numeric_result_str, strlen(duco_numeric_result_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        if ( ret = mbedtls_sha1_finish_ret(&sha1_ctx, hashArray) != 0 )
            printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

        if(!(memcmp(expected_hash_arry, hashArray, 20)))
        {
            *duco_numeric_result = hash_number;
            printf("Core0 Find Hash %d\r\n", *duco_numeric_result);
            break;
        }
    }
    if (hash_number >= difficulty)
    {
      printf("Not Find Core0 wait hash_number = %d\r\n", hash_number);
    }
    mbedtls_sha1_free(&sha1_ctx_base);
    mbedtls_sha1_free(&sha1_ctx);

    end_time = time_us_32();
    elapsed_time   = end_time- start_time;
    elapsed_time_s = (elapsed_time * .000001f);

    *hashrate = ((*duco_numeric_result) / elapsed_time_s);
    printf("hashrate:%.2fKH/s (%.2fs)\r\n", (*hashrate)/1000, elapsed_time_s);

    return elapsed_time_s;
}


float calculate_hash_core1(uint8_t socket_num, uint32_t difficulty, uint8_t *last_block_hash_str, \
                    uint8_t *expected_hash_str, uint32_t *duco_numeric_result, float *hashrate)   //1 core full speed is about 36us per once hashing
{
    int ret;
    uint32_t hash_number;    
    uint32_t start_time = 0;
    uint32_t end_time = 0;
    unsigned long elapsed_time = 0; 
    float elapsed_time_s = 0;
    
    uint8_t expected_hash_arry[DUINO_HASH_ARRAY_LEN];
    uint8_t hashArray[DUINO_HASH_ARRAY_LEN];
    uint8_t duco_numeric_result_str[128];
    mbedtls_sha1_context sha1_ctx_base, sha1_ctx;

    start_time = time_us_32();  
    str_to_array(expected_hash_str, expected_hash_arry);

    /* run hash */

    mbedtls_sha1_init(&sha1_ctx_base);

    if( ( ret = mbedtls_sha1_starts_ret( &sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
        
    if ( ret = mbedtls_sha1_update_ret(&sha1_ctx_base, last_block_hash_str, strlen(last_block_hash_str) ) != 0 )
        printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

    for (hash_number = 0; hash_number < difficulty; hash_number++)
    {
        memcpy(&sha1_ctx, &sha1_ctx_base, sizeof(mbedtls_sha1_context));
        memset(duco_numeric_result_str, 0x00, 128);
        sprintf(duco_numeric_result_str, "%d", hash_number);

        if ( ret = mbedtls_sha1_update_ret(&sha1_ctx, duco_numeric_result_str, strlen(duco_numeric_result_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        if ( ret = mbedtls_sha1_finish_ret(&sha1_ctx, hashArray) != 0 )
            printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

        if(!(memcmp(expected_hash_arry, hashArray, 20)))
        {
            *duco_numeric_result = hash_number;
            printf("Core0 Find Hash %d\r\n", *duco_numeric_result);
            break;
        }
    }
    if (hash_number >= difficulty)
    {
      printf("Not Find Core0 wait hash_number = %d\r\n", hash_number);
    }
    mbedtls_sha1_free(&sha1_ctx_base);
    mbedtls_sha1_free(&sha1_ctx);

    end_time = time_us_32();
    elapsed_time   = end_time- start_time;
    elapsed_time_s = (elapsed_time * .000001f);

    *hashrate = ((*duco_numeric_result) / elapsed_time_s);
    printf("hashrate:%.2fKH/s (%.2fs)\r\n", (*hashrate)/1000, elapsed_time_s);

    return elapsed_time_s;
}


void send_hash_result_to_server(uint8_t socket_num, uint32_t duco_numeric_result, float hashrate, uint8_t *send_buf, uint32_t send_buf_size)
{
    /* send msg: response hash data */
    set_duino_res_msg(socket_num, duco_numeric_result, hashrate, send_buf, send_buf_size);
    send(socket_num, send_buf, strlen(send_buf));
    printf("%s\r\n", send_buf);
}

void rp2040_get_board_id(void)
{
    pico_get_unique_board_id_string(rp2040_id, 8);
}
