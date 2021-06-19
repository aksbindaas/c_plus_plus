#ifndef __RAWPACKET_H__
#define __RAWPACKET_H__

#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#include <net/if.h>

#include <netinet/tcp.h>
#include <netinet/ip.h>

#include "checksum.h"
#include "rawPacketParse.h"
using namespace std;

//unsigned char dest_mac[ETHER_ADDR_LEN]  = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
unsigned char dest_mac[ETHER_ADDR_LEN] = {0x50,0x2b,0x73,0x4e,0x60,0xf0};
unsigned char src_mac[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const size_t ETHER_PAYLOAD_START = (2*ETHER_ADDR_LEN) + ETHER_TYPE_LEN;
const size_t ETHER_PAYLOAD_LEN = ETHER_MAX_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN;

/* 
	96 bit (12 bytes) pseudo header needed for tcp header checksum calculation 
*/
struct pseudo_header
{
	in_addr source_address;
	in_addr dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

class Sniffer {
private:
    static void PacketWrite (int bpf, uint8_t * buffer, int datalen) {
         unsigned char buff[65535];
         ether_header header;
         memcpy(&header.ether_dhost, dest_mac, ETHER_HDR_LEN);
         memcpy(&header.ether_shost, src_mac, ETHER_HDR_LEN);

         header.ether_type = htons(ETHERTYPE_IP); //means next header will be IP header
         memcpy(buff, &header, sizeof(ether_header));
         memcpy(buff+sizeof(ether_header), buffer+4, datalen);

        // append header
        struct ip *iph = (struct ip*)(buff +sizeof(ether_header));

        const int iphdrlen = iph->ip_hl * 4; 
        // change destination ip
        inet_aton("23.212.99.43", &iph->ip_dst);

        // recalculate ip header checksum
        iph->ip_sum = 0;
        iph->ip_sum = CheckSum::ip_checksum(iph, iph->ip_hl * 4);

        struct tcphdr *tcp = (tcphdr*)(buff + iphdrlen + sizeof(ether_header));
        
        std::cout<<"Flags == "<<!(tcp->th_flags & (TH_PUSH))<<std::endl;
        if(!(tcp->th_flags & TH_PUSH)) {
            return;
        }

        Parse::data_process(buffer, datalen);
        
        std::cout<<"Flags PASS "<<(tcp->th_flags & TH_PUSH)<<" "<< (tcp->th_flags & TH_ACK)<<std::endl;

        tcp->th_dport = htons(80);
        // recalculate checksum
        tcp->th_sum = 0;
        

        pseudo_header psh;
        memset(&psh, 0, sizeof(pseudo_header));
        inet_aton("192.168.0.188", &psh.source_address);
        inet_aton("23.212.99.43", &psh.dest_address);
        
        psh.protocol = IPPROTO_TCP;
        int data_len = ntohs(iph->ip_len) - iph->ip_hl * 4 - tcp->th_off*4;
        psh.tcp_length = htons(tcp->th_off*4 +data_len);
        int psize = sizeof(pseudo_header) + sizeof(tcphdr) + data_len;
	    char pseudogram[65535];
        memset(&pseudogram, 0, 65535);
        memcpy(pseudogram , (char*) &psh , sizeof (pseudo_header));
	    memcpy(pseudogram + sizeof(pseudo_header) , tcp  , tcp->th_off*4+data_len);
	    tcp->th_sum = CheckSum::tcp_checksum( (unsigned short*) pseudogram , psize);
        
        // if(write(bpf, buff, datalen+ sizeof(ether_header) - 4) <= 0) {
        //       printf("*******************************************Enable to write");
        // }
    }

    static int OpenBPF(short flag) {
        char buf[11];
        for (int i = 0; i < 99; ++i) {
            sprintf(buf, "/dev/bpf%d", i);
            int bpf = open(buf, flag);
            if (bpf != -1) {
                return bpf;
            }
        }
        return -1;
    }

    static  void assoc_dev(int bpf, char* interface) {
        struct ifreq bound_if;
        strcpy(bound_if.ifr_name, interface);
        if(ioctl( bpf, BIOCSETIF, &bound_if ) > 0) {
            printf("Cannot bind bpf device to physical device %s, exiting\n", interface);
            exit(1);
        }
        printf("Bound bpf device to physical device %s\n", interface);
    }

    static int get_buf_len(int bpf) {
        int buf_len = 1;
        // activate immediate mode (therefore, buf_len is initially set to "1")
        if( ioctl( bpf, BIOCIMMEDIATE, &buf_len ) == -1 ) {
            printf("Cannot set IMMEDIATE mode of bpf device\n");
            exit(1);
        }
        // request buffer length
        if( ioctl( bpf, BIOCGBLEN, &buf_len ) == -1 ) {
            printf("Cannot get bufferlength of bpf device\n");
            exit(1);
        }
        printf("Buffer length of bpf device: %d\n", buf_len);
        return buf_len;
    }

    static void CapturePackets(int bpf, int buf_len, int Wbpf) {
        vector<uint8_t> bpf_buf(buf_len);
        size_t i;
        struct bpf_hdr* bh = nullptr;
        struct ether_header* eh = nullptr;

        // capture packets
        while (1) {
            int n = read(bpf, bpf_buf.data(), bpf_buf.size());
            if (n == -1) {
                perror("failed to read");
                exit(-1);
            }
            i = 0;
            while (i < n) {
                bh = reinterpret_cast<struct bpf_hdr*>(bpf_buf.data() + i);
                eh = reinterpret_cast<struct ether_header*>(bpf_buf.data() + i + bh->bh_hdrlen);
                //printf(" [type=%x] caplen=%u, datalen=%u, hdrlen=%u\n", ntohs(eh->ether_type), bh->bh_caplen, bh->bh_datalen, bh->bh_hdrlen); fflush(stdout);
                PacketWrite(Wbpf,bpf_buf.data()+ i + bh->bh_hdrlen, bh->bh_caplen);
                i += BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen);
            }
        }
    }

public:
    
    static void *sniffer(void *arg) {
        int Wbpf = OpenBPF(O_WRONLY);
        assoc_dev(Wbpf, "en0");
        
        int Rbpf = OpenBPF(O_RDONLY);
        assoc_dev(Rbpf, "lo0");
        CapturePackets(Rbpf, get_buf_len(Rbpf), Wbpf);
    }

};

#endif // __RAWPACKET_H__