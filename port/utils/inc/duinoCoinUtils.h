#ifndef __DUINOCOINUTILS_H_
#define __DUINOCOINUTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

static uint8_t str_to_array(uint8_t * str, uint8_t * hex);
uint32_t calculateHashCore0(uint8_t * last_block_hash_str, uint8_t * expected_hash_str, uint32_t difficulty);
uint32_t calculateHashCore1(uint8_t * last_block_hash_str, uint8_t * expected_hash_str, uint32_t difficulty);

#ifdef __cplusplus
}
#endif


#endif