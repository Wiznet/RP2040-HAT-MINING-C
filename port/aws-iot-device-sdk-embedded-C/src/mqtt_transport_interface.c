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
#include <string.h>

#include "pico/stdlib.h"

#include "ssl_transport_interface.h"
#include "mqtt_transport_interface.h"

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
NetworkContext_t g_network_context;
TransportInterface_t g_transport_interface;
mqtt_config_t g_mqtt_config;

/* SSL context pointer */
tlsContext_t *g_mqtt_tls_context_ptr;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
void mqtt_event_callback(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo)
{
    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if ((pPacketInfo->type & 0xF0U) == MQTT_PACKET_TYPE_PUBLISH)
    {
        /* Handle incoming publish. */
        if (pDeserializedInfo->pPublishInfo->payloadLength)
        {
            printf("%.*s,%d,%.*s\n", pDeserializedInfo->pPublishInfo->topicNameLength, pDeserializedInfo->pPublishInfo->pTopicName,
                   pDeserializedInfo->pPublishInfo->payloadLength,
                   pDeserializedInfo->pPublishInfo->payloadLength, pDeserializedInfo->pPublishInfo->pPayload);
        }
    }
    else
    {
        /* Handle other packets. */
        switch (pPacketInfo->type)
        {
            case MQTT_PACKET_TYPE_SUBACK:
            {
                printf("Received SUBACK: PacketID=%u\n", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PINGRESP:
            {
                /* Nothing to be done from application as library handles
                                        * PINGRESP. */
                printf("Received PINGRESP\n");

                break;
            }

            case MQTT_PACKET_TYPE_UNSUBACK:
            {
                printf("Received UNSUBACK: PacketID=%u\n", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBACK:
            {
                printf("Received PUBACK: PacketID=%u\n", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBREC:
            {
                printf("Received PUBREC: PacketID=%u\n", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBREL:
            {
                /* Nothing to be done from application as library handles
                                        * PUBREL. */
                printf("Received PUBREL: PacketID=%u\n", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBCOMP:
            {
                /* Nothing to be done from application as library handles
                                            * PUBCOMP. */
                printf("Received PUBCOMP: PacketID=%u\n", pDeserializedInfo->packetIdentifier);

                break;
            }

            /* Any other packet type is invalid. */
            default:
            {
                printf("Unknown packet type received:(%02x).\n", pPacketInfo->type);
            }
        }
    }
}

int mqtt_transport_yield(uint32_t mqtt_yield_timeout)
{
    int ret;

    ret = MQTT_ProcessLoop(&g_mqtt_config.mqtt_context, 60);
    if (ret != 0)
    {
        printf("MQTT process loop error : %d\n", ret);
    }

    return ret;
}

int8_t mqtt_transport_init(uint8_t cleanSession, uint8_t *ClientId, uint8_t *userName, uint8_t *password, uint32_t keepAlive)
{

    if (g_mqtt_config.mqtt_state != MQTT_IDLE)
        return -1;

    if (ClientId == NULL)
        return -1;
    memset((void *)&g_mqtt_config, 0x00, sizeof(mqtt_config_t));

    /* Set MQTT connection information */
    g_mqtt_config.mqtt_connect_info.cleanSession = cleanSession;
    // Client ID must be unique to broker. This field is required.

    g_mqtt_config.mqtt_connect_info.pClientIdentifier = ClientId;
    g_mqtt_config.mqtt_connect_info.clientIdentifierLength = strlen(g_mqtt_config.mqtt_connect_info.pClientIdentifier);

    // The following fields are optional.
    // Value for keep alive.
    g_mqtt_config.mqtt_connect_info.keepAliveSeconds = keepAlive;
    // Optional username and password.

    g_mqtt_config.mqtt_connect_info.pUserName = userName;
    if (userName == NULL)
        g_mqtt_config.mqtt_connect_info.userNameLength = 0;
    else
        g_mqtt_config.mqtt_connect_info.userNameLength = strlen(userName);

    g_mqtt_config.mqtt_connect_info.pPassword = password;
    if (password == NULL)
        g_mqtt_config.mqtt_connect_info.passwordLength = 0;
    else
        g_mqtt_config.mqtt_connect_info.passwordLength = strlen(password);

    return 0;
}

int mqtt_transport_subscribe(uint8_t qos, char *subscribe_topic)
{
    int packet_id = 0;
    packet_id = MQTT_GetPacketId(&g_mqtt_config.mqtt_context);
    uint32_t ret;

    if (g_mqtt_config.subscribe_count > MQTT_SUBSCRIPTION_MAX_NUM)
    {
        printf("MQTT subscription count error : %d\n", g_mqtt_config.subscribe_count);

        return -1;
    }

    g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count].qos = qos;
    g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count].pTopicFilter = subscribe_topic;
    g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count].topicFilterLength = strlen(subscribe_topic);

    /* Receive message */
    ret = MQTT_Subscribe(&g_mqtt_config.mqtt_context, &g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count], 1, packet_id);

    if (ret != 0)
    {
        printf("MQTT subscription is error : %d\n", ret);

        return -1;
    }
    else
    {
        printf("MQTT subscription is success\n");
    }
    g_mqtt_config.subscribe_count++;

    return 0;
}

int8_t mqtt_transport_connect(uint8_t sock, uint8_t ssl_flag, uint8_t *recv_buf, uint32_t recv_buf_len, uint8_t *domain, uint32_t port, tlsContext_t *tls_context)
{
    bool session_present;
    int ret = -1;
    int packet_id = 0;

    if (g_mqtt_config.mqtt_state != MQTT_IDLE)
        return -1;

    if (!is_ipaddr(domain, g_mqtt_config.mqtt_ip)) // IP
    {
        uint8_t dns_buf[512];
        ret = get_ipaddr_from_dns(domain, g_mqtt_config.mqtt_ip, dns_buf, DNS_TIMEOUT);
        if (ret != 1)
        {
            mqtt_transport_close(sock, &g_mqtt_config);

            return -1;
        }
    }

    if (ssl_flag == 0)
    {
        ret = socket(sock, Sn_MR_TCP, 0, 0x00); // port 0 : any port
        if (ret != sock)
        {
            mqtt_transport_close(sock, &g_mqtt_config);

            return -1;
        }
        ret = connect(sock, g_mqtt_config.mqtt_ip, port);
        if (ret != SOCK_OK)
        {
            mqtt_transport_close(sock, &g_mqtt_config);

            return -1;
        }
        g_transport_interface.send = mqtt_write;
        g_transport_interface.recv = mqtt_read;
    }
    else
    {
        /* Initialize SSL context */
        ret = ssl_transport_init(tls_context, (int *)(intptr_t)sock, domain);
        if (ret != 0)
        {
            mqtt_transport_close(sock, &g_mqtt_config);
            printf("SSL initialization error : %d\n", ret);

            return ret;
        }
        else
        {
            printf("SSL initialization is success\n");
        }

        ret = ssl_socket_connect_timeout(tls_context, g_mqtt_config.mqtt_ip, port, 0, MQTT_TIMEOUT);

        if (ret != 0)
        {
            mqtt_transport_close(sock, &g_mqtt_config);
            printf("SSL connection is error : %d\n", ret);

            return ret;
        }
        else
        {
            printf("SSL connection is success\n");
        }
        g_mqtt_tls_context_ptr = tls_context;

        g_transport_interface.send = mqtts_write;
        g_transport_interface.recv = mqtts_read;
    }
    g_network_context.socketDescriptor = sock;
    g_transport_interface.pNetworkContext = &g_network_context;
    g_mqtt_config.mqtt_fixed_buf.pBuffer = recv_buf;
    g_mqtt_config.mqtt_fixed_buf.size = recv_buf_len;

    /* Initialize MQTT context */
    ret = MQTT_Init(&g_mqtt_config.mqtt_context,
                    &g_transport_interface,
                    (MQTTGetCurrentTimeFunc_t)millis,
                    mqtt_event_callback,
                    &g_mqtt_config.mqtt_fixed_buf);

    if (ret != 0)
    {
        mqtt_transport_close(sock, &g_mqtt_config);
        printf("MQTT initialization is error : %d\n", ret);

        return -1;
    }
    else
    {
        printf("MQTT initialization is success\n");
    }

    /* Connect to the MQTT broker */
    ret = MQTT_Connect(&g_mqtt_config.mqtt_context, &g_mqtt_config.mqtt_connect_info, NULL, MQTT_TIMEOUT, &session_present);
    if (ret != 0)
    {
        mqtt_transport_close(sock, &g_mqtt_config);
        printf("MQTT connection is error : %d\n", ret);

        return -1;
    }
    else
    {
        printf("MQTT connection is success\n");
    }

    return 0;
}

int mqtt_transport_close(uint8_t sock, mqtt_config_t *mqtt_config)
{
    int ret;

    if (mqtt_config->ssl_flag == true)
    {
        mbedtls_ssl_close_notify(&g_mqtt_tls_context_ptr->ssl);
        mbedtls_ssl_free(&g_mqtt_tls_context_ptr->ssl);
        mbedtls_ssl_config_free(&g_mqtt_tls_context_ptr->conf);
        mbedtls_ctr_drbg_free(&g_mqtt_tls_context_ptr->ctr_drbg);
#if defined(MBEDTLS_ENTROPY_C)
        mbedtls_entropy_free(&g_tlsContext.entropy);
#endif
        mbedtls_x509_crt_free(&g_mqtt_tls_context_ptr->cacert);
        mbedtls_x509_crt_free(&g_mqtt_tls_context_ptr->clicert);
        mbedtls_pk_free(&g_mqtt_tls_context_ptr->pkey);
    }
    mqtt_config->subscribe_count = 0;
    ret = disconnect(sock);

    if (ret == SOCK_OK)
        return 0;
    else
        return -1;
}

int mqtt_transport_publish(uint8_t *pub_topic, uint8_t *pub_data, uint32_t pub_data_len, uint8_t qos)
{
    int packet_id;
    int ret;

    g_mqtt_config.mqtt_publish_info.qos = qos;

    g_mqtt_config.mqtt_publish_info.pTopicName = pub_topic;
    g_mqtt_config.mqtt_publish_info.topicNameLength = strlen(pub_topic);

    g_mqtt_config.mqtt_publish_info.pPayload = pub_data;
    g_mqtt_config.mqtt_publish_info.payloadLength = pub_data_len;

    packet_id = MQTT_GetPacketId(&g_mqtt_config.mqtt_context);
    /* Send message */
    ret = MQTT_Publish(&g_mqtt_config.mqtt_context, &g_mqtt_config.mqtt_publish_info, packet_id);

    if (ret != 0)
    {
        printf("MQTT pulishing is error : %d\n", ret);
        printf("PUBLISH FAILED\n");

        return -1;
    }
    else
    {
        printf("MQTT pulishing is success\n");
        printf("PUBLISH OK\n");

        return 0;
    }
}

int32_t mqtt_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
    {
        size = send(pNetworkContext->socketDescriptor, (uint8_t *)pBuffer, bytesToSend);
    }

    return size;
}

int32_t mqtt_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
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
    } while ((millis() - tickStart) <= MQTT_TIMEOUT);

    return size;
}

int32_t mqtts_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
        size = ssl_transport_write(g_mqtt_tls_context_ptr, (uint8_t *)pBuffer, bytesToSend);

    return size;
}

int32_t mqtts_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
        size = ssl_transport_read(g_mqtt_tls_context_ptr, pBuffer, bytesToRecv);

    return size;
}
