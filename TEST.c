/*
 *  Test program for library, also serving as a usage example
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "macros.h"
#include "displaylist.h"

#define OBJECT_SEGMENT 6

segaddr_t seg_addrs[] = {
    0x060249d8, // gLinkAdultSheathNearDL
    0x06024b58, // gLinkAdultLeftHandOutNearDL
    0x06024d70, // gLinkAdultRightHandHoldingHookshotNearDL
};

int main(int argc, const char** argv)
{
    ZObj obj1;
    ZObj obj2;

    // Read an existing ZObj
    ZObj_Read(&obj1, "object_link_boy.zobj", OBJECT_SEGMENT);
    // Create a new empty ZObj
    ZObj_New(&obj2, OBJECT_SEGMENT);

    for (int i = 0; i < ARRLEN(seg_addrs); i++)
    {
        segaddr_t newSeg;
        int ret = DisplayList_Copy(&obj1, seg_addrs[i], &obj2, &newSeg);
        if (ret != 0)
        {
            printf("%s\n", DisplayList_ErrMsg());
            ZObj_Free(&obj1);
            ZObj_Free(&obj2);
            return EXIT_FAILURE;
        }
        printf("Copied to 0x%08X\n", newSeg);
    }

    ZObj_Write(&obj2, "object_link_boy_2.zobj");

    ZObj_Free(&obj1);
    ZObj_Free(&obj2);
    return EXIT_SUCCESS;
}
