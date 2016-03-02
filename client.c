#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <regex.h>     
#include <openssl/md5.h>

#define ADDRESS "127.0.0.1"
#define FILE_TO_SEND "hello.c"
#define FILENAME "foo.c"
#define BUFSIZE 1024
#define FILE_NOT_PRESENT "ERROR: File not present!\n"

// type 1 -> tcp
// type 2 -> udp

void client(int port, int type) {

    int client_socket;
    ssize_t len;
    struct sockaddr_in remote_addr;
    char buffer[BUFSIZE];
    int file_size;
    FILE *received_file;
    int remain_data = 0;
    char request[1024];
    char *command;
    char req_data[BUFSIZE];
    char temp[1024];
    unsigned char mdbuffer[MD5_DIGEST_LENGTH];
    unsigned char md5sum[MD5_DIGEST_LENGTH];
    int bytes2;
    MD5_CTX mdContext;
    unsigned char md[1024];

    socklen_t sock_len = sizeof(struct sockaddr_in);
    // Zeroing remote_addr struct 
    memset(&remote_addr, 0, sizeof(remote_addr));

    // Construct remote_addr struct 
    remote_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ADDRESS, &(remote_addr.sin_addr));
    remote_addr.sin_port = htons(port);

    if(type == 1) {
        // Create client socket 
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        while (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1);
    }
    else {
        client_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_socket == -1) {
            fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    char ch;
    int _i;
    FILE* fp;
    int bytes;
    int flag = 0;
    int i;
    int k;
    char file_name[1024];
    int upload = 0;
    char _send[BUFSIZE];
    int fd;
    struct stat file_stat;

    while(1) {
        printf("$ ");
        _i = 0;
        flag = 0;
        while((ch=getchar())!='\n')
            request[_i++] = ch;
        request[_i] = '\0';
        strcpy(temp, request);
        upload = 1;

        command = strtok(request, " ");
        if (command != NULL) {
            // IndexGet
            if (strcmp(command, "IndexGet") == 0) {
                command = strtok(NULL, " ");
                if (command!=NULL) {
                    // Flags
                    if(strcmp(command, "--shortlist") == 0) {
                        // Input for Flag
                        command = strtok(NULL, " ");
                        strcpy(req_data, "igs ");
                        if( command != NULL ) {
                            // Take Input 1 for Timestamps
                            strcat(req_data, command);     
                        }
                        command = strtok(NULL, " ");
                        if( command != NULL ) {
                            // Take Input 2
                            strcat(req_data, " ");
                            strcat(req_data, command);
                        }
                        if(type==1)
                            send(client_socket, req_data, sizeof(req_data), 0);
                        else
                            sendto(client_socket, req_data, strlen(req_data), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));

                        if(type == 1)
                            bytes = recv(client_socket, buffer, BUFSIZE, 0);
                        else
                            bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);
                        buffer[bytes] = '\0';
                        if( bytes > 0 ) {
                            printf("Response\n");    
                            printf("%s\n", buffer);
                        }
                        else
                            printf("Error!\n");
                    }
                    if(strcmp(command, "--longlist") == 0) {
                        strcpy(req_data, "igl ");
                        if (type == 1)
                            send(client_socket, req_data, sizeof(req_data), 0);
                        else
                            sendto(client_socket, req_data, strlen(req_data), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
                    
                        while(1){
                            memset(buffer, 0, BUFSIZE);
                            if(type==1)
                                bytes = recv(client_socket, buffer, BUFSIZE, 0);
                            else
                                bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);
                            buffer[bytes] = '\0';
                            if (strcmp(buffer, "TheEnd") == 0)
                                break;
                            printf("%s", buffer);
                        }
                    }
                    if(strcmp(command, "--regex") == 0) {
                        // Regex
                        strcpy(req_data, "igr ");
                        command = strtok(temp, "'");
                        command = strtok(NULL, "'");
                        if( command != NULL ) {
                            // Regex Expression
                            // Take Input 1 for Timestamps
                            strcat(req_data, "'");
                            strcat(req_data, command);
                            strcat(req_data, "'");
                        }
                        if(type ==1)
                            send(client_socket, req_data, sizeof(req_data), 0);
                        else
                            sendto(client_socket, req_data, strlen(req_data), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));

                        while(1){
                            memset(buffer, 0, BUFSIZE);
                            if(type==1)
                                bytes = recv(client_socket, buffer, BUFSIZE, 0);
                            else
                                bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);

                            if (strcmp(buffer, "TheEnd") == 0)
                                break;
                            printf("%s\n", buffer);
                        }
                    }
                }
            }
            // FileHash
            if (strcmp(command, "FileHash") == 0) {
                command = strtok(NULL, " ");
                if(!strcmp(command, "--verify")) {

                    strcpy(req_data, "fhv ");
                    command = strtok(NULL, " ");
                    if(command!=NULL) {
                        strcat(req_data, command);
                    }
                    if(type == 1)
                        send(client_socket, req_data, sizeof(req_data), 0);
                    else
                        sendto(client_socket, req_data, strlen(req_data), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));

                    while(1){
                        memset(buffer, 0, BUFSIZE);
                        if(type==1)
                            bytes = recv(client_socket, buffer, BUFSIZE, 0);
                        else
                            bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);

                        buffer[bytes] = '\0';
                        if (strcmp(buffer, "TheEnd") == 0)
                            break;
                        for(i=0;buffer[i]!='\0';i++) {
                                printf("%c", buffer[i]);
                        }
                        if(type==1)
                            bytes2 = recv(client_socket, mdbuffer, BUFSIZE, 0);
                        else
                            bytes2 = recvfrom(client_socket, mdbuffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);

                        mdbuffer[bytes2] = '\0';
                        printf("Checksum(md5): ");
                        for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) printf("%02x", mdbuffer[_i]);
                        printf("\n");
                    }
    
                }
                if(!strcmp(command, "--checkall")) {
                    strcpy(req_data, "fhc ");
                    if(type ==1)
                        send(client_socket, req_data, sizeof(req_data), 0);
                    else
                        sendto(client_socket, req_data, strlen(req_data), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));

                    while(1){
                        memset(buffer, 0, BUFSIZE);
                        if(type==1)
                            bytes = recv(client_socket, buffer, BUFSIZE, 0);
                        else
                            bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);
                        buffer[bytes] = '\0';
                        if (strcmp(buffer, "TheEnd") == 0)
                            break;  
                        for(i=0;buffer[i]!='\0';i++) {
                                printf("%c", buffer[i]);
                        }
                        if(type==1)
                            bytes2 = recv(client_socket, mdbuffer, BUFSIZE, 0);
                        else
                            bytes2 = recvfrom(client_socket, mdbuffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);
                        mdbuffer[bytes2] = '\0';
                        printf("Checksum(md5): ");
                        for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) printf("%02x", mdbuffer[_i]);
                        printf("\n");
                    }
                    
                }
            }
            // File Upload
            if (strcmp(command, "FileUpload") == 0) {
                command = strtok(NULL, " ");    
                strcpy(req_data, "up ");
                strcat(req_data, command);
                fp = fopen(command, "r");
                if(fp == NULL) {
                    printf(FILE_NOT_PRESENT);    
                    continue;
                }
                printf("Asking for permissions to upload %s\n", command);
                if(type ==1)
                    send(client_socket, req_data, sizeof(req_data), 0);
                else
                    sendto(client_socket, req_data, strlen(req_data), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));

                while(1) {
                    if(type==1)
                        bytes = recv(client_socket, buffer, BUFSIZE, 0); 
                    else
                        bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);
                    buffer[bytes] = '\0';
                    if (strcmp(buffer, "TheEnd") == 0)
                        break;
                    if(!strcmp(buffer, "upy")) 
                        upload = 1;
                    else
                        upload = 0;
                }   
                if(upload) {

                    printf("Upload Allowed!\n");
                    fd = open(command, O_RDONLY);
                    fstat(fd, &file_stat);
                    int ok;
                    while(!feof(fp)) {
                        memset(_send, 0, BUFSIZE);
                        // Read data into send_data
                        bytes = fread(_send, 1, BUFSIZE, fp);
                        _send[bytes] = '\0';
                        ok = 0;
                        //while(_send[ok]!='\0')
                        //    printf("%c", _send[ok++]);
                        if(type ==1)
                            send(client_socket, _send, BUFSIZE, 0);
                        else
                            sendto(client_socket, _send, BUFSIZE, 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
                    }
                    strcpy(_send, "TheEnd");
                    if(type ==1)
                        send(client_socket, _send, BUFSIZE, 0);
                    else
                        sendto(client_socket, _send, BUFSIZE, 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
                    printf("Uploaded!\n");
                    fclose(fp);
                }
                else {
                    printf("Upload Denied\n");
                }
            }
            // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            // File Download
            if (strcmp(command, "FileDownload") == 0) {
                command = strtok(NULL, " ");
                printf("Downloading %s...\n", command);
                strcpy(req_data, "do ");
                strcat(req_data, command);
                if(type==1)
                    send(client_socket, req_data, sizeof(req_data), 0);
                else
                    sendto(client_socket, req_data, strlen(req_data), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
                
                fp = fopen("downloaded", "w");

                while(1){

                    memset(buffer, 0, BUFSIZE);
                    if(type==1)
                        bytes = recv(client_socket, buffer, BUFSIZE, 0); 
                    else
                        bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);

                    buffer[bytes] = '\0';

                    if (strcmp(buffer, FILE_NOT_PRESENT) == 0) { 
                       printf(FILE_NOT_PRESENT); 
                       printf("\n");
                       flag = 1;
                       break;
                    }
                    if (strcmp(buffer, "TheEnd") == 0) 
                        break;
                    for(i=0;buffer[i]!='\0';i++) {
                        fprintf(fp, "%c", buffer[i]);
                    }
                }
                memset(buffer, 0, BUFSIZE);
                fclose(fp);
                            
                if( !flag ) {
                    printf("\n");
                    printf("#### File Stats ####\n");
                    if(type==1)
                        bytes = recv(client_socket, buffer, BUFSIZE, 0); 
                    else
                        bytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);
                    buffer[bytes] = '\0';
                    if(type==1)
                        bytes2 = recv(client_socket, mdbuffer, BUFSIZE, 0); 
                    else
                        bytes2 = recvfrom(client_socket, mdbuffer, BUFSIZE, 0, (struct sockaddr *)&remote_addr, &sock_len);

                    mdbuffer[bytes2] = '\0';
                    //bytes = recv(client_socket, buffer, BUFSIZE, 0);
                    //bytes2 = recv(client_socket, mdbuffer, BUFSIZE, 0);
                    if( bytes > 0 && bytes2 > 0) {
                            printf("%s", buffer);
                            printf("Checksum(md5): ");
                            for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) printf("%02x", mdbuffer[_i]);
                            printf("\n");
                            for(i=0;buffer[i]!='\0';i++) {
                                if ( buffer[i] == 'a' && buffer[i+1] == 'm' && buffer[i+2] == 'e' && buffer[i+3] == ':')    
                                    break;
                            }
                            k = 0;
                            for(_i=0;buffer[i+5+_i]!='\0' && buffer[i+5+_i]!='\n';_i++) {
                                file_name[k++] = buffer[i+5+_i];
                            }

                            MD5_Init (&mdContext);
                            fp = fopen("downloaded", "r");
                            // Md5 Sum
                            while ((bytes = fread(md, 1, 1024, fp)) != 0)
                                MD5_Update(&mdContext, md, bytes);
                            MD5_Final(md5sum, &mdContext);
                            printf("Downloaded File Checksum: ");
                            for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) printf("%02x", md5sum[_i]);
                            printf("\n");
                            //printf("Downloaded File Checksum: \n");
                            //for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) printf("%02x", md5sum[_i]);
                            printf("File Downloaded!\n\n");
                        //else {
                        //    for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) printf("%02x", mdbuffer[_i]);
                        //    printf("\n");
                        //    for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) printf("%02x", md5sum[_i]);
                         //   printf("Downloaded Failed! File Corrupt\n");
                       // }
                        //if(strcmp(md5sum, mdbuffer)==0)
                        //    printf("Same\n");
                        //else
                        //    printf("Diff\n");
                        /*    int _flag = 1;
                            for(_i = 0; _i < MD5_DIGEST_LENGTH; _i++) {
                                if(mdbuffer[_i] != md5sum[_i])
                                    _flag = 0;
                            }
                            if(_flag)
                                printf("Same");
                            else    
                                printf("Diff");
                            rename("downloaded", file_name);*/
                    }
                    else
                        printf("Error !\n");
                }
            }
        }
    }
};

