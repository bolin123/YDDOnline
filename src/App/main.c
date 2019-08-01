#include "Sys.h"

int main(void)
{
    SysInitalize();
    while (1)
    {
        SysPoll();
    }
//		return 0;
}

