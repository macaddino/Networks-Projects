#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include "sr_arpcache.h"
#include "sr_router.h"
#include "sr_if.h"
#include "sr_protocol.h"
#include "sr_utils.h"

/* 
  This function gets called every second. For each request sent out, we keep
  checking whether we should resend an request or destroy the arp request.
  See the comments in the header file for an idea of what it should look like.
*/
void sr_arpcache_sweepreqs(struct sr_instance *sr) { 
    struct sr_arpreq * request;
    struct sr_arpreq * nextRequest;
    nextRequest = sr->cache.requests;
    while (nextRequest)
    {
        request = nextRequest;
        nextRequest = request->next;
        handle_arpreq(sr, request);
    }
}


/* handle_arpreq(): Send ARP request packet unless it has already been
 * sent out five times. If it has been sent out five times, delete the
 * request and send ICMP Host Unreachable response
 */
void handle_arpreq(struct sr_instance *sr, struct sr_arpreq *req)
{
    time_t now;
    time(&now);

    if (difftime(now, req->sent) > 1.0)
    {
        if (req->times_sent >= 5)
        {
            /* Send ICMP Host Unreachable response*/
            struct sr_packet * packet = req->packets;
            while (packet)
            {
                sr_send_t3_icmp(3, 1, packet->buf, sr, packet->iface);
                packet = packet->next;
            }

            sr_arpreq_destroy(&sr->cache, req);
        }
        else
        {
            /* send ARP request */
            int arpPackLen = sizeof(sr_ethernet_hdr_t) + sizeof(sr_arp_hdr_t);
            uint8_t * packet = (uint8_t *) malloc(arpPackLen);
            memset(packet, 0, arpPackLen);
            /* ARP requests are sent to the broadcast MAC address */
            char * ether_desthost = (char *) malloc(sizeof(ETHER_ADDR_LEN));
            memset(ether_desthost, 0, sizeof(ETHER_ADDR_LEN));
            int desthostIdx;
            for (desthostIdx = 0; desthostIdx < ETHER_ADDR_LEN; desthostIdx++)
            {
                ether_desthost[desthostIdx] = 0xff;
            }

            /* Since ARP request is broadcasted, send request through every iface */
            struct sr_if * iface;
            iface = sr->if_list;
            while (iface)
            {
                sr_ethernet_hdr_t * e_hdr = (sr_ethernet_hdr_t *) packet;
                sr_arp_hdr_t * a_hdr = (sr_arp_hdr_t *) (packet+sizeof(sr_ethernet_hdr_t));
                e_hdr->ether_type = htons(ethertype_arp);
                memcpy(e_hdr->ether_dhost, ether_desthost, ETHER_ADDR_LEN);
                memcpy(e_hdr->ether_shost, iface->addr, ETHER_ADDR_LEN);
            
                a_hdr->ar_op = htons(arp_op_request);
                a_hdr->ar_hrd = htons(arp_hrd_ethernet);
                a_hdr->ar_pro = htons(ethertype_ip);
                a_hdr->ar_hln = ETHER_ADDR_LEN;
                a_hdr->ar_pln = 4;
                memcpy(a_hdr->ar_sha, iface->addr, ETHER_ADDR_LEN);
                a_hdr->ar_sip = iface->ip;
                memcpy(a_hdr->ar_tha, ether_desthost, ETHER_ADDR_LEN);
                a_hdr->ar_tip = req->ip;

                sr_send_packet(sr, packet, arpPackLen, iface->name);
                iface = iface->next;
            }
            req->sent = now;
            req->times_sent++;
        }    
    }
}


/* handle_arpresp(): Insert IP/MAC mapping contained in an ARP  response packet
 * into ARP cache. If the ARP request is found in the request queue, send all
 * corresponding packets waiting on MAC/IP pairing of the ARP request */
void handle_arpresp(struct sr_instance *sr, uint8_t *arpRespPacket)
{
    sr_ethernet_hdr_t * e_hdr = (sr_ethernet_hdr_t *) arpRespPacket;
    sr_arp_hdr_t * a_hdr = (sr_arp_hdr_t *) (arpRespPacket + sizeof(sr_ethernet_hdr_t));

    struct sr_arpreq * req;
    /* insert IP/MAC address pair into ARP cache */
    req = sr_arpcache_insert(&sr->cache, e_hdr->ether_shost, a_hdr->ar_sip);
 
    /* if corresponding ARP  request found in request queue, send all packets
     * pending on the corresponding ARP response */
    if (req)
    {
        struct sr_packet * sendPacket;
        sendPacket = req->packets;
        sr_ethernet_hdr_t * sendPacket_e_hdr;
        while (sendPacket)
        {
            /* update destination MAC address for each packet */
            sendPacket_e_hdr = (sr_ethernet_hdr_t *) sendPacket->buf;
            memcpy(sendPacket_e_hdr->ether_dhost, e_hdr->ether_shost, ETHER_ADDR_LEN);

            printf("Modified IP packet, length(%d)\n", sendPacket->len);
            sr_send_packet(sr, sendPacket->buf, sendPacket->len, sendPacket->iface);
            sendPacket = sendPacket->next;
        }
        sr_arpreq_destroy(&sr->cache, req);
    }
}


