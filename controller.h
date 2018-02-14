/**
 * Justin Herrera
 * Winter 2018
 * CPE 465 - Program 2
 *
 * controller.h
 **/

#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <inttypes.h>
#include <string.h>
#include <stdint.h>

#include "openflow.h"

/* Header Or Address Lengths */
#define ETHERNET_HDR_LEN 14
#define MAC_ADDR_LEN 6
#define OFP_HDR_LEN 8
#define PORT_DATA_LEN 48
#define FEATURE_LEN 24

/* OpenFlow Types */
#define OF_HELLO 0
#define OF_FEATURE_REPLY 6
#define OF_ECHO_REQUEST 2
#define OF_PACKET_IN 10
#define OF_PORT_STATUS 12

/* Ethernet Header */
struct ethernet_header {
    uint8_t dest[MAC_ADDR_LEN];
    uint8_t src[MAC_ADDR_LEN];
    uint16_t type;
}__attribute__((packed));

/* IP Header */
struct ip_header {
    uint8_t ver_hdr_len;
    uint8_t diffserv_ecn;
    uint16_t tot_len;
    uint16_t identification;
    uint16_t fragment;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    struct in_addr source_addr;
    struct in_addr dest_addr;
}__attribute__((packed));

/* TCP Header */
struct tcp_header {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t seq;
    uint32_t ack_num;
    uint8_t hdr_len;
    uint8_t flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urget;
}__attribute__((packed));

#if defined (__linux__)
#  include <endian.h>
#elif define(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
# define be16toh(x) betoh16(x)
# define be32toh(x) betoh32(x)
# define be64toh(x) betoh64(x)
#endif
