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

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_HTTP 0
#define SOCKET_TCP  1

/* Port */
#define PORT_SSL 443

/* HTTP */
#define HTTP_BUF_MAX_SIZE (1024 * 2)


/* Duino coin */
#define DUINO_CHIP_ID           "1234b" 
#define DUINO_USER_NAME         "USER_NAME"
#define DUINO_MINER_KEY         "1234"
    // Change the part in brackets if you want to set a custom miner name (use Auto to autogenerate, None for no name)
#define DUINO_RIG_IDENTIFIER    "None" 
    // Do not change the lines below. These lines are static and dynamic variables
    //  that will be used by the program for counters and measurements.
#define DUINO_HTTP_GET_URL      "https://server.duinocoin.com/getPool"
#define DUINO_MINER_VER         "3.18"                  
#define DUINO_MINER_BANNER      "Official W5100 Miner"  

#define DUINO_MSG_LEN_HASH  41
#define DUINO_MSG_LEN_TOTAL 85


/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/* duino host */
typedef struct Duino_host_t
{
    uint32_t port;   ///< host port
    uint8_t  ip[4];   ///< host ip
}Duino_host;

static Duino_host g_duino_host; 

uint8_t  g_recv_buf[ETHERNET_BUF_MAX_SIZE];
uint8_t  g_send_req_hash[ETHERNET_BUF_MAX_SIZE];
uint8_t  g_send_res_hash[ETHERNET_BUF_MAX_SIZE];

/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip =  {192, 168, 11, 2},                     // IP address
        .sn =  {255, 255, 255, 0},                    // Subnet Mask
        .gw =  {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};

/* Http */
static tlsContext_t g_http_tls_context;
static uint8_t g_http_buf[HTTP_BUF_MAX_SIZE] = {
    0,
};

/* ssl */
static mbedtls_ctr_drbg_context g_ctr_drbg;
static mbedtls_ssl_config g_conf;
static mbedtls_ssl_context g_ssl;

/* Timer  */
static volatile uint32_t g_msec_cnt = 0;
static mbedtls_sha1_context sha1_ctx, sha1_ctx_base;
sha1_context br_sha1_ctx, br_sha1_ctx_base;

/* duino hash*/
static char last_block_hash_str[DUINO_MSG_LEN_HASH];
static char expected_hash_str[DUINO_MSG_LEN_HASH];  
static uint32_t difficulty = 0;
static uint8_t hashArray[20];
static uint8_t duco_numeric_result_str[128];
static uint8_t expected_hash_arry[20];
static uint32_t duco_numeric_result;



/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);


static uint8_t str_to_array(uint8_t * str, uint8_t * hex);

/* duino */

static uint8_t str_to_array(uint8_t * str, uint8_t * hex);

static void set_duino_req_msg (void);
static void set_duino_res_msg (uint32_t duco_numeric_result, float hashrate);
static int  get_duino_hash_data (char* data, char* separator, char* last_block_hash, char* expected_hash, uint32_t* difficulty);
extern void set_duino_host_addr_info (uint8_t* addr, uint32_t* port);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */

