#include <stddef.h>


void * memset (void *destination, int value, size_t count)
{
    unsigned char * dest;
    dest = (unsigned char *) destination;

    for( size_t i =0U; i< count; i++)
    {
        dest[i] = (unsigned char) value;

    }
    return destination;
}

void * memcpy (void * destination, const void * source, size_t count )
{
    unsigned char * dest;
    dest = (unsigned char *) destination;

    const unsigned char * src;
    src = (const unsigned char *)source;

    for( size_t i =0U; i< count; i++)
    {
        dest[i] = src[i];

    }
    return destination;

}