void server(int port, int type) {

    int ssocket;
    int psocket;
    socklen_t sock_len;
    ssize_t len;
    struct sockaddr_in saddr;
    struct sockaddr_in paddr;
    int fd;
    int sent_bytes = 0;
    // Send Files in 256 bytes chunk
    struct stat file_stat;
    int remain_data;
    char buffer[BUFSIZE];

    if(type ==1) {
        ssocket = socket(AF_INET, SOCK_STREAM, 0);
        if (ssocket == -1) {
            printf("[ERROR|SOCKET] %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else {
        ssocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (ssocket == -1) {
            printf("[ERROR|SOCKET] %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    /* Zeroing server_addr struct */
    memset(&saddr, 0, sizeof(saddr));
    /* Construct server_addr struct */
    saddr.sin_family = AF_INET;
    inet_aton(ADDRESS, &(saddr.sin_addr));
    saddr.sin_port = htons(port);

    // ==> Bind
    if ((bind(ssocket, (struct sockaddr *)&saddr, sizeof(struct sockaddr))) == -1) {
        printf("[ERROR|BIND] %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // ==> Listen
    if(type ==1) {
        // Listening to incoming connections
        if ((listen(ssocket, 5)) == -1) {
            printf("[ERROR|LISTEN] %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    /*
    fd = open(FILE_TO_SEND, O_RDONLY);
    if (fd == -1) {
        printf("[ERROR|FILE] %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Get file stats 
    if (fstat(fd, &file_stat) < 0) {
        printf("[ERROR|FSTAT] %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("File Size: \n%d bytes\n", file_stat.st_size);
    */

    sock_len = sizeof(struct sockaddr_in);

    int rdata;
    int bytes;
    char upload[2048];
    char filename[2048];
    //int input;
    char input[2048];
    FILE *fp;
    char _send[BUFSIZE];
    char file_size[2048];
    char temp[2048];
    char *input1;
    char *input2;
    DIR *curdir;
    struct dirent *in_file;
    FILE *common_file;
    FILE *entry_file;
    char buffer2[BUFSIZ];
    char ftype;
    char *regex;
    regex_t regX;
    int reti;
    unsigned char md5sum[MD5_DIGEST_LENGTH];
    MD5_CTX mdContext;
    unsigned char md[1024];
    int i;
    int _i;
    char ch;

    while(1) {
        // Step 1
        // Accept Requests from Client
        // ----------------------------
        //while((ch=getchar())!='\n');
        if(type == 1) {
            psocket = accept(ssocket, (struct sockaddr *)&paddr, &sock_len);
            if (psocket == -1) {
                printf("[ERROR|ACCEPT] %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        while(1) {

        memset(buffer, 0, BUFSIZE);
        if(type ==1) 
            rdata = recv(psocket, buffer, BUFSIZE, 0);
        else 
            rdata = recvfrom(ssocket, buffer, BUFSIZE, 0, (struct sockaddr *)&paddr, &sock_len);
        buffer[rdata] = '\0';

        if (strlen(buffer) > 3 && buffer[0] == 'u' && buffer[1] == 'p') {

            fp = fopen("history", "a");
            i = 2;
            fprintf(fp, "%s", "FileUpload");
            while(buffer[i]!='\0') 
                    fprintf(fp, "%c", buffer[i++]);
            fprintf(fp, "%c", '\n');
            fclose(fp);

            strcpy(upload, buffer+3);
            printf("Client Request to Upload %s\n", upload);
            printf("Taking Permission from file [cors.txt]\n");
            
            FILE *cors;
            char perm[20];
            cors = fopen("cors.txt","r");
            if(cors == NULL) {
                // deny if file is not present
                printf("File not present! - Upload denied\n");
                strcpy(input, "deny");
            }
            else
                fscanf(cors, "%s", input);
    
            if (!strcmp(input, "allow") || !strcmp(input, "deny")) {
                _send[0] = 'u';    
                _send[1] = 'p';
                _send[2] = !strcmp(input, "allow") ? 'y' : 'n';
                _send[3] = '\0';
                if(type==1)
                    send(psocket, _send, BUFSIZE, 0);
                else
                    sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));

                //send(psocket, _send, BUFSIZE, 0);
                strcpy(_send, "TheEnd");
                if(type==1)
                    send(psocket, _send, BUFSIZE, 0);
                else
                    sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
                if(!strcmp(input, "allow")) {
                    printf("Upload Allowed\n");
                    fp = fopen("downloaded", "w");
                    while(1) {
                        if(type ==1)
                            bytes = recv(psocket, buffer, BUFSIZE, 0);
                        else
                            bytes = recvfrom(ssocket, buffer, BUFSIZE, 0, (struct sockaddr *)&paddr, &sock_len);
 
                        if (strcmp(buffer, "TheEnd") == 0)
                            break;
                        buffer[bytes] = '\0';
                        for(i=0;buffer[i]!='\0';i++) {
                            //printf("%c", buffer[i]);
                            fprintf(fp, "%c", buffer[i]);
                        }
                    }
                    fclose(fp);
                    rename("downloaded", upload);
                }
                if(input[0] == 'n')
                    printf("Upload Denied\n");
            }
        }
        if (strlen(buffer) > 3 && buffer[0] == 'd' && buffer[1] == 'o') {

            fp = fopen("history", "a");
            i = 2;
            fprintf(fp, "%s", "FileDownload");
            while(buffer[i]!='\0') {
                fprintf(fp, "%c", buffer[i++]);
            }
            fprintf(fp, "%c", '\n');
            fclose(fp);

            strcpy(filename, buffer+3);
            fp = fopen(filename, "r");
            if(fp == NULL) {
                strcpy(_send, FILE_NOT_PRESENT);
                if(type==1)
                    send(psocket, _send, BUFSIZE, 0);
                else
                    sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
                continue;
            }
            // Find Size and send size

            fd = open(filename, O_RDONLY);
            fstat(fd, &file_stat);
            int ok;
            while(fscanf(fp, "%c", &ch)!=EOF) {
                i = 0;
                memset(_send, 0, BUFSIZE);
                _send[i++] = ch;

                while(i < BUFSIZE && fscanf(fp, "%c", &ch)!=EOF) {
                    _send[i++] = ch;
                }
                _send[i] = '\0';
                // Read data into send_data
                ok = 0;
                //printf("Sending...\n");
                //printf("%s\n", _send);
                //while(_send[ok]!='\0')
                //    printf("%c", _send[ok++]);
                if(type==1)
                    send(psocket, _send, BUFSIZE, 0);
                else
                    sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
            }
            strcpy(_send, "TheEnd");
            if(type==1)
                send(psocket, _send, BUFSIZE, 0);
            else
                sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));

            fclose(fp);
            
            // Send file stats
            strcpy(_send, "File Size: ");
            sprintf(temp, "%d", file_stat.st_size);
            strcat(_send, temp);
            strcat(_send, "\n");
            strcat(_send, "File Name: ");
            strcat(_send, filename);
            strcat(_send, "\n");
            strcat(_send, "Last Modified: ");
            // MD5SUM
            MD5_Init (&mdContext);
            fd = open(filename, O_RDONLY);
            fstat(fd, &file_stat);
            fp = fopen(filename, "r");

            while ((bytes = fread(md, 1, 1024, fp)) != 0)
                MD5_Update(&mdContext, md, bytes);
            MD5_Final(md5sum, &mdContext);
            fclose(fp);

            strcat(_send, ctime(&file_stat.st_mtime));
            if(type==1)
                send(psocket, _send, BUFSIZE, 0);
            else
                sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));

            if(type==1)
                send(psocket, md5sum, BUFSIZE, 0);
            else
                sendto(ssocket, md5sum, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
        }
        if (strlen(buffer) > 4 && buffer[0] == 'i' && buffer[1] == 'g' && buffer[2] == 's') {
            // Indexget --shortlist
            /*
            printf("Recieved Buffer %s\n", buffer);
            strtok(buffer, " ");
            input1 = strtok(NULL, " ");
            if(input1 != NULL) {
                input2 = strtok(NULL, " ");
                if (input2 != NULL) {

                    strcpy(filename, "hello.c");
                    fd = open(filename, O_RDONLY);
                    fstat(fd, &file_stat);

                    // Recieved Both filestamps
                    printf("Input1 %s:", input1);
                    printf("Input2 %s:", input2);

                    struct tm tm;
                    time_t start_time , end_time;

                    
                    if(strptime(input1, "%d-%b-%Y-%H:%M:%S", &tm) == NULL)
                        printf("Wrong-Format. The correct format is:\nDate-Month-Year-hrs:min:sec\n");
                    else
                        start_time = mktime(&tm);

                    if(strptime(input2, "%d-%b-%Y-%H:%M:%S", &tm) == NULL) 
                        printf("Wrong-Format. The correct format is:\nDate-Month-Year-hrs:min:sec\n");
                    else
                        end_time = mktime(&tm);

                    if (difftime(file_stat.st_mtime,start_time) > 0 && difftime(end_time,file_stat.st_mtime) > 0) {
                        printf("Found!");
                    }
                    printf("----------------------------\n\n");
                    printf("Last accessed:\t %s",ctime(&file_stat.st_atime));
                    printf("Last modified:\t %s",ctime(&file_stat.st_mtime));
                    printf("Last changed:\t %s",ctime(&file_stat.st_ctime));                     
                }
                else {
                    // Lesser Inputs
                    strcpy(_send, "Too few arguments\n");
                    send(psocket, _send, BUFSIZE, 0);
                }
            }*/
        }
        if (strlen(buffer) > 3 && buffer[0] == 'i' && buffer[1] == 'g' && buffer[2] == 'l') {
            // Indexget --longlist

            fp = fopen("history", "a");
            fprintf(fp, "%s", "IndexGet --longlist");
            fprintf(fp, "%c", '\n');
            fclose(fp);



            // Remove past contents
            strtok(buffer, " ");
            common_file = fopen("longlist", "w");
            // Open Current Directory
            curdir = opendir(".");
            while ((in_file = readdir(curdir)) != NULL) {
                strcpy(filename, "\n");
                if (!strcmp(in_file->d_name, "."))
                    continue;
                if (!strcmp(in_file->d_name, ".."))    
                    continue;
                if (!strcmp("longlist", in_file->d_name))
                    continue;

                // File stats
                fd = open(in_file->d_name, O_RDONLY);
                fstat(fd, &file_stat);
            
                strcat(filename, "File: ");
                strcat(filename, in_file->d_name);
                strcat(filename, "\n");
                strcat(filename, "Size: ");
                sprintf(file_size, "%d", file_stat.st_size);
                strcat(filename, file_size);
                strcat(filename, "\n"); 
                strcat(filename, "Type: ");
                if(S_ISDIR(file_stat.st_mode)) 
                    strcat(filename, "Directory");
                else if(S_ISLNK(file_stat.st_mode))
                    strcat(filename, "Symbolic Link");
                else
                    strcat(filename, "File");
                strcat(filename, "\n");
                strcat(filename, "Modified on: ");
                strcat(filename, ctime(&file_stat.st_mtime));
                strcat(filename, "Accessed on: ");
                strcat(filename, ctime(&file_stat.st_atime));
                fprintf(common_file, "%s", filename);
                memset(filename, 0, 2048);
            }
            fclose(common_file);
            fp = fopen("longlist", "r");
            fd = open("longlist", O_RDONLY);

            while(!feof(fp)) {
                // Read data into send_data
                memset(_send, 0, BUFSIZE);
                bytes = fread(_send, 1, BUFSIZE, fp);
                _send[bytes] = '\0';
                if(type==1)
                    send(psocket, _send, BUFSIZE, 0);
                else
                    sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
            }
            strcpy(_send, "TheEnd");
            if(type==1)
                send(psocket, _send, BUFSIZE, 0);
            else
                sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
            remove("longlist");
            fclose(fp); 
        }
        if (buffer[0] == 'i' && buffer[1] == 'g' && buffer[2] == 'r') {

            regex = strtok(buffer, "'"); 
            regex = strtok(NULL, "'");
            /* Compile regular expression */
            reti = regcomp(&regX, regex, 0);
            if( reti )
                printf("Could not compile regex\n");

            common_file = fopen("longlist", "w");
            curdir = opendir(".");

            while ((in_file = readdir(curdir)) != NULL) {
                strcpy(filename, "\n");
                if (!strcmp(in_file->d_name, "."))
                    continue;
                if (!strcmp(in_file->d_name, ".."))
                    continue;
                if (!strcmp("longlist", in_file->d_name))
                    continue;

                reti = regexec(&regX, in_file->d_name, 0, NULL, 0);
                if( !reti ){
                    // File stats
                    fd = open(in_file->d_name, O_RDONLY);
                    fstat(fd, &file_stat);

                    strcat(filename, "File: ");
                    strcat(filename, in_file->d_name);
                    strcat(filename, "\n");
                    strcat(filename, "Size: ");
                    sprintf(file_size, "%d", file_stat.st_size);
                    strcat(filename, file_size);
                    strcat(filename, "\n");
                    strcat(filename, "Type: ");
                    if(S_ISDIR(file_stat.st_mode))
                        strcat(filename, "Directory");
                    else if(S_ISLNK(file_stat.st_mode))
                        strcat(filename, "Symbolic Link");
                    else
                        strcat(filename, "File");
                    strcat(filename, "\n");
                    strcat(filename, "Modified on: ");
                    strcat(filename, ctime(&file_stat.st_mtime));
                    strcat(filename, "Accessed on: ");
                    strcat(filename, ctime(&file_stat.st_atime));
                    fprintf(common_file, "%s", filename);
                    memset(filename, 0, 2048);
                }
            }
            fclose(common_file);
            fp = fopen("longlist", "r");
            fd = open("longlist", O_RDONLY);

            while(!feof(fp)) {
                // Read data into send_data
                memset(_send, 0, BUFSIZE);
                bytes = fread(_send, 1, BUFSIZE, fp);
                _send[bytes] = '\0';
                send(psocket, _send, BUFSIZE, 0);
            }
            strcpy(_send, "TheEnd");
            send(psocket, _send, BUFSIZE, 0);
            remove("longlist");
            fclose(fp);
        }
        if (buffer[0] == 'f' && buffer[1] == 'h' && buffer[2] == 'v') {
            
            fp = fopen("history", "a");
            i = 3;
            fprintf(fp, "%s", "FileHash --verify");
            while(buffer[i]!='\0') {
                fprintf(fp, "%c", buffer[i++]);
            }
            fprintf(fp, "%c", '\n');
            fclose(fp);

            input1 = strtok(buffer, " ");
            input1 = strtok(NULL,  " ");
            if(input1!=NULL) {
                // MD5
                MD5_Init (&mdContext);
                fd = open(input1, O_RDONLY);
                fstat(fd, &file_stat);
                fp = fopen(input1, "r");
                if(fp != NULL) {
                    while ((bytes = fread(md, 1, 1024, fp)) != 0)
                        MD5_Update(&mdContext, md, bytes);
                    MD5_Final(md5sum, &mdContext);
                    fclose(fp);
                    memset(filename, 0, BUFSIZE);
                    strcat(filename, "File: ");
                    strcat(filename, input1);
                    strcat(filename, "\n");
                    strcat(filename, "Modified on: ");
                    strcat(filename, ctime(&file_stat.st_mtime));
                    if(type==1)
                        send(psocket, filename, BUFSIZE, 0);
                    else
                        sendto(ssocket, filename, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
                    if(type==1)
                        send(psocket, md5sum, BUFSIZE, 0);
                    else
                        sendto(ssocket, md5sum, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
                }
            }
            strcpy(_send, "TheEnd");
            if(type==1)
                send(psocket, _send, BUFSIZE, 0);
            else
                sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
        }
        if (buffer[0] == 'f' && buffer[1] == 'h' && buffer[2] == 'c') {

            fp = fopen("history", "a");
            fprintf(fp, "%s", "FileHash --checkall");
            fprintf(fp, "%c", '\n');
            fclose(fp);

            // Check all File hashes
            common_file = fopen("longlist", "w");
            // Open Current Directory
            curdir = opendir(".");
            // MD5
            MD5_Init (&mdContext);

            while ((in_file = readdir(curdir)) != NULL) {
                strcpy(filename, "\n");
                if (!strcmp(in_file->d_name, "."))
                    continue;
                if (!strcmp(in_file->d_name, ".."))
                    continue;
                if (!strcmp("longlist", in_file->d_name))
                    continue;

                // File stats
                fd = open(in_file->d_name, O_RDONLY);
                fstat(fd, &file_stat);

                fp = fopen(in_file->d_name, "r");
                // Md5 Sum
                while ((bytes = fread(md, 1, 1024, fp)) != 0)
                    MD5_Update(&mdContext, md, bytes);
                MD5_Final(md5sum, &mdContext);
                fclose(fp);
    
                strcat(filename, "File: ");
                strcat(filename, in_file->d_name);
                strcat(filename, "\n");
                strcat(filename, "Modified on: ");
                strcat(filename, ctime(&file_stat.st_mtime));
                len = strlen(filename);
                filename[len] = '\0';
                if(type==1)
                    send(psocket, filename, BUFSIZE, 0);
                else
                    sendto(ssocket, filename, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));

                //strcat(filename, "Checksum(md5):");
                //strcat(filename, md5sum);
                if(type==1)
                    send(psocket, md5sum, BUFSIZE, 0);
                else
                    sendto(ssocket, md5sum, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
                
                //fprintf(common_file, "%s", filename);
                memset(md, 0, 1024); 
            }
            strcpy(_send, "TheEnd");
            if(type==1)
                send(psocket, _send, BUFSIZE, 0);
            else
                sendto(ssocket, _send, BUFSIZE, 0, (struct sockaddr *)&paddr, sizeof(struct sockaddr));
        }
        }
    }        
};

int main(int argc, char **argv) {

    int type, port, connect;

    printf(" =================== Networking ========================= \n");
    printf(" >>> Type of Connection <<< \n >>> (1) TCP\n >>> (2) UDP\n");
    printf("Enter Type:\n");
    scanf("%d", &type);
    if(type!=1 && type !=2) {   
        type = 1;
        printf("Using default connection type: TCP(1)\n");
    }
    printf("Enter Server Port\n");
    scanf("%d", &port);
    printf("Enter Port to connect to\n");
    scanf("%d", &connect);

    int pid = fork();
    if(!pid)
        client(connect, type);
    else 
        server(port, type);

return 0;
}
