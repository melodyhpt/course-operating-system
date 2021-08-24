#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#define MB 1048576;

int inode_check = 0, name_check = 0, smin_check = 0, smax_check = 0;
int inode;
char *name;
double smin, smax;

void find(char path[]);
int char2int(char input[]);
double char2double(char input[]);

int main(int argc, char *argv[]) {
    for(int i=2; i<argc; i = i+2) {
        if(!strcmp(argv[i], "-inode")){
            inode_check = 1;
            inode = char2int(argv[i+1]);
        }
        else if(!strcmp(argv[i], "-name")){
            name_check = 1;
            name = argv[i+1];
        }
        else if(!strcmp(argv[i], "-size_min")){
            smin_check = 1;
            smin = char2double(argv[i+1]);
        }
        else if(!strcmp(argv[i], "-size_max")){
            smax_check = 1;
            smax = char2double(argv[i+1]);
        }
    }

    find(argv[1]);
}

void find(char path[]) {
    DIR *dir;
    struct dirent *entry;

    if((dir = opendir(path)) == NULL)
        perror("opendir error\n");
    else {
        while((entry = readdir(dir)) != NULL) {
            struct stat sb;
            char filepath[200];
            strcpy(filepath, path);
            strcat(filepath, "/");
            strcat(filepath, entry->d_name);
            stat(filepath, &sb);

            double filesize = 0;
            filesize = (double) sb.st_size / (double) MB;

            //check all limitations
            int print = 1;
            if(inode_check == 1 && entry->d_ino != inode) {
                print = 0;
            }
            if(name_check == 1 && strcmp(entry->d_name, name)!=0) {
                print = 0;
            }
            if(smin_check == 1 && filesize < smin) {
                print = 0;
            }
            if(smax_check == 1 && filesize > smax) {
                print = 0;
            }

            //print if qualified
            if(print == 1) {
                if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."));
                else printf("%s %ld %fMB\n", filepath, (long)entry->d_ino, filesize);
            }

            //enter sub-directory
            if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."));
            else {
                switch (sb.st_mode & S_IFMT) {
                    case S_IFDIR:  
                        find(filepath);
                        break;
                }
            }
        }
    }
}

int char2int(char input[]){
    int output = 0;
    for(int i=0; i<strlen(input); i++) {
        output = output*10 + (input[i] - '0');
    }

    return output;
}

double char2double(char input[]){
    double output = 0, temp = 0;
    int i=0;
    while(i<strlen(input) && (input[i] != '.')) {
        output = output*10 + (input[i] - '0');
        i++;
    }

    for(int j = strlen(input)-1; j>i; j--) {
        temp = (temp / 10) + ((double)(input[j] - '0') / 10);
    }
    
    output = output + temp;
    return output;
}