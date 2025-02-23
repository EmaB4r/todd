// TODO: Hello from todd.c
// here is a `code snippet` for you
// can yo `Snitch` do that?
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#define DEBUG
#ifdef DEBUG
#define  deb printf("HERE\n");
#endif

#define ANSI_BOLD "1"
#define ANSI_UNDERLINED "4"
#define ANSI_ESCAPE "\e["
#define ANSI_BLACK_COLOR	"30"
#define ANSI_RED_COLOR	"31"
#define ANSI_GREEN_COLOR	"32"
#define ANSI_YELLOW_COLOR	"33"
#define ANSI_BLUE_COLOR	"34"
#define ANSI_MAGENTA_COLOR	"35"
#define ANSI_CYAN_COLOR	"36"
#define ANSI_WHYTE_COLOR	"37"
#define ANSI_DEFAULT_COLOR	"39"


#define CODE_BLOCK "\e[48;5;235;38;5;253m"
#define ANSI_RESET_COLOR	"\e[0m"

#define TEXT_BOLD(text) ANSI_ESCAPE ANSI_BOLD "m" text ANSI_RESET_COLOR
#define TEXT_UNDERLINED(text) ANSI_ESCAPE ANSI_UNDERLINED "m" text ANSI_RESET_COLOR

#define TEXT_BLACK(text) ANSI_ESCAPE ANSI_BLACK_COLOR "m" text ANSI_RESET_COLOR
#define TEXT_RED(text) ANSI_ESCAPE ANSI_RED_COLOR "m" text ANSI_RESET_COLOR
#define TEXT_GREEN(text) ANSI_ESCAPE ANSI_GREEN_COLOR "m" text ANSI_RESET_COLOR
#define TEXT_YELLOW(text) ANSI_ESCAPE ANSI_YELLOW_COLOR "m" text ANSI_RESET_COLOR
#define TEXT_BLUE(text) ANSI_ESCAPE ANSI_BLUE_COLOR "m" text ANSI_RESET_COLOR
#define TEXT_MAGENTA(text) ANSI_ESCAPE ANSI_MAGENTA_COLOR "m" text ANSI_RESET_COLOR
#define TEXT_CYAN(text) ANSI_ESCAPE ANSI_CYAN_COLOR "m" text ANSI_RESET_COLOR
#define TEXT_WHYTE(text) ANSI_ESCAPE ANSI_WHYTE_COLOR "m" text ANSI_RESET_COLOR

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
#define is_alnum(x) (x!='`' && x!='\0')
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
    int i=0;
    parser_skip_blanks(file);
    if (file->contents[file->fp]=='\0') return 0;
    if (file->contents[file->fp]=='`') {
        file->current_word[i]=file->contents[file->fp];
        file->fp++;
        i++;
        file->current_word[i]=0;
        return 1;
    }
    strcpy(file->previous_word, file->current_word);
    while(!is_blank(file->contents[file->fp]) && is_alnum(file->contents[file->fp])){
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
    int code_mode=0;
    char todo_prefix[64]={0};
    putc('|', stdout);
    strcpy(todo_prefix, file->previous_word);
    parser_advance(file);
    while(!stop){
        switch (file->current_word[0]) {
            case '`': {
                if(!code_mode){fputs(CODE_BLOCK" ", stdout); code_mode=1;}
                else{fputs(ANSI_RESET_COLOR" ", stdout); code_mode=0;}
            } break;
            default:
                if (nl_used){putc('|', stdout); nl_used=0;}
                printf("%s ", file->current_word);
            break;
        }
        stop = !parser_advance(file);
        if(file->newline){
            nl_used=1;
            puts(ANSI_RESET_COLOR);
            if(strcmp(todo_prefix, file->current_word)) stop = 1;
            else parser_advance(file);
        }
    }
    if(!nl_used) putc('\n', stdout);
}

void find_todo(file_t * file){
    int printed_filename=0;
    while(file->contents[file->fp]!='\0'){
        parser_advance(file);
        if(!strcmp("TODO:", file->current_word)){
            if(!printed_filename){
                printed_filename=1; 
                printf("\n "TEXT_BOLD("%s")"\n", file->name);
                printf("/-----------------------------------------------------------------------\n");
            }
            print_todo(file);
            printf("|-----------------------------------------------------------------------\n");
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