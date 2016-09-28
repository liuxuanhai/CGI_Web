
#include "random.h"

const int32_t Random::A = 48271;
const int32_t Random::M = 2147483647;
const int32_t Random::Q = Random::M / Random::A;
const int32_t Random::R = Random::M % Random::A;


Random::Random(int32_t seed) :
    seed_(seed)
{
    if(seed_ == 0)
        seed_ = time(NULL);
}

int32_t Random::GetRand()
{
    int32_t temp_seed = A * ( seed_ % Q ) - R * ( seed_ / Q );
    if(temp_seed >= 0)
        seed_ = temp_seed;
    else
        seed_ = temp_seed + M;
    return seed_;
}

uint64_t Random::Rdtsc()
{
#if (defined _MSC_VER && (defined _M_IX86 || defined _M_X64))
    return __rdtsc ();
#elif (defined __GNUC__ && (defined __i386__ || defined __x86_64__))
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return (uint64_t) high << 32 | low;
#elif (defined __SUNPRO_CC && (__SUNPRO_CC >= 0x5100) && (defined __i386 || \
            defined __amd64 || defined __x86_64))
    union {
        uint64_t u64val;
        uint32_t u32val [2];
    } tsc;
    asm("rdtsc" : "=a" (tsc.u32val [0]), "=d" (tsc.u32val [1]));
    return tsc.u64val;
#elif defined(__s390__)
    uint64_t tsc;
    asm("\tstck\t%0\n" : "=Q" (tsc) : : "cc");
    tsc >>= 12;		/* convert to microseconds just to be consistent */
    return(tsc);
#else
    return 0;
#endif
}
