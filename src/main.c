#include <stdio.h>
#include <sds.h>

int main(int argc, char* argv[]){
    sds s1 = sdsnew("Hello, world!\n");
    printf("%s", s1);
    return 0;
}
