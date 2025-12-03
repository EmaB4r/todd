#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

// TODO: remove all these useless defines
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
    char path[1024*4];
    char name[256];         
    char current_word[256]; 
    char previous_word[256];
    int current_line;
    int newline;
} file_t;

#define is_blank(x) (x == ' ' || x == '\n' || x == '\t')
#define is_alnum(x) (x!='`' && x!='\0')
void parser_skip_blanks(file_t * file){
    if(file->newline) file->newline=0;
    while(is_blank(file->contents[file->fp])){
        if(file->contents[file->fp]=='\n') {
            file->current_line++;
            file->newline=1;
        }
        file->fp++;
    }
}
int parser_advance(file_t * file){
    int i=0;
    parser_skip_blanks(file);
    if (file->contents[file->fp]=='\0') return 0;
    if (file->contents[file->fp]=='`') {
        file->current_word[0] = '`';
        file->current_word[1] = '\0';
        file->fp++;
        return 1;
    }
    strncpy(file->previous_word, file->current_word, sizeof(file->previous_word)-1);
    file->previous_word[sizeof(file->previous_word)-1] = '\0';
    while(i < (int)sizeof(file->current_word)-1 && 
          !is_blank(file->contents[file->fp]) && 
          is_alnum(file->contents[file->fp])){
        file->current_word[i]=file->contents[file->fp];
        file->fp++;
        i++;
    }
    file->current_word[i]='\0';
    return 1;
}

char* new_path(const char* str1, const char* str2){
    size_t len = strlen(str1) + strlen(str2) + 2;
    char* buf = malloc(len);
    snprintf(buf, len, "%s/%s", str1, str2);
    return buf;
}

typedef struct print_state{
    int stop;
    int nl_used;
    int code_mode;
    char prefix[256];
}print_state_t;
// TODO: maybe use a struct for state tracking -> much cleaner
void print_todo(file_t * file){
    print_state_t state = {0};
    
    strncpy(state.prefix, file->previous_word, sizeof(state.prefix)-1);
    state.prefix[sizeof(state.prefix)-1] = '\0';
    printf("|%s:%d\n", file->path, file->current_line);
    putc('|', stdout);
    parser_advance(file);
    while(!state.stop){
        switch (file->current_word[0]) {
            case '`': 
                if(!state.code_mode){
                    fputs(CODE_BLOCK" ", stdout); 
                    state.code_mode=1;
                } else {
                    fputs(ANSI_RESET_COLOR" ", stdout); 
                    state.code_mode=0;
                }
                break;
            default:
                if (state.nl_used){
                    putc('|', stdout); 
                    state.nl_used=0;
                }
                fputs(file->current_word, stdout);
                putc(' ', stdout);
                break;
        }
        state.stop = !parser_advance(file);
        if(file->newline){
            state.nl_used=1;
            if(state.code_mode) fputs(ANSI_RESET_COLOR, stdout);
            putchar('\n');
            if(strcmp(state.prefix, file->current_word)) 
                state.stop = 1;
            else {
                parser_advance(file);
                if (!strcmp(file->current_word, state.prefix)) parser_advance(file);
            }
        }
    }
    if(state.code_mode) fputs(ANSI_RESET_COLOR, stdout);
    if(!state.nl_used) putc('\n', stdout);
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

void cat(const char* path, const char* filename){
    file_t file = {0};
    file.current_line = 1;
    
    strncpy(file.name, filename, sizeof(file.name)-1);
    file.name[sizeof(file.name)-1] = '\0';
    strncpy(file.path, path, sizeof(file.path)-1);
    file.path[sizeof(file.path)-1] = '\0';
    
    FILE* fin = fopen(path, "r");
    if (!fin) return;
    
    fseek(fin, 0, SEEK_END);
    long fsize = ftell(fin);
    if(fsize <= 0) {
        fclose(fin);
        return;
    }
    fseek(fin, 0, SEEK_SET);
    
    char* str = malloc(fsize+1);
    if(!str) {
        fclose(fin);
        return;
    }
    
    size_t nread = fread(str, 1, fsize, fin);
    str[nread] = '\0';
    file.contents = str;
    
    find_todo(&file);
    fclose(fin);
    free(str);
}

void list_files(const char *base_path) {
    DIR *dir = opendir(base_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue; 

        char *full_path = new_path(base_path, entry->d_name);
        struct stat path_stat;
        stat(full_path, &path_stat);

        if (S_ISREG(path_stat.st_mode)) {
            cat(full_path, entry->d_name);
        } 
        else if (S_ISDIR(path_stat.st_mode)) {
            list_files(full_path); 
        }
        free(full_path);
    }
    closedir(dir);
}

int main() {
    list_files(".");
    return 0;
}