/* send_arpresp(): Given an ARP request packet and the routing table entry which
 * contains the correct gateway address the ARP response packet should leave from,
 * forms an ARP response packet to be sent to the senders of the ARP request packet
 */
void send_arpresp(struct sr_instance *sr, uint8_t * packet, struct sr_rt * rtEntry)
{
    int sendPacketLen = sizeof(sr_ethernet_hdr_t) + sizeof(sr_arp_hdr_t);
    uint8_t * sendPacket = (uint8_t *) malloc(sendPacketLen);
    memset(sendPacket, 0, sendPacketLen);
    struct sr_if * iface = sr_get_interface(sr, rtEntry->interface);
    sr_ethernet_hdr_t * send_e_hdr = (sr_ethernet_hdr_t *) sendPacket;
    sr_arp_hdr_t * send_arp_hdr = (sr_arp_hdr_t *) (sendPacket+sizeof(sr_ethernet_hdr_t));
    sr_ethernet_hdr_t * recv_e_hdr = (sr_ethernet_hdr_t *) packet;
	
    send_e_hdr->ether_type = htons(ethertype_arp);
    memcpy(send_e_hdr->ether_dhost, recv_e_hdr->ether_shost, ETHER_ADDR_LEN); 
    memcpy(send_e_hdr->ether_shost, iface->addr, ETHER_ADDR_LEN);

    send_arp_hdr->ar_hrd = htons(arp_hrd_ethernet);
    send_arp_hdr->ar_pro = htons(ethertype_ip);
    send_arp_hdr->ar_hln = ETHER_ADDR_LEN;
    send_arp_hdr->ar_pln = 4;
    send_arp_hdr->ar_op = htons(arp_op_reply);
    memcpy(send_arp_hdr->ar_sha, iface->addr, ETHER_ADDR_LEN);
    send_arp_hdr->ar_sip = iface->ip;
    memcpy(send_arp_hdr->ar_tha, recv_e_hdr->ether_shost, ETHER_ADDR_LEN);
    send_arp_hdr->ar_tip = (uint32_t) rtEntry->gw.s_addr;

    sr_send_packet(sr, sendPacket, sendPacketLen, iface->name);
}


/* Checks if an IP->MAC mapping is in the cache. IP is in network byte order.
   You must free the returned structure if it is not NULL. */
struct sr_arpentry *sr_arpcache_lookup(struct sr_arpcache *cache, uint32_t ip) {
    pthread_mutex_lock(&(cache->lock));
    
    struct sr_arpentry *entry = NULL, *copy = NULL;
    
    int i;
    for (i = 0; i < SR_ARPCACHE_SZ; i++) {
        if ((cache->entries[i].valid) && (cache->entries[i].ip == ip)) {
            entry = &(cache->entries[i]);
        }
    }
    
    /* Must return a copy b/c another thread could jump in and modify
       table after we return. */
    if (entry) {
        copy = (struct sr_arpentry *) malloc(sizeof(struct sr_arpentry));
        memcpy(copy, entry, sizeof(struct sr_arpentry));
    }
        
    pthread_mutex_unlock(&(cache->lock));
    
    return copy;
}

/* Adds an ARP request to the ARP request queue. If the request is already on
   the queue, adds the packet to the linked list of packets for this sr_arpreq
   that corresponds to this ARP request. You should free the passed *packet.
   
   A pointer to the ARP request is returned; it should not be freed. The caller
   can remove the ARP request from the queue by calling sr_arpreq_destroy. */
struct sr_arpreq *sr_arpcache_queuereq(struct sr_arpcache *cache,
                                       uint32_t ip,
                                       uint8_t *packet,           /* borrowed */
                                       unsigned int packet_len,
                                       char *iface)
{
    pthread_mutex_lock(&(cache->lock));
    
    struct sr_arpreq *req;
    for (req = cache->requests; req != NULL; req = req->next) {
        if (req->ip == ip) {
            break;
        }
    }
    
    /* If the IP wasn't found, add it */
    if (!req) {
        req = (struct sr_arpreq *) calloc(1, sizeof(struct sr_arpreq));
        req->ip = ip;
        req->next = cache->requests;
        cache->requests = req;
    }
    
    /* Add the packet to the list of packets for this request */
    if (packet && packet_len && iface) {
        struct sr_packet *new_pkt = (struct sr_packet *)malloc(sizeof(struct sr_packet));
        
        new_pkt->buf = (uint8_t *)malloc(packet_len);
        memcpy(new_pkt->buf, packet, packet_len);
        new_pkt->len = packet_len;
		new_pkt->iface = (char *)malloc(sr_IFACE_NAMELEN);
        strncpy(new_pkt->iface, iface, sr_IFACE_NAMELEN);
        new_pkt->next = req->packets;
        req->packets = new_pkt;
    }
    
    pthread_mutex_unlock(&(cache->lock));
    
    return req;
}

