// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
    

#define MAXLINE 1024 

char *last(const char *str)
{
    size_t len = strlen(str);
    return (char *)str + len - 4;
}

// ./client IP port nom fichier
//ip /sbin/ifconfig
int main(int argc, char **argv) { 
    int sockfd, sockdo; 
    char buffer[MAXLINE]; 
    char buff[MAXLINE];
    char *msg = "SYN"; 
    char *donnee;
    char cpy[7];
    char sport[10];
    char writ[MAXLINE-7];
    struct sockaddr_in     servaddr, servaddr1; 
    FILE *fp;
    int i;
    char seq[7];
    int lengt;
    char ack[10];

    if(argc != 4){
      printf("Problem with the number of arguments\n" ); 
      exit(EXIT_FAILURE);
    }
    int PORT=atoi(argv[2]);
    char *filename=argv[3];
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&servaddr1, 0, sizeof(servaddr));

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); 

    
    int n, len; 
        
    sendto(sockfd, (const char *)msg, strlen(msg), 
        0, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)); 
    printf("SYN sent.\n"); 
            
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                0, (struct sockaddr *) &servaddr, 
                &len); 
    buffer[n] = '\0'; 

    strncpy(cpy,buffer,7); //copier le contenu du buffer dans une variable, syn-ack
    cpy[7] = '\0';
    printf("Server : %s\n", cpy); 

    
    int port=atoi(last(buffer));

    msg="ACK";
    sendto(sockfd, (const char *)msg, strlen(msg), 
        0, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)); 
    printf("ACK sent.\n"); 


    if ( (sockdo = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    //Filling server second socket information
    servaddr1.sin_family = AF_INET; 
    servaddr1.sin_port = htons(port); 
    servaddr1.sin_addr.s_addr = inet_addr(argv[1]); 

    sendto(sockdo, (const char *)filename, strlen(filename), 
        0, (const struct sockaddr *) &servaddr1,  
            sizeof(servaddr1)); 
    printf("File %s sent.\n",filename);  

    fp=fopen("file1","w");
    i=0;
    while (1)
    {
        n=recvfrom(sockdo, (char *)buff, MAXLINE,  
                0, (struct sockaddr *) &servaddr1, 
                &len); 
        buff[n] = '\0';
        //printf("%s\n",buff);
        if (strcmp(buff, "FIN") == 0)
            break;
        strncpy(seq,buff,6); 
        seq[6] = '\0';
        memset(ack,0,strlen(ack));
        strcpy(ack, "ACK");
        strcpy(ack,seq);
        printf("Receiving data number %s\n", seq);
        printf("Sending ACK number %s\n", seq);
        sendto(sockdo, ack, strlen(buff),  
        0, (const struct sockaddr *) &servaddr1, 
            len);
        fwrite(buff+6, 1, n-6, fp);
        memset(buff,0, sizeof(buff));
        i++;
    }
    printf("Fichier recu\n");
    fclose(fp);

    return 0; 
}