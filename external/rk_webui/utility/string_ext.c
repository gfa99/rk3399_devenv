#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "string_ext.h"

int str_startsWith(char * str, char * search_str) {
    if ((str == NULL) || (search_str == NULL)) return false;
    return (strstr(str, search_str) == str);
}
