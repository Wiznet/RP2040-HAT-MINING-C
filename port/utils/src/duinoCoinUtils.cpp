#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
// #include "mbedtls/compat-2.x.h"
#include "mbedtls/sha1.h"

#include "duinoCoinUtils.h"
#include "hwSetupUtils.h"

static uint32_t hashDelayUs = 50;

#define HASH_DELAY() sleep_us(hashDelayUs)
#define LED_TOGGLE_SPEED 1000

typedef enum _DucoState{
    DUCO_WAIT_JOB,
    DUCO_CALCULATING,
    DUCO_JOB_DONE,
}DucoState;

static mbedtls_sha1_context core0_sha1_ctx, core0_sha1_ctx_base;
static mbedtls_sha1_context core1_sha1_ctx, core1_sha1_ctx_base;

void setHashDelay(uint32_t us){
    hashDelayUs = us;
}

uint32_t calculateHashCore0(uint8_t * last_block_hash_str, uint8_t * expected_hash_str, uint32_t difficulty){

    static uint8_t duco_numeric_result_str[16];
    static uint8_t expected_hash_arry[20];
    static uint8_t hash_result[20];

    uint32_t hash_number = 0;
    uint32_t ret;
    
    printf("Core 0: Calculation starts\n");
    //convert string to hex code
    str_to_array(expected_hash_str, expected_hash_arry);
    for (int i = 0 ; i < 20; i++)
        printf("%02x ",expected_hash_arry[i]);
        printf("\n");
    // startTime = time_us_32();
    mbedtls_sha1_init(&core0_sha1_ctx_base);

    //hash start???    
    if( ( ret = mbedtls_sha1_starts_ret( &core0_sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);

    //update last block hash string;
    if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx_base, last_block_hash_str, strlen((char *)last_block_hash_str) ) != 0 )
        printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

    //find duino coin hash number
    for (hash_number = 0; hash_number < difficulty * 100 + 1; hash_number++){
        memcpy(&core0_sha1_ctx, &core0_sha1_ctx_base, sizeof(mbedtls_sha1_context));
        //initialize the hash number character arry
        memset(duco_numeric_result_str, 0x00, 16);
        sprintf((char *)duco_numeric_result_str, "%d", hash_number);

        //add hash number to the last block hash
        if ( ret = mbedtls_sha1_update_ret(&core0_sha1_ctx, duco_numeric_result_str, strlen((char *)duco_numeric_result_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        //calculate hash result
        if ( ret = mbedtls_sha1_finish_ret(&core0_sha1_ctx, hash_result) != 0 )
            printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

        if(hash_number % LED_TOGGLE_SPEED == 0) {outsideLedToggle();}
        //if expected_hash_array equals to the hash result, job is done.
        if(!(memcmp(expected_hash_arry, hash_result, 20)))
        {   
            for (int i = 0 ; i < 20; i++)
            printf("%02x ",hash_result[i]);
            printf("\n");
            printf("Core0 Hash result %d\r\n", hash_number);
            break;
        }// where is if not!
        HASH_DELAY();
    }

    // onBoardLedOff();

    //free the sha1 memory
    mbedtls_sha1_free(&core0_sha1_ctx_base);

    //return hash number
    return hash_number;
}

uint32_t calculateHashCore1(uint8_t * last_block_hash_str, uint8_t * expected_hash_str, uint32_t difficulty){
    
    static uint8_t duco_numeric_result_str[16];
    static uint8_t expected_hash_arry[20];
    static uint8_t hash_result[20];

    uint32_t hash_number = 0;
    uint32_t ret;
    
    printf("Core 1: Calculation starts\n");
    //convert string to hex code
    str_to_array(expected_hash_str, expected_hash_arry);
    for (int i = 0 ; i < 20; i++)
        printf("%02x ",expected_hash_arry[i]);
        printf("\n");
    // startTime = time_us_32();
    mbedtls_sha1_init(&core1_sha1_ctx_base);

    //hash start???    
    if( ( ret = mbedtls_sha1_starts_ret( &core1_sha1_ctx_base ) ) != 0 )
        printf("Failed mbedtls_sha1_starts_ret = %d\r\n", ret);

    //update last block hash string;
    if ( ret = mbedtls_sha1_update_ret(&core1_sha1_ctx_base, last_block_hash_str, strlen((char *)last_block_hash_str) ) != 0 )
        printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

    //find duino coin hash number
    for (hash_number = 0; hash_number < difficulty * 100 + 1; hash_number++){
        memcpy(&core1_sha1_ctx, &core1_sha1_ctx_base, sizeof(mbedtls_sha1_context));
        //initialize the hash number character arry
        memset(duco_numeric_result_str, 0x00, 16);
        sprintf((char *)duco_numeric_result_str, "%d", hash_number);

        //add hash number to the last block hash
        if ( ret = mbedtls_sha1_update_ret(&core1_sha1_ctx, duco_numeric_result_str, strlen((char *)duco_numeric_result_str) ) != 0 )
            printf("Failed mbedtls_sha1_update_ret = %d\r\n", ret);

        //calculate hash result
        if ( ret = mbedtls_sha1_finish_ret(&core1_sha1_ctx, hash_result) != 0 )
            printf("Failed mbedtls_sha1_finish_ret = %d\r\n", ret);

        if(hash_number % LED_TOGGLE_SPEED == 0) {onBoardLedToggle();}
        //if expected_hash_array equals to the hash result, job is done.
        if(!(memcmp(expected_hash_arry, hash_result, 20)))
        {
            for (int i = 0 ; i < 20; i++)
            printf("%02x ",hash_result[i]);
            printf("\n");
            printf("Core1 Hash result: %d\r\n", hash_number);
            break;
        }
        HASH_DELAY();
    }

    // onBoardLedOff();

    //free the sha1 memory
    mbedtls_sha1_free(&core1_sha1_ctx_base);

    //return hash number
    return hash_number;
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