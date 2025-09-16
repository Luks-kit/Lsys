#include "lstr.h"

size_t lstrlen(const char* s) {
    size_t len=0; while(s[len]!='\0') len++; return len;
}

int lstrcmp(const char* a, const char* b) {
    size_t i=0;
    while(a[i]!='\0' && b[i]!='\0') { if(a[i]!=b[i]) return a[i]-b[i]; i++; }
    return a[i]-b[i];
}

int lstrncmp(const char* a, const char* b, size_t len) {
    for(size_t i=0;i<len;i++) {
        if(b[i]=='\0') return a[i];
        if(a[i]!=b[i]) return a[i]-b[i];
    }
    return 0;
}

void lstrcpy(char* dest, const char* src) { size_t i=0; while(src[i]!='\0'){dest[i]=src[i]; i++;} dest[i]='\0'; }

void lstrcat(char* dest, const char* src) { size_t len=lstrlen(dest); size_t i=0; while(src[i]!='\0'){dest[len+i]=src[i]; i++;} dest[len+i]='\0'; }

void litoa(int value, char* buf) {
    int i=0; int neg=0;
    if(value==0){ buf[0]='0'; buf[1]='\0'; return; }
    if(value<0){ neg=1; value=-value; }
    while(value){ buf[i++]='0'+(value%10); value/=10; }
    if(neg) buf[i++]='-';
    for(int j=0;j<i/2;j++){ char t=buf[j]; buf[j]=buf[i-1-j]; buf[i-1-j]=t; }
    buf[i]='\0';
}

