//compile with: gcc main.c -lncurses

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define true 1
#define false 0

int i;

bool isautocomp = false;
char ** lines;
int linessize = 0;
char filename[500];
char message[1000];
int keystrokes = 0;
char key;
int scrw;
int scrh;
int editw;
int edith;
int cx = 0;
int cy = 0;
int sx = 0;
int sy = 0;
char * blank;

void err(char * s){
    endwin();
    printf("Error: %s\n", s);
    exit(0);
}

char * readfile(char * filename) {
    char * buffer = 0;
    long length;
    FILE * f = fopen (filename, "rb");
    
    if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = malloc (length);
      if (buffer)
      {
        fread (buffer, 1, length, f);
      }
      fclose (f);
    }
    return buffer;
}

void adx_store_data(const char *filepath, const char *data)
{
    FILE *fp = fopen(filepath, "w");
    if (fp != NULL)
    {
        fputs(data, fp);
        fclose(fp);
    }
}

void savefile(char * filepath){
    int size = 0;
    for (i = 0; i < linessize; i++) {
        size += strlen(lines[i]) + 1;
    }
    char * s = malloc(size);
    int curlen = 0;
    for (i = 0; i < linessize; i++) {
        int j;
        for (j = 0; j < strlen(lines[i]) + 1; j++) {
            s[curlen] = lines[i][j];
            curlen++;
        }
        s[curlen-1] = '\n';
    }
    s[curlen] = 0;
    adx_store_data(filepath, s);
    free(s);
}

char * strtok_new(char * string, char const * delimiter){
   static char *source = NULL;
   char *p, *riturn = 0;
   if(string != NULL)         source = string;
   if(source == NULL)         return NULL;
   
   if((p = strpbrk (source, delimiter)) != NULL) {
      *p  = 0;
      riturn = source;
      source = ++p;
   }
   return riturn;
}

void filllines(char * s) {
    lines = malloc(sizeof(char*));

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(s, "ab+");
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    if (sz == 0) fprintf(fp, "\n");
    fclose(fp);
    fp = fopen(s, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        linessize++;
        lines = realloc(lines, sizeof(char*) * linessize);
        lines[linessize-1] = malloc(strlen(line)+1);
        sprintf(lines[linessize-1], "%s", line);
        char * l = lines[linessize-1];
        if (l[strlen(l)-1] == '\n') l[strlen(l)-1] = 0;
    }

    fclose(fp);
    if (line)
        free(line);
}

char * strimpose(char * s, int n,  char * imp){
    char * result = malloc(strlen(s) + 1);
    result[strlen(s)] = 0;
    for (i = 0; i < strlen(s); i++){
        if (i < n) result[i] = s[i];
        else result[i] = imp[i];
    }
    return result;
}

char * strinsert(char * s, int n, char * ins){
    if (n < 0) n = strlen(s);
    int size = strlen(s) + strlen(ins);
    char * result = malloc(size + 1);
    result[size] = 0;
    for(i = 0; i < size; i++){
        if (i < n) result[i] = s[i];
        else if (i < n + strlen(ins)) result[i] = ins[i - n];
        else result[i] = s[i - strlen(ins)];
    }
    return result;
}

char * substr(char * s, int a, int b){
    if (b > strlen(s)) b = strlen(s);
    if (a > strlen(s)) a = strlen(s);
    if (b < a) err("at substr b < a");
    int len = b - a;
    char * result = malloc(len+1);
    result[len] = 0;
    if (len > 0){
        memcpy(result, s+a, len);
    }
    return result;
}

void upfunc(){
    if (isautocomp){
        
    }else{
        if(cy == 0) cx = 0;
        else cy--;
    }
}

void downfunc(){
    if (isautocomp){
        
    } else {
        if (cy == linessize - 1) cx = strlen(lines[linessize-1]);
        else cy++;
    }
}

void leftfunc(){
    if (cx == 0 && cy >= 1){
        cy--;
        cx = strlen(lines[cy]);
    } else {
        cx--;
        int len = strlen(lines[cy]);
        if (cx >= len) cx = len - 1;
    }
}

void rightfunc(){
    if (cx < strlen(lines[cy])) cx++;
    else if (cy < linessize - 1){
        cx = 0;
        cy++;
    }
}

void backfunc(){
    if (cx > 0){
        char * word = lines[cy];  
        int idxToDel = cx-1; 
        memmove(&word[idxToDel], &word[idxToDel + 1], strlen(word) - idxToDel);
        cx--;    
    } else if (cx == 0 && cy > 0) {
        cx = strlen(lines[cy-1]);
        char * tmp = strinsert(lines[cy-1], -1, lines[cy]);
        free(lines[cy-1]);
        lines[cy-1] = tmp;
        free(lines[cy]);
        memmove(&lines[cy], &lines[cy+1], (sizeof(char**)) * (linessize-cy));
        linessize--;
        cy--;
    }
}

void enterfunc(){
    if (isautocomp){
        
    }else{
        
        int spaces = 0;
        while(lines[cy][spaces] == ' ' && spaces < cx) spaces++;
        char * ind = malloc(spaces + 1);
        ind[spaces] = 0;
        for (i = 0; i < spaces; i++) ind[i] = ' ';
        int n = cx;
        if (n > strlen(lines[cy])) n = strlen(lines[cy]);
        char * sub = substr(lines[cy], n, strlen(lines[cy]));
        char * indented = strinsert(sub, 0, ind);
        free(ind);
        free(sub);
        
        char * cut = substr(lines[cy], 0, cx);
        free(lines[cy]);
        lines = realloc(lines, sizeof(char**) * (linessize+1));
        memmove(&lines[cy+1], &lines[cy], sizeof(char**) * (linessize-cy));
        lines[cy] = cut;
        lines[cy+1] = indented;
        linessize++;
        cy++;
        cx = spaces;
    }    
}

