#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    system("gnome-terminal -e ./producer");
    system("gnome-terminal -e ./consumer");
}