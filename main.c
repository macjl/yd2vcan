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

/*--- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)\
  (((i) &0x80 ll) ? '1' : '0'), \
  (((i) &0x40 ll) ? '1' : '0'), \
  (((i) &0x20 ll) ? '1' : '0'), \
  (((i) &0x10 ll) ? '1' : '0'), \
  (((i) &0x08 ll) ? '1' : '0'), \
  (((i) &0x04 ll) ? '1' : '0'), \
  (((i) &0x02 ll) ? '1' : '0'), \
  (((i) &0x01 ll) ? '1' : '0')
#define PRINTF_BINARY_PATTERN_INT16\
PRINTF_BINARY_PATTERN_INT8 PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i)\
PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32\
PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i)\
PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64\
PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i)\
PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
/*--- end macros --- */

int debug=0, snet, scan;

void printcanframe(char *title, struct can_frame canframe, char *netframe)
{
  char canstr[60], buff[20];
  sprintf(canstr, "%08X [%d] ", canframe.can_id &CAN_EFF_MASK, canframe.can_dlc);
  for (int i = 0; i < canframe.can_dlc; i++)
  {
    sprintf(buff, " %02X", canframe.data[i]);
    strcat(canstr, buff);
  }

  printf("%*s | %*s | %s", -11, title, -40, canstr, netframe);
}

int ydnr2canframe(char *ydnr, struct can_frame *canframe)
{
  char buffer[9];
  unsigned long ibuff;

 	// printf(" Get CAN ID\n");
  buffer[8] = '\0';
  strncpy(buffer, ydnr + 15, 8);
  sscanf(buffer, "%8lx", &ibuff);
  canframe->can_id = ibuff | CAN_EFF_FLAG;
 	//printf("bit: "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(ibuff));
 	//printf("bit: "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(canframe->can_id));

 	// printf("Get CAN DLC\n");
  canframe->can_dlc = (strlen(ydnr) - 25) / 3;

 	// printf("Get CAN DATAs\n");
  buffer[2] = '\0';
  for (int i = 0; i < canframe->can_dlc; i++)
  {
    strncpy(buffer, ydnr + 24 + (3 *i), 2);
    sscanf(buffer, "%lx", &ibuff);
    canframe->data[i] = ibuff;
   	//printf("%d - ", ibuff);
  }

  if (ydnr[13] == 'T')
  {
    return -1;
  } else {
    return 0;
  }
}

int openydnr(char *ip, int port)
{
  int socketfd;
  struct sockaddr_in addr;

  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Inet socket");
    return -1;
  }

  bzero(&addr, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(port);

  if (connect(socketfd, (SA*) &addr, sizeof(addr)) < 0)
  {
    perror("Inet connect");
    return -1;
  }

  return socketfd;
}

int opencan( char *canport)
{
  int socketfd;
  struct sockaddr_can addr;
  struct ifreq ifr;

  if ((socketfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
  {
    perror("CAN Socket");
    return -1;
  }

  bzero(&addr, sizeof(addr));

  addr.can_family = AF_CAN;
  strcpy(ifr.ifr_name, canport);
  ioctl(socketfd, SIOCGIFINDEX, &ifr);
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(socketfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
  {
    perror("CAN Bind");
    return -1;
  }

  return socketfd;
}

void *ydnr2can()
{
  struct can_frame frame;
  char buff[60], buffer[2];

  while (1)
  {
    buff[0] = '\0';
    buffer[0] = 0x00;
    buffer[1] = '\0';

    while (buffer[0] != 0x0a)
    {
      read(snet, buffer, 1);
     	//printf("=> %s\n", buffer);
      strcat(buff, buffer);
     	//printf("==> %s\n", buff);
    }

   	//printf("YDNR line : %s", buff);

    if (ydnr2canframe(buff, &frame) == 0) {

      if (write(scan, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
      {
        perror("Write");
        exit(1);
      }
      else if ( debug > 0 )
      {
        printcanframe("Net to CAN", frame, buff);
      }
    }
  }
}

void *can2ydnr()
{
  char buff[60], buff2[4];
  struct can_frame frame;
  int nbytes;

  while (1)
  {
    nbytes = read(scan, &frame, sizeof(struct can_frame));

    if (nbytes < 0)
    {
      perror("Read");
      exit(1);
    }

    sprintf(buff, "%08lX", (long unsigned int) frame.can_id &CAN_EFF_MASK);
   	//printf("bit: "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(frame.can_id));

    for (int i = 0; i < frame.can_dlc; i++)
    {
      sprintf(buff2, " %02X", frame.data[i]);
      strcat(buff, buff2);
    }

    strcat(buff, "\r\n");
    if (write(snet, buff, sizeof(buff)) != sizeof(buff))
    {
      perror("Write");
      exit(1);
    }
    else if ( debug > 0 )
    {
      printcanframe("CAN to Net", frame, buff);
    }
  }
}

int main(int argc, char **argv)
{
    int opt, port=0;
    char ip[255]="",canport[255]="";
      
    while((opt = getopt(argc, argv, ":i:p:c:dx")) != -1) 
    { 
        switch(opt) 
        { 
	    case 'd':
		    debug++;
		    printf("Debug %d\n", debug);
		    break;
            case 'i': 
		strcpy(ip, optarg);
                break; 
            case 'p': 
                sscanf(optarg, "%d", &port);
                break; 
            case 'c': 
		strcpy(canport, optarg);
                break; 
            case ':': 
                printf("option needs a value\n"); 
                break; 
            case '?': 
                printf("unknown option: %c\n", optopt);
                break; 
        } 
    } 
     
  if ( ( port == 0 ) | ( ip[0] == '\0' ) | ( canport[0] == '\0') )
  {
	 printf("Usage :  yd2vcan -i <ip of yachtd gateway> -p <port of yathd gateway raw/tcp> -c <name of can device>");
	 return 32;
  }

  printf("YDNR device: \"%s:%d\"\nCAN device: \"%s\"\n", ip, port, canport);

  if ( (snet = openydnr(ip, port)) < 0 )
    return -1;
  if ( (scan = opencan(canport)) < 0 )
    return -1;

  pthread_t thread_id_send, thread_id_rcv;

  pthread_create(&thread_id_send, NULL, ydnr2can, NULL);
  pthread_create(&thread_id_rcv, NULL, can2ydnr, NULL);

  pthread_join(thread_id_send, NULL);
  pthread_join(thread_id_rcv, NULL);

  close(snet);
  close(scan);

  return 0;
}