/* This method performs two functions:
   1) Looks up this IP in the request queue. If it is found, returns a pointer
      to the sr_arpreq with this IP. Otherwise, returns NULL.
   2) Inserts this IP to MAC mapping in the cache, and marks it valid. */
struct sr_arpreq *sr_arpcache_insert(struct sr_arpcache *cache,
                                     unsigned char *mac,
                                     uint32_t ip)
{
    pthread_mutex_lock(&(cache->lock));
    
    struct sr_arpreq *req, *prev = NULL, *next = NULL; 
    for (req = cache->requests; req != NULL; req = req->next) {
        if (req->ip == ip) {            
            if (prev) {
                next = req->next;
                prev->next = next;
            } 
            else {
                next = req->next;
                cache->requests = next;
            }
            
            break;
        }
        prev = req;
    }
    
    int i;
    for (i = 0; i < SR_ARPCACHE_SZ; i++) {
        if (!(cache->entries[i].valid))
            break;
    }
    
    if (i != SR_ARPCACHE_SZ) {
        memcpy(cache->entries[i].mac, mac, 6);
        cache->entries[i].ip = ip;
        cache->entries[i].added = time(NULL);
        cache->entries[i].valid = 1;
    }
    
    pthread_mutex_unlock(&(cache->lock));
    
    return req;
}

/* Frees all memory associated with this arp request entry. If this arp request
   entry is on the arp request queue, it is removed from the queue. */
void sr_arpreq_destroy(struct sr_arpcache *cache, struct sr_arpreq *entry) {
    pthread_mutex_lock(&(cache->lock));
    
    if (entry) {
        struct sr_arpreq *req, *prev = NULL, *next = NULL; 
        for (req = cache->requests; req != NULL; req = req->next) {
            if (req == entry) {                
                if (prev) {
                    next = req->next;
                    prev->next = next;
                } 
                else {
                    next = req->next;
                    cache->requests = next;
                }
                
                break;
            }
            prev = req;
        }
        
        struct sr_packet *pkt, *nxt;
        
        for (pkt = entry->packets; pkt; pkt = nxt) {
            nxt = pkt->next;
            if (pkt->buf)
                free(pkt->buf);
            if (pkt->iface)
                free(pkt->iface);
            free(pkt);
        }
        
        free(entry);
    }
    
    pthread_mutex_unlock(&(cache->lock));
}

/* Prints out the ARP table. */
void sr_arpcache_dump(struct sr_arpcache *cache) {
    fprintf(stderr, "\nMAC            IP         ADDED                      VALID\n");
    fprintf(stderr, "-----------------------------------------------------------\n");
    
    int i;
    for (i = 0; i < SR_ARPCACHE_SZ; i++) {
        struct sr_arpentry *cur = &(cache->entries[i]);
        unsigned char *mac = cur->mac;
        fprintf(stderr, "%.1x%.1x%.1x%.1x%.1x%.1x   %.8x   %.24s   %d\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ntohl(cur->ip), ctime(&(cur->added)), cur->valid);
    }
    
    fprintf(stderr, "\n");
}

/* Initialize table + table lock. Returns 0 on success. */
int sr_arpcache_init(struct sr_arpcache *cache) {  
    /* Seed RNG to kick out a random entry if all entries full. */
    srand(time(NULL));
    
    /* Invalidate all entries */
    memset(cache->entries, 0, sizeof(cache->entries));
    cache->requests = NULL;
    
    /* Acquire mutex lock */
    pthread_mutexattr_init(&(cache->attr));
    pthread_mutexattr_settype(&(cache->attr), PTHREAD_MUTEX_RECURSIVE);
    int success = pthread_mutex_init(&(cache->lock), &(cache->attr));
    
    return success;
}

/* Destroys table + table lock. Returns 0 on success. */
int sr_arpcache_destroy(struct sr_arpcache *cache) {
    return pthread_mutex_destroy(&(cache->lock)) && pthread_mutexattr_destroy(&(cache->attr));
}

/* Thread which sweeps through the cache and invalidates entries that were added
   more than SR_ARPCACHE_TO seconds ago. */
void *sr_arpcache_timeout(void *sr_ptr) {
    struct sr_instance *sr = sr_ptr;
    struct sr_arpcache *cache = &(sr->cache);
    
    while (1) {
        sleep(1.0);
        
        pthread_mutex_lock(&(cache->lock));
    
        time_t curtime = time(NULL);
        
        int i;    
        for (i = 0; i < SR_ARPCACHE_SZ; i++) {
            if ((cache->entries[i].valid) && (difftime(curtime,cache->entries[i].added) > SR_ARPCACHE_TO)) {
                cache->entries[i].valid = 0;
            }
        }
        
        sr_arpcache_sweepreqs(sr);

        pthread_mutex_unlock(&(cache->lock));
    }
    
    return NULL;
}

