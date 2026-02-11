#include "../printf.h"
int main(int argc, char **argv)
{
    printf("Hello, World!\n");
    print("This is a test.\n");
    println("This should be on a new line.");
    int x = 10;
    printf("The value of x is: %d\n", x);
    return 0;
}
