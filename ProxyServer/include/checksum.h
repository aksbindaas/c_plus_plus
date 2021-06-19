#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

class CheckSum {
public:
static u_short tcp_checksum(unsigned short* buff, int _16bitword) {
        unsigned long sum;
        for(sum=0;_16bitword>0;_16bitword--)
            sum+=(*(buff)++);
        sum = ((sum >> 16) + (sum & 0xFFFF));
        sum += (sum>>16);
        return (u_short)(~sum);
    }

    static uint16_t ip_checksum(void* vdata,size_t length) {
        char* data=(char*)vdata;
        uint32_t acc=0xffff;

        // Handle complete 16-bit blocks.
        for (size_t i=0;i+1<length;i+=2) {
            uint16_t word;
            memcpy(&word,data+i,2);
            acc+=ntohs(word);
            if (acc>0xffff) {
                acc-=0xffff;
            }
        }

        // Handle any partial block at the end of the data.
        if (length&1) {
            uint16_t word=0;
            memcpy(&word,data+length-1,1);
            acc+=ntohs(word);
            if (acc>0xffff) {
                acc-=0xffff;
            }
        }
        // Return the checksum in network byte order.
        return htons(~acc);
    }

};
#endif // __CHECKSUM_H__