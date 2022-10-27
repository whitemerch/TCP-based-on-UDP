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

// ./client IP port
//ip 134.214.202.227
int main(int argc, char **argv) { 
    int sockfd, sockdo; 
    char buffer[MAXLINE]; 
    char buff[MAXLINE];
    char *msg = "SYN"; 
    char *donnee;
    char cpy[128];
    char sport[10];
    struct sockaddr_in     servaddr, servaddr1; 
    FILE *fp;
    int i;
    char mystring[10];

    if(argc != 3){
      printf("Problem with the number of arguments\n" ); 
      exit(EXIT_FAILURE);
    }
    int PORT=atoi(argv[2]);
    
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

    int length = strlen(buffer) - 7; //obtenir le port
    if(length >= 0) {
        strncpy(sport, buffer+7, length);
        sport[length] = '\0';
    }
    int port=atoi(sport);

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

    donnee="Socket 2";
    sendto(sockdo, (const char *)donnee, strlen(donnee), 
        0, (const struct sockaddr *) &servaddr1,  
            sizeof(servaddr1)); 
    printf("Socket openned.\n"); 

    fp=fopen("file1","w");
    i=0;
    while (1)
    {
        recvfrom(sockdo, (char *)buff, MAXLINE,  
                0, (struct sockaddr *) &servaddr1, 
                &len); 
        buffer[n] = '\0';
        if (strcmp(buff, "END") == 0)
            break;
        sprintf(mystring, "%06d", i);
        printf("Receiving data number i %s\n", mystring);
        printf("Sending ACK number %s\n", mystring);
        sendto(sockdo, mystring, strlen(buff),  
        0, (const struct sockaddr *) &servaddr1, 
            len);
        fwrite(buff, 1, sizeof(buff), fp);
        bzero(buff, MAXLINE);
        i++;
    }
    printf("Fichier recu");
    fclose(fp);

    return 0; 
}