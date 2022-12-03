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

//ACK000001-->000001 substr(string,3,9) 9 len(ACK000001) 3: a partir de quel element on veut le string
char* substr(const char *src, int m, int n)
{
   
    int len = n - m;
 
    
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
 
    
    for (int i = m; i < n && (*(src + i) != '\0'); i++)
    {
        *dest = *(src + i);
        dest++;
    }
 
    
    *dest = '\0';
 
    return dest - len;
}

//000001-->1
int removezeros(const char *a){
    int i, c = -1;

    for (i = 0; i < strlen(a); i++) {
        if (a[i] != '0') {
            c = i;
            break;
        }
    }
    int num=atoi(substr(a,c,6));
    return num;
}

//Pour voir si le ackattendu est déjà dans la liste des acks reçus
int inlist(const char acks[1000][10], const char ackattendu[13]){
    int i;
    for(i = 0; i < 1000; i++)
    {
        if(strcmp(acks[i], ackattendu)==0)
        {
            return 1;
        }   
    }
    return 0;
}

void envoi(int PORT1, int sockdo, struct sockaddr_in cliaddr, char filename[30]){
    FILE *fp;
    char buff[MAXLINE], packetfinal[7], ackattendu[7], seq[7];
    char acks[1000][10];
    char copy[10];
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
    cwnd=3;
    int j;
    sprintf(packetfinal,"%06d",(int)floor(size/1018)+1); //00000N
    int iattendu=1;
    sprintf(ackattendu, "%06d", iattendu);//00000N
    afread=1018;
    while (1)
    {   
        FD_ZERO(&readfds);
        FD_SET(sockdo, &readfds);
        timeout.tv_sec = 0; // timeout = 0 seconds
        timeout.tv_usec = 500000; //microseondes >19000 =20000
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
        }
        for (j=0;j<cwnd;j++){
            select(sockdo+1, &readfds, NULL, NULL, &timeout);
            if (FD_ISSET(sockdo, &readfds)) {
                n = recvfrom(sockdo, (char *)buff, MAXLINE,
                        0, (struct sockaddr *)&cliaddr,
                        &len);
                buff[n] = '\0';
                printf("%s\n",buff);
                if (inlist(acks, substr(buff,3,9))==1){
                    retrans++;
                }
                else{
                    strcpy(acks[(removezeros(substr(buff,3,9))-1)],substr(buff,3,9));
                }
                if (strcmp(substr(buff,3,9),ackattendu)==0){
                    printf("ACK number %s received\n", buff);
                    //000001 est à l'indice 0 du tableau. Le dernier 000106 est à l'indice 105
                    if (strcmp(substr(buff,3,9), packetfinal)==0)
                        break;
                    while (1){
                        iattendu++;
                        sprintf(ackattendu, "%06d", iattendu);
                        if (inlist(acks,ackattendu)==0)
                            break;
                    }
                }
                else{
                    //Normally when we don't get the ack of a certain packet we retransmit. The problem is sometimes the client considers that the paquet was received 
                    //so he sends ack of the next paquet. Here we should consider that the packet is received so we should transmit the one after after the one not acknowledged
                    if (retrans==2){
                        printf("Wrong ACK received 3 times. The packet that wasn't acknowledged has been received. Transmission of packet number %d\n", removezeros(ackattendu)+2);
                        retrans=0;
                        strcpy(acks[removezeros(ackattendu-1)],ackattendu);
                        fseek(fp, (removezeros(ackattendu)+1)*1018,SEEK_SET);
                        iattendu+=2;
                        sprintf(ackattendu, "%06d", iattendu);
                        i=iattendu;

                    }
                }
            }
            //When we are waiting for the ack to be received. Either the packet is dropped or the ack is lost.
            else{
                if (afread!=1018)
                    goto finished;
                printf("Timeout. Packet number %s retransmitted\n", ackattendu);
                afread=1018;
                i=removezeros(ackattendu);
                fseek(fp, (removezeros(ackattendu)-1)*1018,SEEK_SET);
            }
        }

    }
    finished: strcpy(buff, "FIN");
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