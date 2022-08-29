# Install script for directory: /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/ARM/bin/arm-none-eabi-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._aes.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._aesni.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._arc4.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._aria.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._asn1.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._asn1write.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._base64.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._bignum.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._blowfish.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._bn_mul.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._camellia.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ccm.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._certs.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._chacha20.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._chachapoly.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._check_config.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._cipher.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._cipher_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._cmac.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._compat-1.3.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._config.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._config_psa.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ctr_drbg.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._debug.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._des.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._dhm.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ecdh.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ecdsa.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ecjpake.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ecp.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ecp_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._entropy.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._entropy_poll.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._error.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._gcm.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._havege.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._hkdf.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._hmac_drbg.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._md.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._md2.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._md4.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._md5.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._md_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._memory_buffer_alloc.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._net.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._net_sockets.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._nist_kw.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._oid.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._padlock.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._pem.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._pk.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._pk_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._pkcs11.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._pkcs12.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._pkcs5.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._platform.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._platform_time.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._platform_util.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._poly1305.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._psa_util.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ripemd160.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._rsa.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._rsa_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._sha1.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._sha256.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._sha512.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ssl.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ssl_cache.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ssl_ciphersuites.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ssl_cookie.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ssl_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._ssl_ticket.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._threading.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._timing.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._version.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._x509.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._x509_crl.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._x509_crt.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._x509_csr.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/._xtea.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/aes.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/aesni.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/arc4.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/aria.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/asn1.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/asn1write.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/base64.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/bignum.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/blowfish.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/bn_mul.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/camellia.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ccm.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/certs.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/chacha20.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/chachapoly.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/check_config.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/cipher.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/cipher_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/cmac.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/compat-1.3.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/config.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/config_psa.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ctr_drbg.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/debug.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/des.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/dhm.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ecdh.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ecdsa.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ecjpake.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ecp.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ecp_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/entropy.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/entropy_poll.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/error.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/gcm.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/havege.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/hkdf.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/hmac_drbg.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/md.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/md2.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/md4.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/md5.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/md_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/net.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/net_sockets.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/nist_kw.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/oid.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/padlock.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/pem.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/pk.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/pk_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/pkcs11.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/pkcs12.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/pkcs5.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/platform.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/platform_time.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/platform_util.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/poly1305.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/psa_util.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ripemd160.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/rsa.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/rsa_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/sha1.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/sha256.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/sha512.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ssl.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ssl_cache.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ssl_cookie.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ssl_internal.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/ssl_ticket.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/threading.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/timing.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/version.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/x509.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/x509_crl.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/x509_crt.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/x509_csr.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/mbedtls/xtea.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_accel_driver.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_compat.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_config.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_driver_common.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_entropy_driver.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_extra.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_platform.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_se_driver.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_sizes.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_struct.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_types.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/._crypto_values.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_accel_driver.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_compat.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_config.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_driver_common.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_entropy_driver.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_extra.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_platform.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_se_driver.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_sizes.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_struct.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_types.h"
    "/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/mbedtls/include/psa/crypto_values.h"
    )
endif()

