#include <rtthread.h>
#include <board.h>
#define LED_PIN    GET_PIN(E, 3)




void autoaddr_thread_entry(void* parameter)
{	
		rt_thread_delay(20);
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);