void tabfunc(){
    char * line = lines[cy];
    char * ind = "    ";
    char * newline = strinsert(line, 0, ind);
    free(line);
    lines[cy] = newline;
    cx += strlen(ind);
}

void shifttabfunc(){
    char * line = lines[cy];
    int ind = 4;
    int index = 0;
    while (index < ind && line[index] == ' ') index++;
    char * newline = substr(line, index, strlen(line));
    free(line);
    lines[cy] = newline;
    cx -= index;
}

void ctrlkfunc(){
    char * line = lines[cy];
    if (cx > 0) {
        char * newline = substr(line, 0, cx);
        free(line);
        lines[cy] = newline;
    } else {
        if (linessize > 1) {
            memmove(&lines[cy], &lines[cy+1], (sizeof(char**)) * (linessize-cy));
            linessize--;
            free(line);
        } else {
            char * newline = malloc(1);
            newline[0] = 0;
            lines[cy] = newline;
            free(line);
        }
    }
}

void ctrlsfunc(){
    savefile(filename);
}

void ctrlrbfunc(){
    sy -= edith;
    if (sy < 0) sy = 0;
    cy = sy;
}

void ctrlbsfunc(){
    sy += edith;
    if (sy > linessize - edith + 1) sy = linessize - edith + 1;
    cy = sy;
}

void defaultfunc(){
    char c[10];
    sprintf(c, "%c", (char)key);
    int x = cx;
    if (x > strlen(lines[cy])) x = strlen(lines[cy]);
    char * s = strinsert(lines[cy], x, c);
    free(lines[cy]);
    lines[cy] = s;
    cx++;
}


void doinput(){
    key = getch();
    if (key == (char)KEY_UP) upfunc();
    else if (key == (char)KEY_DOWN) downfunc();
    else if (key == (char)KEY_LEFT) leftfunc();
    else if (key == (char)KEY_RIGHT) rightfunc();
    else if (key == (char)KEY_BACKSPACE || key == 127) backfunc();
    else if (key == 'a') defaultfunc();
    else if (key == '\n') enterfunc();
    else if (key == '\t') tabfunc();
    else if (key == (char)KEY_BTAB) shifttabfunc();
    else if (key == 11) ctrlkfunc();
    else if (key == 19) ctrlsfunc();
    else if (key == 29) ctrlrbfunc();
    else if (key == 28) ctrlbsfunc();
    else if (key == 27) {}
    else if (key >= 1 && key <= 26) {}
    else defaultfunc();
    
    //make carot behave
    if (cx < 0) cx = 0;
    if (cy < 0) cy = 0;
    if (cy >= linessize) cy = linessize - 1;
    
    //scroll screen
    if (cx < sx) sx = cx;
    if (cy < sy) sy = cy;
    if (cx > sx + editw - 1) sx = cx - (editw-1);
    if (cy > sy + edith - 2) sy = cy - (edith-2);
    
    keystrokes++;
    if (keystrokes % 20 == 0) {
        savefile(filename);
        sprintf(message, "Autosaved at %d keystrokes", keystrokes);
    }
}

void update(){
    char buf[100];
    sprintf(buf, "%d|", linessize);
    int lnlen = strlen(buf);
    editw = scrw - lnlen;
    edith = scrh;
    //clear screen
    attron(COLOR_PAIR(1));
    for (i = 0; i < scrh - 1; i++) mvprintw(i, 0, blank);
    for (i = 0; i < edith && sy + i < linessize; i++){
        attron(COLOR_PAIR(1));
        char * draw = substr(lines[sy+i], sx, sx + editw);
        mvprintw(i, lnlen, draw);
        free(draw);
        //draw line numbers
        sprintf(buf, "%d|", sy + i + 1);
        attron(COLOR_PAIR(5));
        mvprintw(i, lnlen - strlen(buf), buf);
    }
    //draw carot
    int px = cx;
    int py = cy;
    if (px > strlen(lines[py])) px = strlen(lines[py]);
    attron(COLOR_PAIR(4));
    char * s = malloc(sizeof(char) * 2);
    char c = lines[py][px];
    if (c == 0) c = ' ';
    sprintf(s, "%c", c);
    px -= sx;
    py -= sy;
    px += lnlen;
    if (py < 0) py = 0;
    if (px < 0) px = 0;
    mvprintw(py, px, s);
    free(s);
    
    //message
    attron(COLOR_PAIR(8));
    s = malloc(sizeof(char) * 2);
    sprintf(s, "%c", ' ');
    for (i = 0; i < scrw - 1; i++)
        mvprintw(scrh-1, i, s);
    free(s);
    char * messsub = substr(message, 0, editw-1);
    mvprintw(scrh-1, 0, messsub);
}

int main(int argc, char ** argv) {    
    if (argc == 2) {
        sprintf(filename, "%s", argv[1]);
    } else if (argc == 1) {
        printf("Enter file name: ");
        scanf("%s", filename);
    } else {
        err("Too many arguments");
    }
    
    initscr();          
    raw();
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    filllines(filename);
    use_default_colors();
    init_pair(1, COLOR_BLACK, -1);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, -1);
    init_pair(8, 6, 7);
    scrw = COLS;
    scrh = LINES;
    blank = malloc(scrw + 1);
    for (i = 0; i < scrw; i++) blank[i] = ' ';
    blank[scrw] = 0;
    curs_set(0);
    sprintf(message, "%s", "Welcome!");
    while (key != 27) {
        update();
        doinput();
        refresh();      
    }       
    endwin();
    savefile(filename);
    return 0;
}
