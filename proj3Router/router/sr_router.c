/**********************************************************************
 * file:  sr_router.c
 * date:  Mon Feb 18 12:50:42 PST 2002
 * Contact: casado@stanford.edu
 *
 * Description:
 *
 * Project implemented by Laura Macaddino and Isabelle Rice
 * 
 * This file contains all the functions that interact directly
 * with the routing table, as well as the main entry method
 * for routing.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_utils.h"

/*---------------------------------------------------------------------
 * Method: sr_init(void)
 * Scope:  Global
 *
 * Initialize the routing subsystem
 *
 *---------------------------------------------------------------------*/

void sr_init(struct sr_instance* sr)
{
    /* REQUIRES */
    assert(sr);

    /* Initialize cache and cache cleanup thread */
    sr_arpcache_init(&(sr->cache));

    pthread_attr_init(&(sr->attr));
    pthread_attr_setdetachstate(&(sr->attr), PTHREAD_CREATE_JOINABLE);
    pthread_attr_setscope(&(sr->attr), PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setscope(&(sr->attr), PTHREAD_SCOPE_SYSTEM);
    pthread_t thread;

    pthread_create(&thread, &(sr->attr), sr_arpcache_timeout, sr);
    
    /* Add initialization code here! */

} /* -- sr_init -- */

/*---------------------------------------------------------------------
 * Method: sr_handlepacket(uint8_t* p,char* interface)
 * Scope:  Global
 *
 * This method is called each time the router receives a packet on the
 * interface.  The packet buffer, the packet length and the receiving
 * interface are passed in as parameters. The packet is complete with
 * ethernet headers.
 *
 * Note: Both the packet buffer and the character's memory are handled
 * by sr_vns_comm.c that means do NOT delete either.  Make a copy of the
 * packet instead if you intend to keep it around beyond the scope of
 * the method call.
 *
 *---------------------------------------------------------------------*/

void sr_handlepacket(struct sr_instance* sr,
	uint8_t * packet/* lent */,
	unsigned int len,
	char* interface/* lent */)
{
  /* REQUIRES */
  assert(sr);
  assert(packet);
  assert(interface);

  assert(len > sizeof(sr_ethernet_hdr_t));
  sr_ethernet_hdr_t * e_hdr;
  e_hdr = (sr_ethernet_hdr_t *) packet;
  if (ntohs(e_hdr->ether_type) == ethertype_arp)
  {
    /* Received ARP packet */
    assert(len == sizeof(sr_ethernet_hdr_t) + sizeof(sr_arp_hdr_t));
    sr_arp_hdr_t * arp_hdr;
    arp_hdr = (sr_arp_hdr_t *) (packet+sizeof(sr_ethernet_hdr_t));

    if (ntohs(arp_hdr->ar_op) == arp_op_request)
    {
      /* Received ARP request */
      struct sr_rt * rtEntry = sr_rtentry_for_ip(sr, arp_hdr->ar_sip);
      if (rtEntry == NULL)
      {
        /* Nonexistent route to Dest IP; drop packet */
        return;
      }
      /* check arpcache for response and send if there */
      /* if there, send */
      /* if not,*/
      send_arpresp(sr, packet, rtEntry);
      /* add this reply to the cache */
    }
    else if (ntohs(arp_hdr->ar_op) == arp_op_reply)
    {
      /* Received ARP reply */
      handle_arpresp(sr, packet);
    }
  }
  else if (ntohs(e_hdr->ether_type) == ethertype_ip)
  {
    printf("Receive IP packet, length(%d)\n", len);
    /* Sanity check the packet */
    assert(len >= sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t));
    sr_ip_hdr_t * ip_hdr;
    ip_hdr = (sr_ip_hdr_t *) (packet+sizeof(sr_ethernet_hdr_t));
    print_hdr_eth((uint8_t *)e_hdr);
    print_hdr_ip((uint8_t *)ip_hdr);
    int chksum = ip_hdr->ip_sum;
    ip_hdr->ip_sum = 0;
    assert(chksum == cksum(ip_hdr, sizeof(sr_ip_hdr_t)));

    /* check if IP is sent to one of our interfaces */
    struct sr_if * iface;
    iface = sr->if_list;
    while (iface)
    {
      if (iface->ip == ip_hdr->ip_dst)
        break;
      iface = iface->next;
    }
    if (iface == NULL)
    {
      /* case in which IP is not sent to one of our interfaces */ 
      /* decrement the TTL by 1, and recompute 
       * the packet checksum over the modified header */
      ip_hdr->ip_ttl -= 1;

      /* if TTL is zero, send Time exceeded ICMP message */
      if (ip_hdr->ip_ttl == 0)
      {
        sr_send_t3_icmp(11, 0, packet, sr, interface);
        return;
      } 

      /* check for closest ip address in routing table */
      struct sr_rt * rtEntry = sr_rtentry_for_ip(sr, ip_hdr->ip_dst);
      if (rtEntry == NULL)
      {
        /* Send Destination net unreachable ICMP message */
        sr_send_t3_icmp(3, 0, packet, sr, interface);
        return;
      }
      struct sr_if * srcIf = sr_get_interface(sr, rtEntry->interface);
      memcpy(e_hdr->ether_shost, srcIf->addr, ETHER_ADDR_LEN);
      ip_hdr->ip_sum = 0;
      ip_hdr->ip_sum = cksum(ip_hdr, sizeof(sr_ip_hdr_t));
      /* check the ARP cache for the next-hop MAC address corresponding to
       * the next-hop IP */
      struct sr_arpentry * macIpMap = sr_arpcache_lookup(&sr->cache, (uint32_t) rtEntry->gw.s_addr);
      if ((macIpMap == NULL) || (!macIpMap->valid))
      {
	/* Not in ARP cache; send an ARP request for the next-hop IP */
	/* add ARP request to request queue */
	sr_arpcache_queuereq(&sr->cache, (uint32_t) rtEntry->gw.s_addr, packet, len, rtEntry->interface);
      }
      else
      {
	/* in ARP cache; forward IP packet */
	struct sr_if * sourceIface = sr_get_interface(sr, rtEntry->interface);
	memcpy(e_hdr->ether_dhost, macIpMap->mac, ETHER_ADDR_LEN);
        printf("Modified IP packet, length(%d)\n", len);
	sr_send_packet(sr, packet, len, sourceIface->name);
        free(macIpMap);
      }

    }
    else
    {
      /* Case in which IP is sent to one of our interfaces */
      if (ip_hdr->ip_p == ip_protocol_icmp)
      {
        /* Received ICMP packet; check validity */
        struct sr_if * recvIface = sr_get_interface(sr, interface);
        sr_icmp_hdr_t * icmp_hdr = (sr_icmp_hdr_t *) (packet+sizeof(sr_ethernet_hdr_t)+sizeof(sr_ip_hdr_t));
	assert(len >= sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t) + sizeof(sr_icmp_hdr_t));
        int cksum_icmp = icmp_hdr->icmp_sum;
        icmp_hdr->icmp_sum = 0;
        assert(cksum_icmp = cksum((void *) icmp_hdr, len - sizeof(sr_ethernet_hdr_t) - sizeof(sr_ip_hdr_t)));
       
        if (icmp_hdr->icmp_type == 8)
        {
          /* Received echo request */ 
          icmp_hdr->icmp_type = 0;
          icmp_hdr->icmp_code = 0;
          icmp_hdr->icmp_sum = cksum((void *) icmp_hdr, len - sizeof(sr_ethernet_hdr_t) - sizeof(sr_ip_hdr_t));

          ip_hdr->ip_dst = ip_hdr->ip_src;
          ip_hdr->ip_src = recvIface->ip;
          ip_hdr->ip_sum = 0;
          ip_hdr->ip_sum = cksum((void *) ip_hdr, sizeof(sr_ip_hdr_t));
          memcpy(e_hdr->ether_dhost, e_hdr->ether_shost, ETHER_ADDR_LEN);
          memcpy(e_hdr->ether_shost, recvIface->addr, ETHER_ADDR_LEN);
          /* Send echo reply */ 
          sr_send_packet(sr, packet, len, recvIface->name);
        }
      }
      else
      {
        /* Received TCP/UDP packet for interface; drop packet */
        sr_send_t3_icmp(3, 3, packet, sr, interface);
        return;
      }
    }
  }
}/* end sr_ForwardPacket */


