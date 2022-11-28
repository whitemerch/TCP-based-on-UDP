#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <math.h>
#include <signal.h>


#define MAXLINE 1024


char *last(const char *str)
{
    size_t len = strlen(str);
    return (char *)str + len - 6;
}

int removezeros(const char *a){
    int i, c = -1;

    for (i = 3; i < strlen(a); i++) {
        if (a[i] != '0') {
            c = i;
            break;
        }
    }
    int num=atoi(((char *)a  - c));
    return num;
}

void envoi(int PORT1, int sockdo, struct sockaddr_in cliaddr, char filename[30]){
    FILE *fp;
    char buff[MAXLINE], packetfinal[7], ackattendu[7], seq[7];
    int acks[1000][10];
    int n, i, cwnd, size, afread;
    int len = sizeof(cliaddr);
    int retrans=0;
    int ssthresh=1;
    fd_set readfds;
    struct timeval timeout;
    
    // Openning the file 
    fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    i = 1;
    cwnd=1;
    int j;
    sprintf(packetfinal,"%06d",(int)floor(size/1018)+1);
    int iattendu=1;
    sprintf(ackattendu, "%06d", iattendu);
    afread=1018;
    int loop;
    int lost=0;
    //calculer le rtt
    while (1)
    {   
        FD_ZERO(&readfds);
        FD_SET(sockdo, &readfds);
        timeout.tv_sec = 3; // timeout = 3 seconds
        timeout.tv_usec = 0; //microseondes
        if (afread==1018){
            for (j=0;j<cwnd;j++){
                sprintf(seq, "%06d", i);
                strcpy(buff, seq);
                afread = fread(buff + 6, 1, MAXLINE - 6, fp);
                sendto(sockdo, buff, afread + 6,
                    0, (const struct sockaddr *)&cliaddr,
                    len);
                printf("Sending data number %s\n", seq);
                memset(buff, 0, sizeof(buff));
                i++;
            }
            // if (lost==1){
            //     iattendu++;
            //     sprintf(ackattendu, "%06d", iattendu);
            //     lost=0;
            // }
        }
        select(sockdo+1, &readfds, NULL, NULL, &timeout);
        if (FD_ISSET(sockdo, &readfds)) {
            n = recvfrom(sockdo, (char *)buff, MAXLINE,
                     0, (struct sockaddr *)&cliaddr,
                     &len);
            buff[n] = '\0';
            if (strcmp(last(buff),ackattendu)==0){
                printf("ACK number %s received\n", buff);
                if (strcmp(last(buff), packetfinal)==0)
                    break;
                //Pour eviter d'augmenter la fenetre pour rien
                if (afread==1018)
                    if (cwnd<ssthresh)
                        cwnd+=1;
                //retrans=0;
                iattendu++;
                sprintf(ackattendu, "%06d", iattendu);
            }
            // else{
            //     retrans++;
            // }
            // if (retrans==3){
            //     fseek(fp, iattendu*1018,SEEK_SET);
            //     retrans=0;
            //     i=iattendu;
            //     ssthresh=cwnd;
            //     cwnd=floor(cwnd/2);
            // }
        }
        //When we are waiting for the ack to be received. Either the packet is dropped or the ack is lost.
        else{
            printf("Timeout. Packet number %s retransmitted\n", ackattendu);
            //lost=1;
            i--;
            fseek(fp, (removezeros(ackattendu)-1)*1018,SEEK_SET);
        }

    }
    strcpy(buff, "FIN");
    sendto(sockdo, buff, strlen(buff),
           0, (const struct sockaddr *)&cliaddr,
           len);
    printf("Fichier envoye\n");
    fclose(fp);

}

// ./server PORT 
int main(int argc, char **argv)
{
    int sockfd, sockdo;
    char buffer[MAXLINE], port[5];
    struct sockaddr_in servaddr, servaddr1, cliaddr;
    int size;
    int PORT1;
    pid_t childpid;
    char msg[11];
    char filename[30];

    if (argc != 2)
    {
        printf("Problem with the number of arguments\n");
        exit(EXIT_FAILURE);
    }
    
    //Port publique
    int PORT = atoi(argv[1]);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);


    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int len, n;
    len = sizeof(cliaddr);
    PORT1=1400;
    
    while (1){
        //Receiving the SYN
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                    0, (struct sockaddr *)&cliaddr,
                    &len);
        buffer[n] = '\0';
        printf("Client : %s\n", buffer);
        
        memset(msg,0,strlen(msg));
        strcat(msg,"SYN-ACK");
        sprintf(port, "%d",PORT1);
        strcat(msg, port);

        memset(&servaddr1, 0, sizeof(servaddr1));
        // Filling server information for second socket
        servaddr1.sin_family = AF_INET; // IPv4
        servaddr1.sin_addr.s_addr = INADDR_ANY;
        servaddr1.sin_port = htons(PORT1);

        // Creating second socket once connected
        if ((sockdo = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        if (bind(sockdo, (const struct sockaddr *)&servaddr1,
                sizeof(servaddr1)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        //Sending the SYN-ACKPORT
        sendto(sockfd, (const char *)msg, strlen(msg),
           0, (const struct sockaddr *)&cliaddr,
           len);
        printf("SYN-ACK sent.\n");
    
        //Receiving the ACK
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                 0, (struct sockaddr *)&cliaddr,
                 &len);
        buffer[n] = '\0';
        printf("Client : %s\n", buffer);

        //Receiving the name of the file
        n = recvfrom(sockdo, (char *)buffer, MAXLINE,
                 0, (struct sockaddr *)&cliaddr,
                 &len);
        buffer[n] = '\0';
        printf("Client : %s\n", buffer);
    
        strcpy(filename,buffer);
        
        if ((childpid = fork()) == 0) {
           envoi(PORT1, sockdo, cliaddr, filename);
           kill(getpid(), SIGTERM);
        }
        PORT1++;

    }

    return 0;
}