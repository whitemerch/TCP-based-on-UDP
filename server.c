#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
    
#define PORT1    1234
#define MAXLINE 1024 
    
// ./server PORT file
int main(int argc, char **argv) { 
    int sockfd, sockdo; 
    char buffer[MAXLINE]; 
    char buff[MAXLINE];
    char stream[MAXLINE];
    char *msg = "SYN-ACK1234"; 
    struct sockaddr_in servaddr, cliaddr, servaddr1; 
    FILE *fp;
    int size;
    char mystring[10];
    char ch;
    
    if(argc != 3){
      printf("Problem with the number of arguments\n" ); 
      exit(EXIT_FAILURE);
    }
    int PORT=atoi(argv[1]);

    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    memset(&servaddr1, 0, sizeof(servaddr1));
        
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 

    //Filling server information for second socket
    servaddr1.sin_family    = AF_INET; // IPv4 
    servaddr1.sin_addr.s_addr = INADDR_ANY; 
    servaddr1.sin_port = htons(PORT1); 
        
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    int len, n; 
    
    len = sizeof(cliaddr);  
    
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                0, ( struct sockaddr *) &cliaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Client : %s\n", buffer); 

    sendto(sockfd, (const char *)msg, strlen(msg),  
        0, (const struct sockaddr *) &cliaddr, 
            len); 
    printf("SYN-ACK sent.\n");  

    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                0, ( struct sockaddr *) &cliaddr, 
                &len); 
    buffer[n] = '\0'; 

    //printf("Client : %s\n", buffer); //probleme ici

    //Creating second socket once connected
    if ( (sockdo = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    if ( bind(sockdo, (const struct sockaddr *)&servaddr1,  
            sizeof(servaddr1)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    n = recvfrom(sockdo, (char *)buff, MAXLINE,  
                0, ( struct sockaddr *) &cliaddr, 
                &len); 
    buff[n] = '\0'; 
    printf("Client : %s\n", buff);
    
    //Openning the file
    fp=fopen(argv[2], "r");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    int i=0;
    while (ch != EOF)
    {   
        fread(stream, 1, sizeof(stream), fp);
        strncpy(buff, stream, MAXLINE);
        printf("%s",stream);
        // sendto(sockdo, buff, strlen(buff),  
        // 0, (const struct sockaddr *) &cliaddr, 
        //     len); 
        // printf("Sending data number %d\n", i);
        // n = recvfrom(sockdo, (char *)buff, MAXLINE,  
        //         0, ( struct sockaddr *) &cliaddr, 
        //         &len); 
        // buff[n] = '\0'; 
        // sprintf(mystring, "%06d", i);
        // //if (buff==mystring){
        // printf("ACK number %s received\n",buff);
        // bzero(buff, MAXLINE);
        // bzero(stream, MAXLINE);
        //}
        //else{

        //}
        i++;
    }
    strcpy(buff, "END");   
    sendto(sockdo, buff, strlen(buff),  
        0, (const struct sockaddr *) &cliaddr, 
            len); 
    printf("Fichier envoye");
    fclose(fp);

    return 0;
}