/* BEGIN USER-DEFINED FUNCTIONS */


/* sr_rtentry_for_ip(): Given an IP address, checks if router
 * can forward packets to IP address through any of its interfaces.
 * -This is done by applying the subnet mask to both the IP and 
 * rtable destIP entry, 8 bits at a time. 
 * -Next, this 8-bit portion of both IPs are compared. If they are 
 * equal, the following 8 bits of the masked IPs are compared.
 * -Finally, of all the entries in the rtable, the entry with the
 * a completely-matching destIP is returned. 
 * -If there are no completely matching entries, return NULL
 */
struct sr_rt *sr_rtentry_for_ip(struct sr_instance* sr, int ip)
{
  struct sr_rt * mostMatchingRtEntry = NULL;
  int mask;
  int destIP;
  int ourIP;
  int octetNum;

  struct sr_rt *rtEntry = sr->routing_table;
  while (rtEntry)
  {
    mask = rtEntry->mask.s_addr;
    for (octetNum=0; octetNum<32; octetNum+=8)
    {
      mask = rtEntry->mask.s_addr<<octetNum>>24;
      destIP = rtEntry->dest.s_addr<<octetNum>>24;
      ourIP = ip<<octetNum>>24;
      if ((mask & destIP) == (mask & ourIP))
      {
        if (octetNum == 24)
        {
          /* ip completely matches masked destIP */
          mostMatchingRtEntry = rtEntry;
          break;
        }
      }
      else
      {
        /* ip does not match masked destIP */
        break;
      }
    }
    rtEntry = rtEntry->next;
  }
  return mostMatchingRtEntry;
}


