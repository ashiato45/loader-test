#include <stdio.h>
#include <stdlib.h>

int func(int x, int y){
    return 0;
}

int main(int argc, char** argv){
    exit(func(argc, argv[3]));
}