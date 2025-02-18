// TODO: Hello from todd.c
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define DEBUG
#ifdef DEBUG
#define  deb printf("HERE\n");
#endif

typedef struct file{
    char* contents;
    uint64_t fp;
    char name[100];
    char current_word[100];
    char previous_word[100];
    int current_line;
    int newline;
}file_t;


#define is_blank(x) (x == ' ' || x == '\n' || x == '\t')
void parser_skip_blanks(file_t * file){
    if(file->newline) file->newline=0;
    while(is_blank(file->contents[file->fp])){
        if(file->contents[file->fp]=='\n') {
            file->current_line++;
            file->newline=1;
        }
        file->fp ++;
    }
}
int parser_advance(file_t * file){
    if (file->contents[file->fp]=='\0') return 0;
    strcpy(file->previous_word, file->current_word);
    parser_skip_blanks(file);
    int i=0;
    while(!is_blank(file->contents[file->fp]) && file->contents[file->fp]!='\0'){
        file->current_word[i]=file->contents[file->fp];
        file->fp++;
        i++;
    }
    file->current_word[i]=0;
    return 1;
}

char* new_path(char*str1, char*str2){
    char* buf = malloc(1024);
    char* buf_p=buf;
    while(*str1!='\0'){*buf=*str1; buf++, str1++;}
    *buf='/';
    buf++;
    while(*str2!='\0'){*buf=*str2; buf++, str2++;}
    *buf=0;
    return buf_p;
}

// TODO: implement possibility to add colors for urgency 
// RED[THIS IS IMPORTAAAANT]
// GREEN[not important if not done]
void print_todo(file_t * file){
    int stop=0, nl_used=0;
    char todo_prefix[64]={0};
    strcpy(todo_prefix, file->previous_word);
    parser_advance(file);
    while(!stop){
        printf("%s ", file->current_word);
        stop = !parser_advance(file);
        if(file->newline){
            nl_used=1;
            putc('\n', stdout);
            if(strcmp(todo_prefix, file->current_word)) stop = 1;
            else parser_advance(file);
        }
    }
    if(!nl_used) putc('\n', stdout);
    printf("------------------------------------------------------------------------\n");
}

void find_todo(file_t * file){
    int printed_filename=0;
    while(file->contents[file->fp]!='\0'){
        parser_advance(file);
        if(!strcmp("TODO:", file->current_word)){
            if(!printed_filename){
                printed_filename=1; 
                printf("\n%s\n", file->name);
                printf("------------------------------------------------------------------------\n");
            }
            print_todo(file);
        }
    }
}

void cat(char*path, char*filename){
    file_t file={.current_line=0, .fp=0};
    strcpy(file.name, filename);
    
    FILE*fin=fopen(path, "r");
    if (fin==NULL) return;
    fseek(fin, 0, SEEK_END);
    size_t fsize=ftell(fin);
    if(!fsize) return;
    char* str = malloc(fsize+1);
    fseek(fin, 0, SEEK_SET);
    fread(str, 1, fsize, fin);
    str[fsize]='\0';
    file.contents=str;
    
    find_todo(&file);
    fclose(fin);
    free(str);
}

void list_files(char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) return;
    while ((entry = readdir(dir)) != NULL) {
        if(entry->d_type==8 && entry->d_name[0]!='.') {
            char* str = new_path(path, entry->d_name);
            cat(str, entry->d_name);
            free(str);
        }
    }
    
    seekdir(dir, SEEK_SET);
    
    if (dir == NULL) return;
    while ((entry = readdir(dir)) != NULL) {
        if(entry->d_type==4 && entry->d_name[0]!='.'){
            char*newpath=new_path(path, entry->d_name);
            list_files(newpath);
            free(newpath);
        }
    }
    closedir(dir);
}


int main() {
    char* path = calloc(512, 1);
    list_files("./"); 
    return 0;
}