/* sr_send_t3_icmp(): Sends a type 3 ICMP response message to the source IP of given
 * packet through given interface. The type of ICMP message depends on the provided
 * code and type
 */
void sr_send_t3_icmp(int type, int code, uint8_t * packet, struct sr_instance * sr, char * interface)
{
  int sendPacketLen = sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t) + sizeof(sr_icmp_t3_hdr_t);
  uint8_t * sendPacket = (uint8_t *) malloc(sendPacketLen);
  memset(sendPacket, 0, sendPacketLen);
  sr_ethernet_hdr_t * ethHdr = (sr_ethernet_hdr_t *) sendPacket;
  sr_ip_hdr_t * ipHdr = (sr_ip_hdr_t *) (sendPacket+sizeof(sr_ethernet_hdr_t));
  sr_icmp_t3_hdr_t * icmpPacket = (sr_icmp_t3_hdr_t *) (sendPacket+sizeof(sr_ethernet_hdr_t)+sizeof(sr_ip_hdr_t));
  struct sr_if * recvIface = sr_get_interface(sr, interface);
  sr_ethernet_hdr_t * recvEthHdr = (sr_ethernet_hdr_t *) packet;
  sr_ip_hdr_t * recvIpHdr = (sr_ip_hdr_t *) (packet+sizeof(sr_ethernet_hdr_t));

  /* Fill ICMP Packet */
  icmpPacket->icmp_type = type;
  icmpPacket->icmp_code = code;
  memcpy(icmpPacket->data, recvIpHdr, ICMP_DATA_SIZE);
  icmpPacket->icmp_sum = 0;
  icmpPacket->icmp_sum =  cksum((void *) icmpPacket, sizeof(sr_icmp_t3_hdr_t));

  /* Encapsulate ICMP Packet in IP packet */
  ipHdr->ip_hl = recvIpHdr->ip_hl;
  ipHdr->ip_v = recvIpHdr->ip_v;
  ipHdr->ip_tos = recvIpHdr->ip_tos;
  ipHdr->ip_id = recvIpHdr->ip_id;
  ipHdr->ip_len = htons(sizeof(sr_ip_hdr_t) + sizeof(sr_icmp_t3_hdr_t));
  ipHdr->ip_off = recvIpHdr->ip_off;
  ipHdr->ip_ttl = 64;
  ipHdr->ip_p = ip_protocol_icmp;
  ipHdr->ip_src = recvIface->ip;
  ipHdr->ip_dst = recvIpHdr->ip_src;
  ipHdr->ip_sum = 0;
  ipHdr->ip_sum = cksum((void *) ipHdr, sizeof(sr_ip_hdr_t));

  /* Encapsulate IP packet in Ethernet packet */
  memcpy(ethHdr->ether_dhost, recvEthHdr->ether_shost, ETHER_ADDR_LEN);
  memcpy(ethHdr->ether_shost, recvIface->addr, ETHER_ADDR_LEN);
  ethHdr->ether_type = htons(ethertype_ip);

  sr_send_packet(sr, sendPacket, sendPacketLen, recvIface->name);
}
