#ifndef HTML_STRING_H
#define HTML_STRING_H

extern int html_string_map(char * in, char * out, int out_but_len);
extern int URLEncode(char* str, char* result, const int resultSize); 
extern int URLDecode(char* str, char* result, const int resultSize); 
#define //HTML_STRING_H
