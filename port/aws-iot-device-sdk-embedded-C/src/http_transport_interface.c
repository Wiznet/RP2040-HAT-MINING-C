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
#include <ctype.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

/* JSON API header. */
#include "core_json.h"

#include "http_transport_interface.h"
#include "http_parser.h"
#include "core_http_client_private.h"

#include "dns_interface.h"
#include "timer_interface.h"
#include "util.h"


/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Connection informaion */
NetworkContext_t g_http_network_context;
TransportInterface_t g_http_transport_interface;
http_config_t g_http_config;

/* SSL context */
tlsContext_t *g_http_tls_context_ptr;

void inet_addr_(uint8_t * addr, uint8_t * ip);

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Common */
int32_t http_send_request(TransportInterface_t *pTransportInterface, uint8_t *buffer, char *pMethod, http_config_t *http_config)
{
    /* Return value of this method. */
    int32_t returnStatus = 0;

    uint32_t currentReceived = 0;
    uint32_t currentTotalLen = 0;

    /* Configurations of the initial request headers that are passed to
     * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t requestInfo;
    /* Represents a response returned from an HTTP server. */
    HTTPResponse_t response;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t requestHeaders;

    /* Return value of all methods from the HTTP Client library API. */
    HTTPStatus_t httpStatus = HTTPSuccess;

    /* Initialize all HTTP Client library API structs to 0. */
    (void)memset(&requestInfo, 0, sizeof(requestInfo));
    (void)memset(&response, 0, sizeof(response));
    (void)memset(&requestHeaders, 0, sizeof(requestHeaders));

    /* Initialize the request object. */
    requestInfo.pHost = http_config->http_domain;
    requestInfo.hostLen = http_config->http_domain_len;
    requestInfo.pMethod = pMethod;
    requestInfo.methodLen = strlen(pMethod);
    requestInfo.pPath = http_config->http_path;
    requestInfo.pathLen = http_config->http_path_len;

#if 0 // not used
	/* Set "Connection" HTTP header to "keep-alive" so that multiple requests
	 * can be sent over the same established TCP connection. */
	requestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;
#endif

    /* Set the buffer used for storing request headers. */
    requestHeaders.pBuffer = buffer;
    requestHeaders.bufferLen = HTTP_BUF_MAX_SIZE;

    /* Initialize the response object. The same buffer used for storing
     * request headers is reused here. */

    response.pBuffer = buffer;
    response.bufferLen = HTTP_BUF_MAX_SIZE;
    response.getTime = (HTTPClient_GetCurrentTimeFunc_t)millis;

    httpStatus = HTTPClient_InitializeRequestHeaders(&requestHeaders, &requestInfo);
    if (httpStatus != HTTPSuccess)
        printf("Failed to initialize HTTP request headers: Error=%s.\n", HTTPClient_strerror(httpStatus));

    httpStatus = HTTPClient_AddHeader(&requestHeaders,
                                      HTTP_CONTENT_TYPE_FIELD,
                                      HTTP_CONTENT_TYPE_FIELD_LEN,
                                      HTTP_CONTENT_TYPE_VALUE,
                                      HTTP_CONTENT_TYPE_VALUE_LEN);
    if (httpStatus != HTTPSuccess)
        printf("Failed to initialize HTTP request headers: Error=%s.\n", HTTPClient_strerror(httpStatus));

    if ((HTTP_REQUEST_KEEP_ALIVE_FLAG & requestInfo.reqFlags) == 0U)
    {
        httpStatus = HTTPClient_AddHeader(&requestHeaders,
                                          HTTP_CONNECTION_FIELD,
                                          HTTP_CONNECTION_FIELD_LEN,
                                          HTTP_CONNECTION_CLOSE_VALUE,
                                          HTTP_CONNECTION_CLOSE_VALUE_LEN);
        if (httpStatus != HTTPSuccess)
            printf("Failed to initialize HTTP request headers: Error=%s.\n", HTTPClient_strerror(httpStatus));
    }

    if (httpStatus == HTTPSuccess)
    {
        printf("Sending HTTP %.*s request to %.*s%.*s...\n",
               (int32_t)requestInfo.methodLen, requestInfo.pMethod,
               (int32_t)requestInfo.hostLen, requestInfo.pHost,
               (int32_t)requestInfo.pathLen, requestInfo.pPath);

        printf("Request Headers:\n%.*s\n"
               "Request Body:\n%.*s\n",
               (int32_t)requestHeaders.headersLen, (char *)requestHeaders.pBuffer,
               (int32_t)http_config->bodyLen, http_config->bodyDataPtr);

        /* Send the request and receive the response. */
        httpStatus = HTTPClient_Send(pTransportInterface,
                                     &requestHeaders,
                                     (uint8_t *)http_config->bodyDataPtr,
                                     http_config->bodyLen,
                                     &response,
                                     0);
    }
    else
    {
        printf("Failed to initialize HTTP request headers: Error=%s.\n", HTTPClient_strerror(httpStatus));
    }

    printf("Response Headers Length: %d\n", response.headersLen);
    printf("Response Content Length: %d\n\n", response.contentLength);

    currentTotalLen = response.bodyLen;

    if (httpStatus == HTTPInsufficientMemory)
    {
        printf("Response buffer has insufficient\n");
        printf("Response Content Length: %d\n\n", response.contentLength);
        printf("Response Body Length: %d\n", response.bodyLen);
        printf("Response Body:\n%.*s\n", response.bodyLen, response.pBody);

        while (1)
        {
            memset(buffer, 0, HTTP_BUF_MAX_SIZE);
            currentReceived = pTransportInterface->recv(pTransportInterface->pNetworkContext, buffer, HTTP_BUF_MAX_SIZE);

            printf("Current Received: %d\n", currentReceived);
            printf("Current Received Total Length: %d/%d\n", currentTotalLen + currentReceived, response.contentLength);

            if (currentReceived > 0)
            {
                printf("\n%.*s\n", response.bodyLen, response.pBody);

                currentTotalLen += currentReceived;
                if (currentTotalLen >= response.contentLength)
                {
                    httpStatus = HTTPSuccess;
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    else if (httpStatus == HTTPSuccess)
    {    
        http_config->bodyDataPtr= response.pBody;
        http_config->bodyLen    = response.bodyLen;

        printf("Response Content Length: %d\n", response.contentLength);
        printf("Response Body:\n%.*s\n", response.bodyLen, response.pBody);
    }

    if (httpStatus != HTTPSuccess)
    {
        returnStatus = -1;
    }

    return returnStatus;
}

/* HTTP */
int8_t http_connect(uint8_t sock, http_config_t *http_config)
{
    int8_t ret = SOCK_ERROR;

    /* Open TCP socket */
    ret = socket(g_http_network_context.socketDescriptor, Sn_MR_TCP, 0, 0);

    if (ret != sock)
    {
        printf(" failed\n  ! socket returned %d\n\n", ret);

        return -1;
    }

    /* Connect to HTTP server */
    ret = connect(g_http_network_context.socketDescriptor, http_config->http_ip, http_config->http_port);

    if (ret != SOCK_OK)
    {
        printf(" failed\n  ! connect returned %d\n\n", ret);

        return -1;
    }

    return 0;
}

int8_t http_close(uint8_t sock, http_config_t *http_config)
{
    int8_t ret = SOCK_ERROR;

    if (http_config->ssl_flag)
    {
        mbedtls_ssl_close_notify(&g_http_tls_context_ptr->ssl);
#ifdef MBEDTLS_ENTROPY_C
        mbedtls_entropy_free(&g_http_tls_context_ptr->entropy);
#endif // MBEDTLS_ENTROPY_C
        mbedtls_x509_crt_free(&g_http_tls_context_ptr->cacert);
        mbedtls_x509_crt_free(&g_http_tls_context_ptr->clicert);
        mbedtls_pk_free(&g_http_tls_context_ptr->pkey);
        mbedtls_ssl_free(&g_http_tls_context_ptr->ssl);
        mbedtls_ssl_config_free(&g_http_tls_context_ptr->conf);
        mbedtls_ctr_drbg_free(&g_http_tls_context_ptr->ctr_drbg);
        g_http_tls_context_ptr = NULL;
    }
    ret = disconnect(sock);
    http_config->http_state = HTTP_IDLE;
    if (ret != SOCK_OK)
    {
        printf(" failed\n  ! disconnect returned %d\n\n", ret);

        return ret;
    }

    return ret;
}

int32_t http_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
    {
        size = send(pNetworkContext->socketDescriptor, (uint8_t *)pBuffer, bytesToSend);
    }

    return size;
}

int32_t http_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
{
    int32_t size = 0;
    uint32_t tickStart = millis();

    do
    {
        if (getSn_RX_RSR(pNetworkContext->socketDescriptor) > 0)
            size = recv(pNetworkContext->socketDescriptor, pBuffer, bytesToRecv);
        if (size != 0)
        {
            break;
        }
        sleep_ms(10);
    } while ((millis() - tickStart) <= HTTP_TIMEOUT);

    return size;
}

int32_t http_get(uint8_t sock, uint8_t *buffer, char *http_url, tlsContext_t *tls_context, char** pResBody, size_t* resBodyLen)
{
    int32_t ret = -1;
    uint8_t *pPath, *pDomain;
    uint32_t pPathLen, pDomainLen;
    uint32_t port = 0;

    if (g_http_config.http_state != HTTP_IDLE)
        return -1;
    memset(&g_http_config, 0, sizeof(http_config_t));
    g_http_config.http_state = HTTP_RUNNING;

    ret = is_https(http_url);
    if (ret < 0)
    {
        http_close(sock, &g_http_config);

        return -1;
    }
    if (ret == 1)
        g_http_config.http_port = 443;
    else
        g_http_config.http_port = 80;
    g_http_config.ssl_flag = ret;

    ret = getUrlInfo(http_url, strlen(http_url), &pDomain, (size_t *)&pDomainLen, &pPath, (size_t *)&pPathLen, &port);
    if (ret)
    {
        http_close(sock, &g_http_config);

        return -1;
    }

    memset(g_http_config.http_path, 0, HTTP_DOMAIN_MAX_SIZE);
    memset(g_http_config.http_domain, 0, HTTP_DOMAIN_MAX_SIZE);

    memcpy(g_http_config.http_path, pPath, pPathLen);
    g_http_config.http_path_len = pPathLen;

    memcpy(g_http_config.http_domain, pDomain, pDomainLen);
    g_http_config.http_domain_len = pDomainLen;
    if (port)
        g_http_config.http_port = port;

    if (!is_ipaddr(g_http_config.http_domain, g_http_config.http_ip)) // IP
    {
        uint8_t dns_buf[512];
        ret = get_ipaddr_from_dns(g_http_config.http_domain, g_http_config.http_ip, dns_buf, DNS_TIMEOUT);
        if (ret != 1)
        {
            http_close(sock, &g_http_config);

            return -1;
        }
    }
    g_http_config.http_method = GET;

    g_http_network_context.socketDescriptor = sock;
    g_http_transport_interface.pNetworkContext = &g_http_network_context;

    if (g_http_config.ssl_flag == 0)
    {
        ret = http_connect(g_http_network_context.socketDescriptor, &g_http_config);
        if (ret != 0)
        {
            http_close(sock, &g_http_config);

            return HTTPNetworkError;
        }
        g_http_transport_interface.send = http_write;
        g_http_transport_interface.recv = http_read;
    }
    else if (g_http_config.ssl_flag == 1)
    {
        if (tls_context == NULL)
        {
            http_close(sock, &g_http_config);

            return -1;
        }
        ret = https_connect(g_http_network_context.socketDescriptor, &g_http_config, tls_context);
        if (ret != 0)
        {
            http_close(sock, &g_http_config);

            return HTTPNetworkError;
        }
        g_http_transport_interface.send = https_write;
        g_http_transport_interface.recv = https_read;
    }

    ret = http_send_request(&g_http_transport_interface,
                            buffer,
                            HTTP_METHOD_GET,
                            &g_http_config);

     
    *pResBody  = g_http_config.bodyDataPtr;
    *resBodyLen= g_http_config.bodyLen;

    http_close(sock, &g_http_config);
    if (ret != HTTPSuccess)
    {
        printf(" failed\n  ! http_send_request returned %d\n\n", ret);
    }

    return ret;
}

int32_t http_post(uint8_t sock, uint8_t *buffer, char *http_url, tlsContext_t *tls_context)
{
    int32_t ret = -1;
    uint8_t *pPath, *pDomain;
    uint32_t pPathLen, pDomainLen;
    uint32_t port = 0;

    if (g_http_config.http_state != HTTP_IDLE)
        return -1;
    memset(&g_http_config, 0, sizeof(http_config_t));
    g_http_config.http_state = HTTP_RUNNING;

    ret = is_https(http_url);
    if (ret < 0)
    {
        http_close(sock, &g_http_config);

        return -1;
    }
    if (ret == 1)
        g_http_config.http_port = 443;
    else
        g_http_config.http_port = 80;
    g_http_config.ssl_flag = ret;

    ret = getUrlInfo(http_url, strlen(http_url), &pDomain, (size_t *)&pDomainLen, &pPath, (size_t *)&pPathLen, &port);
    if (ret)
    {
        http_close(sock, &g_http_config);

        return -1;
    }

    memset(g_http_config.http_path, 0, HTTP_DOMAIN_MAX_SIZE);
    memset(g_http_config.http_domain, 0, HTTP_DOMAIN_MAX_SIZE);

    memcpy(g_http_config.http_path, pPath, pPathLen);
    g_http_config.http_path_len = pPathLen;

    memcpy(g_http_config.http_domain, pDomain, pDomainLen);
    g_http_config.http_domain_len = pDomainLen;
    if (port)
        g_http_config.http_port = port;

    if (!is_ipaddr(g_http_config.http_domain, g_http_config.http_ip)) // IP
    {
        uint8_t dns_buf[512];
        ret = get_ipaddr_from_dns(g_http_config.http_domain, g_http_config.http_ip, dns_buf, HTTP_TIMEOUT);
        if (ret != 1)
        {
            http_close(sock, &g_http_config);

            return -1;
        }
    }
    g_http_config.http_method = POST;

    g_http_network_context.socketDescriptor = sock;
    g_http_transport_interface.pNetworkContext = &g_http_network_context;

    if (g_http_config.ssl_flag == 0)
    {
        ret = http_connect(g_http_network_context.socketDescriptor, &g_http_config);
        if (ret != 0)
        {
            http_close(sock, &g_http_config);

            return HTTPNetworkError;
        }
        g_http_transport_interface.send = http_write;
        g_http_transport_interface.recv = http_read;
    }
    else if (g_http_config.ssl_flag == 1)
    {
        ret = https_connect(g_http_network_context.socketDescriptor, &g_http_config, tls_context);
        if (ret != 0)
        {
            http_close(sock, &g_http_config);

            return HTTPNetworkError;
        }
        g_http_transport_interface.send = https_write;
        g_http_transport_interface.recv = https_read;
    }

    ret = http_send_request(&g_http_transport_interface,
                            buffer,
                            HTTP_METHOD_POST,
                            &g_http_config);

    http_close(sock, &g_http_config);
    if (ret != HTTPSuccess)
    {
        printf(" failed\n  ! http_send_request returned %d\n\n", ret);

        return ret;
    }

    return ret;
}

/* HTTPS */
int8_t https_connect(uint8_t sock, http_config_t *http_config, tlsContext_t *tls_context)
{
    int8_t ret = -1;

    /* Initialize SSL context */
    ret = ssl_transport_init(tls_context, (int *)(intptr_t)sock, http_config->http_domain);
    if (ret != 0)
    {
        printf(" failed\n  ! wiz_tls_init returned %d\n\n", ret);

        return ret;
    }

    /* Connect to HTTPS server */
    ret = ssl_socket_connect_timeout(tls_context, http_config->http_ip, http_config->http_port, 0, HTTP_TIMEOUT);
    if (ret != 0)
    {
        printf(" failed\n  ! connect returned %d\n\n", ret);

        return ret;
    }
    g_http_tls_context_ptr = tls_context;

    return ret;
}

int32_t https_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
    {
        size = ssl_transport_write(g_http_tls_context_ptr, (uint8_t *)pBuffer, bytesToSend);
    }

    return size;
}

int32_t https_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
{
    int32_t size = 0;
    uint32_t tickStart = millis();

    do
    {
        if (getSn_RX_RSR(pNetworkContext->socketDescriptor) > 0)
            size = ssl_transport_read(g_http_tls_context_ptr, pBuffer, bytesToRecv);
        if (size != 0)
            break;
    } while ((millis() - tickStart) <= HTTP_TIMEOUT);

    return size;
}

HTTPStatus_t getUrlInfo(const char *pUrl,
                        size_t urlLen,
                        const char **pAddress,
                        size_t *pAddressLen,
                        const char **pPath,
                        size_t *pPathLen,
                        uint32_t *port)
{
    /* http-parser status. Initialized to 1 to signify failure. */
    int parserStatus = 1;
    struct http_parser_url urlParser;
    HTTPStatus_t httpStatus = HTTPSuccess;

    /* Sets all members in urlParser to 0. */
    http_parser_url_init(&urlParser);

    if ((pUrl == NULL) || (pAddress == NULL) || (pAddressLen == NULL))
    {
        LogError(("NULL parameter passed to getUrlAddress()."));
        httpStatus = HTTPInvalidParameter;
    }

    if (httpStatus == HTTPSuccess)
    {
        parserStatus = http_parser_parse_url(pUrl, urlLen, 0, &urlParser);

        if (parserStatus != 0)
        {
            LogError(("Error parsing the input URL %.*s. Error code: %d.",
                      (int32_t)urlLen,
                      pUrl,
                      parserStatus));
            httpStatus = HTTPParserInternalError;
        }
    }

    if (httpStatus == HTTPSuccess)
    {
        *pAddressLen = (size_t)(urlParser.field_data[UF_HOST].len);

        if (*pAddressLen == 0)
        {
            httpStatus = HTTPNoResponse;
            *pAddress = NULL;
        }
        else
        {
            *pAddress = &pUrl[urlParser.field_data[UF_HOST].off];
        }

        *pPathLen = (size_t)(urlParser.field_data[UF_PATH].len);

        if (*pPathLen == 0)
        {
            httpStatus = HTTPNoResponse;
            *pPath = NULL;
        }
        else
        {
            *pPath = &pUrl[urlParser.field_data[UF_PATH].off];
        }
    }

    if (httpStatus != HTTPSuccess)
    {
        LogError(("Error parsing the address from URL %s. Error code %d",
                  pUrl,
                  httpStatus));
    }

    *port = urlParser.port;

    return httpStatus;
}

int is_https(const char *pUrl)
{
    uint8_t temp_buf[8];
    uint32_t i;

    memset(temp_buf, 0, sizeof(temp_buf));
    for (i = 0; i < 5; i++)
        temp_buf[i] = tolower(pUrl[i]);

    if (!memcmp(temp_buf, "https", 5))
        return 1;
    else if (!memcmp(temp_buf, "http", 4))
        return 0;
    else
        return -1;
}






/**
@brief	CONVERT CHAR INTO HEX
@return	HEX

This function converts HEX(0-F) to a character
*/
static uint8_t C2D(
		uint8_t c	/**< is a character('0'-'F') to convert to HEX */
	)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}


/**
@brief	CONVERT STRING INTO INTEGER
@return	a integer number
*/
uint16_t ATOI(
	uint8_t * str,	/**< is a pointer to convert */
	uint8_t base	/**< is a base value (must be in the range 2 - 16) */
	)
{
        unsigned int num = 0;
// debug_2013_11_25
//        while (*str !=0)
        while ((*str !=0) && (*str != 0x20)) // not include the space(0x020)
                num = num * base + C2D(*str++);
	return num;
}


void inet_addr_(uint8_t * addr, uint8_t *ip)
{
	uint8_t i;
	uint8_t taddr[30];
	uint8_t * nexttok;
	uint8_t num;

	strcpy((char *)taddr, (char *)addr);

	nexttok = taddr;
	for(i = 0; i < 4 ; i++)
	{
		nexttok = (uint8_t *)strtok((char *)nexttok, ".");
		if(nexttok[0] == '0' && nexttok[1] == 'x') num = ATOI(nexttok+2,0x10);
		else num = ATOI(nexttok,10);
		ip[i] = num;
		nexttok = NULL;
	}
}