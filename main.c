#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <pthread.h>

#include <arpa/inet.h>
#define SA struct sockaddr

int snet, scan;

void printcanframe(char *title, struct can_frame canframe, char *netframe)
{
        printf("%s = %08x [%d]",title, canframe.can_id, canframe.can_dlc);
        for (int i=0; i<canframe.can_dlc ; i++)
        {
                printf(" %02x", canframe.data[i]);
        }
        printf(" <<-->> %s", netframe);
}

void ydnr2canframe(char *ydnr, struct can_frame *canframe)
{
        char buffer[9];
        unsigned long ibuff;

        // printf(" Get CAN ID\n");
        buffer[8]='\0';
        strncpy(buffer, ydnr + 15, 8);
        sscanf(buffer, "%8x", &ibuff);
        canframe->can_id = ibuff << 4;
        canframe->can_id = ibuff & 0x1fffffff | 0x80000000;

        // printf("Get CAN DLC\n");
        canframe->can_dlc = ( strlen(ydnr) - 25 ) / 3 ;

        // printf("Get CAN DATAs\n");
        buffer[2]='\0';
        for (int i=0; i<canframe->can_dlc ; i++)
        {
                strncpy(buffer, ydnr + 24 + (3 * i), 2);
                sscanf(buffer, "%x", &ibuff);
                canframe->data[i] = ibuff;
                //printf("%d - ", ibuff);
        }
}

int openydnr()
{
        int sockfd, connfd;
        struct sockaddr_in servaddr, cli;

        // socket create and verification
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
                        printf("socket creation failed...\n");
                        exit(0);
        }
        printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("192.168.8.100");
        servaddr.sin_port = htons(1457);

        // connect the client socket to server socket
        if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
                        printf("connection with the server failed...\n");
                        exit(0);
        }

        return sockfd;
}

int opencan()
{
        int socketfd;
        struct sockaddr_can addr;
        struct ifreq ifr;

        //printf("CAN Sockets Receive Demo\r\n");

        if ((socketfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("Socket");
                return 1;
        }

        strcpy(ifr.ifr_name, "vcan0" );
        ioctl(socketfd, SIOCGIFINDEX, &ifr);

        memset(&addr, 0, sizeof(addr));
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("Bind");
                return 1;
        }
                return socketfd;
}

void *ydnr2can()
{
        struct can_frame frame;
        char buff[60], buffer[2];

        while ( 1 ) {
                buff[0] = '\0';
                buffer[0] = 0x00;
                buffer[1] = '\0';

                while (buffer[0] != 0x0a) {
                        read(snet, buffer, 1);
                        //printf("=> %s\n", buffer);
                        strcat(buff, buffer);
                        //printf("==> %s\n", buff);
                }

                //printf("YDNR line : %s", buff);

                ydnr2canframe(buff, &frame);

                if (write(scan, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
                        perror("Write");
                        exit(1);
                } else {
                        printcanframe("Net to CAN", frame, buff);
                }
        }
}


void *can2ydnr()
{
            char buff[40], buff2[2];
                struct can_frame frame;
                int nbytes;

        while ( 1 ) {
                nbytes = read(scan, &frame, sizeof(struct can_frame));

                if (nbytes < 0) {
                        perror("Read");
                        exit(1);
                }

                sprintf(buff, "%08.8lx ",frame.can_id & 0x1fffffff);

                for (int i = 0; i < frame.can_dlc; i++) {
                        sprintf(buff2, "%02x ",frame.data[i]);
                        strcat(buff, buff2);
                }

                strcat(buff, "\n");
                write(snet, buff, sizeof(buff));
                                printcanframe("CAN to Net", frame, buff);
        }
}

int main(int argc, char **argv)
{

        snet = openydnr();
        scan = opencan();

                pthread_t thread_id_send, thread_id_rcv;

                pthread_create(&thread_id_send, NULL, ydnr2can, NULL);
                pthread_create(&thread_id_rcv, NULL, can2ydnr, NULL);

                pthread_join(thread_id_send, NULL);
                pthread_join(thread_id_rcv, NULL);

        return 0;
}
