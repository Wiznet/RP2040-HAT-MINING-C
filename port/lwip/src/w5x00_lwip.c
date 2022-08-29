/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "w5x00_lwip.h"

#include "socket.h"
#include "wizchip_conf.h"

#include "netif/etharp.h"

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
static ip_addr_t g_ip;
static ip_addr_t g_mask;
static ip_addr_t g_gateway;

static uint8_t tx_frame[1542];
static const uint32_t ethernet_polynomial_le = 0xedb88320U;
static uint8_t mac[6];

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

void wizchip_lwip_init(uint8_t socket_num, struct netif *netif, wiz_NetInfo *net_info)
{
    int retval;

    // Set ethernet chip MAC address
    setSHAR(net_info->mac);
    ctlwizchip(CW_RESET_PHY, 0);
    memcpy(mac, net_info->mac, 6);

    // Initialize LWIP in NO_SYS mode
    lwip_init();

    if (net_info->dhcp == NETINFO_DHCP)
    {
        // Initialize network configuration
        IP4_ADDR(&g_ip, net_info->ip[0], net_info->ip[1], net_info->ip[2], net_info->ip[3]);
        IP4_ADDR(&g_mask, net_info->sn[0], net_info->sn[1], net_info->sn[2], net_info->sn[3]);
        IP4_ADDR(&g_gateway, net_info->gw[0], net_info->gw[1], net_info->gw[2], net_info->gw[3]);
    }
    else
    {
        g_ip.addr = 0;
        g_mask.addr = 0;
        g_gateway.addr = 0;
    }

    netif_add(netif, &g_ip, &g_mask, &g_gateway, NULL, netif_initialize, netif_input);
    netif->name[0] = 'e';
    netif->name[1] = '0';
    netif_set_default(netif);

    if (net_info->dhcp == NETINFO_DHCP)
        dhcp_start(netif);

    // Assign callbacks for link and status
    netif_set_link_callback(netif, netif_link_callback);
    netif_set_status_callback(netif, netif_status_callback);

    // MACRAW socket open
    retval = socket(socket_num, Sn_MR_MACRAW, 0, 0x00);
    if (retval < 0)
        printf(" MACRAW socket open failed\n");

    // Set the default interface and bring it up
    netif_set_link_up(netif);
    netif_set_up(netif);
}


int32_t send_lwip(uint8_t sn, uint8_t *buf, uint16_t len)
{
    uint8_t tmp = 0;
    uint16_t freesize = 0;

    tmp = getSn_SR(sn);

    freesize = getSn_TxMAX(sn);
    if (len > freesize)
        len = freesize; // check size not to exceed MAX size.

    wiz_send_data(sn, buf, len);
    setSn_CR(sn, Sn_CR_SEND);
    while (getSn_CR(sn))
        ;

    while (1)
    {
        uint8_t IRtemp = getSn_IR(sn);
        if (IRtemp & Sn_IR_SENDOK)
        {
            setSn_IR(sn, Sn_IR_SENDOK);
            // printf("Packet sent ok\n");
            break;
        }
        else if (IRtemp & Sn_IR_TIMEOUT)
        {
            setSn_IR(sn, Sn_IR_TIMEOUT);
            // printf("Socket is closed\n");
            //  There was a timeout
            return -1;
        }
    }

    return (int32_t)len;
}

int32_t recv_lwip(uint8_t sn, uint8_t *buf, uint16_t len)
{
    uint8_t head[2];
    uint16_t pack_len = 0;

    pack_len = getSn_RX_RSR(sn);

    if (pack_len > 0)
    {
        wiz_recv_data(sn, head, 2);
        setSn_CR(sn, Sn_CR_RECV);

        // byte size of data packet (2byte)
        pack_len = head[0];
        pack_len = (pack_len << 8) + head[1];
        pack_len -= 2;

        if (pack_len > len)
        {
            // Packet is bigger than buffer - drop the packet
            wiz_recv_ignore(sn, pack_len);
            setSn_CR(sn, Sn_CR_RECV);
            return 0;
        }

        wiz_recv_data(sn, buf, pack_len); // data copy
        setSn_CR(sn, Sn_CR_RECV);
    }

    return (int32_t)pack_len;
}

err_t netif_output(struct netif *netif, struct pbuf *p)
{
    uint32_t send_len = 0;
    uint32_t tot_len = 0;

    memset(tx_frame, 0x00, sizeof(tx_frame));

    for (struct pbuf *q = p; q != NULL; q = q->next)
    {
        memcpy(tx_frame + tot_len, q->payload, q->len);

        tot_len += q->len;

        if (q->len == q->tot_len)
        {
            break;
        }
    }

    if (tot_len < 60)
    {
        // pad
        tot_len = 60;
    }

    uint32_t crc = ethernet_frame_crc(tx_frame, tot_len);

    send_len = send_lwip(0, tx_frame, tot_len);

    return ERR_OK;
}

void netif_link_callback(struct netif *netif)
{
    printf("netif link status changed %s\n", netif_is_link_up(netif) ? "up" : "down");
}

void netif_status_callback(struct netif *netif)
{
    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

err_t netif_initialize(struct netif *netif)
{
    netif->linkoutput = netif_output;
    netif->output = etharp_output;
    netif->mtu = ETHERNET_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
    SMEMCPY(netif->hwaddr, mac, sizeof(netif->hwaddr));
    netif->hwaddr_len = sizeof(netif->hwaddr);
    return ERR_OK;
}

static uint32_t ethernet_frame_crc(const uint8_t *data, int length)
{
    uint32_t crc = 0xffffffff; /* Initial value. */

    while (--length >= 0)
    {
        uint8_t current_octet = *data++;

        for (int bit = 8; --bit >= 0; current_octet >>= 1)
        {
            if ((crc ^ current_octet) & 1)
            {
                crc >>= 1;
                crc ^= ethernet_polynomial_le;
            }
            else
                crc >>= 1;
        }
    }

    return ~crc;
}

void wizchip_lwip_process (uint8_t socket_num, struct netif *netif, uint8_t *pack)
{
    uint32_t pack_len = 0;
    struct pbuf *p = NULL;

    getsockopt(socket_num, SO_RECVBUF, &pack_len);
    if (pack_len > 0)
    {
        pack_len = recv_lwip(socket_num, (uint8_t *)pack, pack_len);
        if (pack_len)
        {
            p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
            pbuf_take(p, pack, pack_len);
        }
        else
        {
            printf(" No packet received\n");
        }

        if (pack_len && p != NULL)
        {
            LINK_STATS_INC(link.recv);
            if (netif->input(p, netif) != ERR_OK)
            {
                pbuf_free(p);
            }
        }
    }
    sys_check_timeouts();
}