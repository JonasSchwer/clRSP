/******************************************************************************
*
*   Implementation of auxiliary.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

size_t
clrspIsPower(const size_t n,
             const size_t num_primes,
             const size_t *primes)
{
    size_t result = 1;
    size_t reminder = n;
    size_t exponents[num_primes];

    size_t i;
    for (i = 0; i < num_primes; ++i) {
        exponents[i] = 0;
        while ((reminder % primes[i]) == 0) {
            reminder /= primes[i];
            ++exponents[i];
        }
        result *= pow(primes[i], exponents[i]);
    }

    return result;
}

size_t
clrspNearestPower(const size_t n,
                  const size_t num_primes,
                  const size_t *primes)
{
    size_t result;
    size_t temp = n;

    result = clrspIsPower(temp,
                          num_primes,
                          primes);
    while (result != temp) {
        ++temp;
        result = clrspIsPower(temp,
                              num_primes,
                              primes);
    }

    return result;
}
