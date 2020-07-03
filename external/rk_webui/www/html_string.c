#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <poll.h>
#include <sys/utsname.h>
#include "../utility/common_web.h"

#define DBG 0

struct html_str_map_s {
	char str[2];
	char code[14];
};

struct html_str_map_s htmlMap[33] = {
{" ", 	/*space*/		"&#32;"},
{"!",	/*exclamation mark*/	"&#33;"},
{"\"",	/*quotation mark*/	"&#34;"},
{"#",	/*number sign*/		"&#35;"},
{"$",	/*dollar sign*/		"&#36;"},
{"%",	/*percent sign*/	"&#37;"},
{"&",	/*ampersand*/		"&#38;"},
{"'",	/*apostrophe*/		"&#39;"},
{"(",	/*left parenthesis*/	"&#40;"},
{")",	/*right parenthesis*/	"&#41;"},
{"*",	/*asterisk*/		"&#42;"},
{"+",	/*plus sign*/		"&#43;"},
{",",	/*comma*/		"&#44;"},
{"-",	/*hyphen*/		"&#45;"},
{".",	/*period*/		"&#46;"},
{"/",	/*slash*/		"&#47;"},
{":",	/*colon*/		"&#58;"},
{";",	/*semicolon*/		"&#59;"},
{"<",	/*less-than*/		"&#60;"},
{"=",	/*equals-to*/		"&#61;"},
{">",	/*greater-than*/	"&#62;"},
{"?",	/*question mark*/	"&#63;"},
{"@",	/*at sign*/		"&#64;"},
{"[",	/*left square bracket*/	"&#91;"},
{"\\",	/*backslash*/		"&#92;"},
{"]",	/*right square bracket*/"&#93;"},
{"^",	/*caret*/		"&#94;"},
{"_",	/*underscore*/		"&#95;"},
{"`",	/*grave accent*/	"&#96;"},
{"{",	/*left curly brace*/	"&#123;"},
{"|",	/*vertical bar*/	"&#124;"},
{"}",	/*right curly brace*/	"&#125;"},
{"~",	/*tilde*/		"&#126;"},
};

int html_string_map(char * in, char * out, int out_but_len) {
	int in_len = strlen(in);
	if (in == NULL) return -1;
	if (out == NULL) return -1;
	if (out_but_len < in_len*6) return -1;

	int j = 0;
	int k = 0;

	if (DBG) ALOGD("in=%s", in);	

	while(in[j] != '\0') {
		if (((in[j] >= '0') && (in[j] <= '9')) || 
			((in[j] >= 'a') && (in[j] <= 'z')) || 
			((in[j] >= 'A') && (in[j] <= 'Z'))) {

			out[k++] = in[j++];
		}else {
			if(DBG) ALOGD("int[%d]=%c", j, in[j]);
			int m=0;
			for (m=0; m<33; m++) {
				if (in[j] == htmlMap[m].str[0]) break;
			}

			if (m<33) {
				int codeLen = strlen(htmlMap[m].code);
				strncpy(out+k, htmlMap[m].code, codeLen); 
				k += codeLen;
			} else {
				ALOGE("unknown character");
				return -1;
			}

			j++;
		}
	}

	out[k] = '\0';
        return 0;
}


#define NON_NUM '0' 

char Char2Num(char ch){ 
	if(ch>='0' && ch<='9')return (char)(ch-'0'); 
	if(ch>='a' && ch<='f')return (char)(ch-'a'+10); 
	if(ch>='A' && ch<='F')return (char)(ch-'A'+10); 
	return NON_NUM; 
} 

int URLEncode(char* str, char* result, const int resultSize) { 
	int i; 
	int j = 0; /* for result index */ 
	char ch; 
	int strSize;

	if ((str == NULL) || (result == NULL) || (resultSize <= 0)) {
		return 0; 
	}

	strSize = strlen(str);
	if (strSize <= 0) return 0;

	for (i=0; (i<strSize) && (j<resultSize); i++) { 
		ch = str[i]; 
		if ((ch >= 'A') && (ch <= 'Z')) { 
			result[j++] = ch; 
		} else if ((ch >= 'a') && (ch <= 'z')) { 
			result[j++] = ch; 
		} else if ((ch >= '0') && (ch <= '9')) { 
			result[j++] = ch;
		} else if(ch == ' '){ 
			result[j++] = '+'; 
		} else { 
			if (j + 3 < resultSize) { 
				sprintf(result+j, "%%%02X", (unsigned char)ch); 
				j += 3; 
			} else { 
				return 0; 
			} 
		} 
	} 

	result[j] = '\0'; 
	return j; 
}


int URLDecode(char* str, char* result, const int resultSize) { 
	char ch, ch1, ch2; 
	int i; 
	int j = 0; /* for result index */ 
	int strSize;

	if ((str == NULL) || (result == NULL) || (resultSize <= 0)) {
		return 0; 
	} 

	strSize = strlen(str);
	if (strSize <= 0) return 0;

	for (i=0; (i<strSize) && (j<resultSize); i++) { 
		ch = str[i];
		switch (ch) { 
			case '+': 
				result[j++] = ' '; 
				break; 

			case '%': 
				if (i+2 < strSize) { 
					ch1 = Char2Num(str[i+1]); 
					ch2 = Char2Num(str[i+2]); 
					if ((ch1 != NON_NUM) && (ch2 != NON_NUM)) { 
						result[j++] = (char)((ch1<<4) | ch2); 

						i += 2; 
						break; 
					} 
				} 

			/* goto default */ 
			default: 
				result[j++] = ch; 
				break; 
		} 
	} 

	result[j] = '\0'; 
	return j; 
}
