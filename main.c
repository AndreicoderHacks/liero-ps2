#include <tamtypes.h>
#include <kernel.h>
#include <gsKit.h>

static GSGLOBAL *gsGlobal;

int main()
{
    gsGlobal = gsKit_init_global();
    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ON);

    while (1)
    {
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x10, 0x10, 0x20, 0x00, 0x00));

        for (int y = 0; y < 224; y += 8)
        {
            for (int x = 0; x < 320; x += 8)
            {
                u64 color = GS_SETREG_RGBAQ(x % 255, y % 255, 0x80, 0x00, 0x00);
                gsKit_prim_sprite(gsGlobal, x, y, x + 4, y + 4, 0, color);
            }
        }

        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
