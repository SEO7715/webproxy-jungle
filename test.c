#include <unistd.h>

int main(void) {
    printf("%d, %d, %d\n", STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    return 0;
}