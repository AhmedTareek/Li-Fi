#include "DIO.h"

void PortFInit()
{
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_DEN_R = 0x11;
    GPIO_PORTF_DIR_R = 0x0;
    GPIO_PORTF_CR_R = 0x11;
}