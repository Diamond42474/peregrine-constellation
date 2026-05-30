#include "c-logger.h"

int main(void)
{
    int ret = 0;

    log_init(LOG_LEVEL_DEBUG);

    LOG_INFO("NEEDS IMPLEMENTATION");

failed:
    return ret;
}