int main()
{

    int ret, i;    
    uint16_t len = 0;
    uint32_t size= 0;
    uint32_t retval  = 0;
    uint32_t hash_cnt= 1;
    float hashrate   = 0;

    char recvBuf[1024];
    void* pBuffer= recvBuf;
    memset(pBuffer, 0x00, 1024);

    float start_time =0;
    unsigned long elapsed_time= 0; 
    float elapsed_time_s= 0;

    /* Initialize */
    set_clock_khz();

    stdio_init_all();

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
            retval = DHCP_run();

            if (retval == DHCP_IP_LEASED)
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
    g_http_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_NONE; // not used client certificate
    g_http_tls_context.clica_option = 0;                        // not used Root CA verify

    /* send msg [GET] */
    http_get(SOCKET_HTTP, g_http_buf, DUINO_HTTP_GET_URL, &g_http_tls_context);
 
    /* connect to server */
    ret = socket(SOCKET_TCP, Sn_MR_TCP, g_duino_host.port, 0);
    if (ret != SOCKET_TCP)
    {
        printf(" failed\n  ! socket returned %d\n\n", ret);
        while (1);
    }
    else
    {
        set_duino_req_msg();
    }

    /* run duino hash loop */
    while(1)
    {
        /* send msg: request hash data */
        ret= connect(SOCKET_TCP, g_duino_host.ip,  g_duino_host.port);
        if (ret == SOCK_FATAL)
        {
            printf(" failed\n  ! connect returned %d\n\n", ret);
            while(1);
        }
        printf("Asking for a new job for user: %s", DUINO_USER_NAME);

        send(SOCKET_TCP, g_send_req_hash, strlen(g_send_req_hash));

         /* wait msg requested hash data */
        while(1)
        {
            if (getSn_RX_RSR(SOCKET_TCP) > 0)  size = recv(SOCKET_TCP, pBuffer, 1024);
            if (size != 0)
            {
                ret= get_duino_hash_data(pBuffer, ",", last_block_hash_str, expected_hash_str, &difficulty);                                                                      
                if (!ret) break;

                sleep_ms(10);
            }
        }
        memset(pBuffer, 0x00, 1024);

        /* run hash */
        start_time= time_us_32();
        mbedtls_sha1_init(&sha1_ctx_base);

        if( ( ret = mbedtls_sha1_starts_ret( &sha1_ctx_base ) ) != 0 )
            printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);
            
        if ( ret = mbedtls_sha1_update_ret(&sha1_ctx_base, last_block_hash_str, strlen(last_block_hash_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        str_to_array(expected_hash_str, expected_hash_arry);
        
        for (duco_numeric_result = 0; duco_numeric_result < difficulty; duco_numeric_result++)
        {
            memcpy(&sha1_ctx, &sha1_ctx_base, sizeof(mbedtls_sha1_context));
            memset(duco_numeric_result_str, 0x00, 128);
            sprintf(duco_numeric_result_str, "%d", duco_numeric_result);

            if ( ret = mbedtls_sha1_update_ret(&sha1_ctx, duco_numeric_result_str, strlen(duco_numeric_result_str) ) != 0 )
                printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

            if ( ret = mbedtls_sha1_finish_ret(&sha1_ctx, hashArray) != 0 )
                printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

            if(!(memcmp(expected_hash_arry, hashArray, 20)))
            {
                elapsed_time= time_us_32() - start_time;
                elapsed_time_s = elapsed_time * .000001f;
                hashrate = duco_numeric_result / elapsed_time_s;
                
                break;
            }
        }

        /* send msg: response hash data */
        set_duino_res_msg(duco_numeric_result, hashrate);
        send(SOCKET_TCP, g_send_res_hash, strlen(g_send_res_hash));

        /* wait msg response GOOD */
        size= 0;
        while (1)
        {
            if (getSn_RX_RSR(SOCKET_TCP) > 0)  size = recv(SOCKET_TCP, pBuffer, 1024);
            if (size != 0)
            {
                if (strcmp(pBuffer, "GOOD"))  break;                  
            }
            sleep_ms(10);
        }
        printf("\r\n GOOD share #%d: %d, hashrate:%0.2fKH/s (%0.2fs)  \r\n\r\n", hash_cnt, duco_numeric_result, hashrate, elapsed_time_s);
        hash_cnt++;

        memset(pBuffer, 0x00, 1024);

    }/* run duino loop */ 
    
    printf("Duino failed...\r\n");
    while(1);
    
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
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
    int temp_num= 0;

    char temp_data[DUINO_MSG_LEN_TOTAL];
    char temp_out1[DUINO_MSG_LEN_HASH];
    char temp_out2[DUINO_MSG_LEN_HASH];

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
    temp_num= atoi(&temp_data[idx_cnt1]) ;
    if (temp_num==0) return 3;

    *difficulty= temp_num*100+1;

    return 0;
}

/* connect to duino server */
static void set_duino_req_msg (void)
{
    memset(g_send_req_hash, 0x00, ETHERNET_BUF_MAX_SIZE);
    sprintf(g_send_req_hash, "JOB,%s,ESP8266,%s\n", DUINO_USER_NAME, DUINO_MINER_KEY);
}

static void set_duino_res_msg(uint32_t duco_numeric_result, float hashrate)
{
    memset(g_send_res_hash, 0x00, ETHERNET_BUF_MAX_SIZE );
    sprintf(g_send_res_hash, "%d,%0.2f,%s %s,%s,DUCOID%s\n", duco_numeric_result , hashrate     , DUINO_MINER_BANNER, DUINO_MINER_VER
                                                           , DUINO_RIG_IDENTIFIER, DUINO_CHIP_ID);
}

void set_duino_host_addr_info (uint8_t* addr, uint32_t* port)
{

    memcpy(&g_duino_host.ip[0], addr, 4);
    g_duino_host.port= *port;
    printf("\r\n > host addr: %d.%d.%d.%d/ %d \r\n", g_duino_host.ip[0], g_duino_host.ip[1]
                                                   , g_duino_host.ip[2], g_duino_host.ip[3], g_duino_host.port );
}

