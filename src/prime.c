#include "prime.h"

/* Trial division needs to check only odd divisors up to sqrt(x). */
int is_prime(int x) {
    if (x < 2) return 0;
    if (x == 2) return 1;
    if (x % 2 == 0) return 0;
    for (int divisor = 3; divisor <= x / divisor; divisor += 2) {
        if (x % divisor == 0) return 0;
    }
    return 1;
}

int next_prime(int x) {
    while (!is_prime(x)) {
        x++;
    }
    return x;
}
