#if 1
//#ifndef __RAWPACKETPARSE_H__
#define __RAWPACKETPARSE_H__

#include <net/ethernet.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>

#include <string.h>
#include <stdio.h>

#include <iostream>
#include <iomanip>
#include <arpa/inet.h>

#define log_txt stdout
int total,tcp,udp,icmp,igmp,other,iphdrlen;

void payload(const uint8_t* bytes,int n, std::ostream& stream) {
        char buff[17];
        size_t i = 0;
        stream << std::hex;
        // Process every byte in the data.
        for (i = 0; i < n; i++) {
            // Multiple of 16 means new line (with line offset).
            if ((i % 16) == 0) {
                // Just don't print ASCII for the zeroth line.
                if (i != 0) {
                    stream << "  " << buff << std::endl;
                }
                // Output the offset.
                stream << "  " << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(i);
            }

            // Now the hex code for the specific character.
            stream << " " << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(bytes[i]);

            // And store a printable ASCII character for later.
            if ((bytes[i] < 0x20) || (bytes[i] > 0x7e)) {
                buff[i % 16] = '.';
            }
            else {
                buff[i % 16] = bytes[i];
            }
            buff[(i % 16) + 1] = '\0';
        }
        stream << std::dec;
        // Pad out last line if not exactly 16 characters.
        while ((i % 16) != 0) {
            stream << "   ";
            i++;
        }
        // And print the final ASCII bit.
        stream << "  " << buff << std::endl;
    }

class Parse{
public:
    
    static void ethernet_header(const uint8_t* buffer,int buflen, int isLO) {
        if(isLO == false) {
            ether_header *eth = (ether_header *)(buffer);
            fprintf(log_txt,"\nEthernet Header\n");
            fprintf(log_txt,"\t|-Destination Address	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->ether_dhost[0],eth->ether_dhost[1],eth->ether_dhost[2],eth->ether_dhost[3],eth->ether_dhost[4],eth->ether_dhost[5]);
            fprintf(log_txt,"\t|-Source Address	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->ether_shost[0],eth->ether_shost[1],eth->ether_shost[2],eth->ether_shost[3],eth->ether_shost[4],eth->ether_shost[5]);
            fprintf(log_txt,"\t|-Protocol		: %d\n",eth->ether_type);
        }
    }

    static void ip_header(const uint8_t* buffer,int buflen, int isLO) {
        int offset = (isLO) ? 0 : sizeof(ether_header);
        struct ip *ip = (struct ip*)(buffer + offset);

        iphdrlen =ip->ip_hl * 4;   

        fprintf(log_txt , "\nIP Header\n");

        fprintf(log_txt , "\t|-Version              : %d\n",(unsigned int)ip->ip_v);
        fprintf(log_txt , "\t|-Internet Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)ip->ip_hl,((unsigned int)(ip->ip_hl))*4);
        fprintf(log_txt , "\t|-Type Of Service   : %d\n",(unsigned int)ip->ip_tos);
        fprintf(log_txt , "\t|-Total Length      : %d  Bytes\n",ntohs(ip->ip_len));
        fprintf(log_txt , "\t|-Identification    : %d\n",ntohs(ip->ip_id));
        fprintf(log_txt , "\t|-Time To Live	    : %d\n",(unsigned int)ip->ip_ttl);
        fprintf(log_txt , "\t|-Protocol 	    : %d\n",(unsigned int)ip->ip_p);
        fprintf(log_txt , "\t|-Header Checksum   : %d\n",ntohs(ip->ip_sum));
        fprintf(log_txt , "\t|-Source IP         : %s\n", inet_ntoa(ip->ip_src));
        fprintf(log_txt , "\t|-Destination IP    : %s\n",inet_ntoa(ip->ip_dst   ));
    }


    static void tcp_header(const uint8_t* buffer,int buflen, int isLO) {
        fprintf(log_txt,"\n*************************TCP Packet******************************");
        ethernet_header(buffer,buflen, isLO);
        ip_header(buffer,buflen, isLO);

        int offset = (isLO) ? 0 : sizeof(ether_header);
        struct tcphdr *tcp = (tcphdr*)(buffer + iphdrlen + offset);
        fprintf(log_txt , "\nTCP Header\n");
        fprintf(log_txt , "\t|-Source Port          : %u\n",ntohs(tcp->th_sport));
        fprintf(log_txt , "\t|-Destination Port     : %u\n",ntohs(tcp->th_dport));
        fprintf(log_txt , "\t|-Sequence Number      : %u\n",ntohl(tcp->th_seq));
        fprintf(log_txt , "\t|-Acknowledge Number   : %u\n",ntohl(tcp->th_ack));
        fprintf(log_txt , "\t|-Header Length        : %d DWORDS or %d BYTES\n" ,(unsigned int)tcp->th_off,(unsigned int)tcp->th_off*4);
        fprintf(log_txt , "\t|----------Flags-----------\n");
        fprintf(log_txt , "\t\t|-Urgent Flag          : %d\n",(unsigned int)tcp->th_flags & TH_URG);
        fprintf(log_txt , "\t\t|-Acknowledgement Flag : %d\n",(unsigned int)tcp->th_flags & TH_ACK);
        fprintf(log_txt , "\t\t|-Push Flag            : %d\n",(unsigned int)tcp->th_flags & TH_PUSH);
        fprintf(log_txt , "\t\t|-Reset Flag           : %d\n",(unsigned int)tcp->th_flags & TH_RST);
        fprintf(log_txt , "\t\t|-Synchronise Flag     : %d\n",(unsigned int)tcp->th_flags & TH_SYN);
        fprintf(log_txt , "\t\t|-Finish Flag          : %d\n",(unsigned int)tcp->th_flags & TH_FIN);
        fprintf(log_txt , "\t|-Window size          : %d\n",ntohs(tcp->th_win));
        fprintf(log_txt , "\t|-Checksum             : %d\n",ntohs(tcp->th_sum));
        fprintf(log_txt , "\t|-Urgent Pointer       : %d\n",tcp->th_urp);

        payload(buffer,buflen, std::cout);

        fprintf(log_txt,"*****************************************************************\n\n\n");
    }

    static void data_process(const uint8_t * buffer,int buflen) {
        
        ip *iphdr = (ip*)(buffer + sizeof (ether_header));
        int isLO = 0 ;
        if(buffer[0] == 0x02) {
            isLO = 1;
            buffer += 4;
        }
        int offset = (isLO) ? 0 : sizeof(ether_header);
        iphdr = (ip*)(buffer + offset);
        ++total;
        switch (iphdr->ip_p){
            case 6:
                ++tcp;
                tcp_header(buffer,buflen, isLO);
                break;
            case 17:
                ++udp;
                break;
            default:
                ++other;
        }
	    printf("TCP: %d  UDP: %d  Other: %d  Toatl: %d  \r",tcp,udp,other,total);
}

};
#endif // __RAWPACKETPARSE_H__