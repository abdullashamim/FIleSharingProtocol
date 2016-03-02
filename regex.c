#Hi
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

int main() {
    int rdata;
    int bytes;
    char upload[2048];
    char filename[2048];
    int input;
    FILE *fp;
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
    char regex[3031];
    int fd;
    regex_t regX;
    int reti;
    struct stat file_stat;

            strcpy(regex, "*.c");
            common_file = fopen("longlist", "w");
            curdir = opendir(".");
            /* Compile regular expression */
            reti = regcomp(&regX, regex, 0);
            if( reti )
                printf("Could not compile regex\n");

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
return 0;
}
