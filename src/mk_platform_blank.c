
#include "mk_platform.h"

/* Insert here the include of gpio driver of your chipset*/

void _mk_gpio_write(mk_gpio_t *gpio, mk_gpio_state_e val){
    if (val == MK_GPIO_LOW){
        // write low value on the gpio port
    }
    else{
        // write high value on the gpio port
    }
}

mk_gpio_state_e _mk_gpio_read(mk_gpio_t *gpio){
    // check the gpio state and return the value:
    // MK_GPIO_LOW if the state is low
    // MK_GPIO_HIGH if the state was high
}