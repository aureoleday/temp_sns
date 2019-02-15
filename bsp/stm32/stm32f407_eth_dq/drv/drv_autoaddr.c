#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DOWN_I    GET_PIN(D, 1)
#define DOWN_O    GET_PIN(D, 15)
#define UP_I      GET_PIN(D, 14)
#define UP_O      GET_PIN(D, 0)

#define LED_PIN    GET_PIN(E, 3)

#define FSM_INIT  0x1
#define FSM_PREP  0x2
#define FSM_SYNC  0x4
#define FSM_PUSH  0x8
#define FSM_SUCC  0x10

typedef struct
{
    uint8_t sr;
    uint8_t rxd;
    uint8_t txd;
    uint8_t cnt;
    uint8_t fsm;
    uint8_t auto_addr;
}autoaddr_st;

autoaddr_st autoaddr_inst;

static struct rt_timer last_timer;

static void last_timer_cb(void *parameter)
{
    rt_pin_write(UP_I,1);
    rt_pin_mode(UP_I, PIN_MODE_INPUT_PULLUP);
}

static void autoaddr_rx_cb(void *args)
{    
    rt_pin_mode(UP_I, PIN_MODE_INPUT_PULLUP);
  
    if(rt_pin_read(UP_I) == 0)
        autoaddr_inst.sr = autoaddr_inst.sr<<1;
    else
        autoaddr_inst.sr = (autoaddr_inst.sr<<1) + 1;    
    autoaddr_inst.cnt++;

    if(autoaddr_inst.cnt>=8)
    {
        autoaddr_inst.rxd = autoaddr_inst.sr>>2;
        autoaddr_inst.sr = 0;
        autoaddr_inst.cnt = 0;
        rt_pin_mode(UP_I, PIN_MODE_OUTPUT);
        rt_pin_write(UP_I,1);
        rt_timer_start(&last_timer);
    }
    else if(autoaddr_inst.cnt==7)
    {
        rt_pin_mode(UP_I, PIN_MODE_OUTPUT);
        rt_pin_write(UP_I,0);
        rt_timer_start(&last_timer);
    }
}

static void inaddr_init(void)
{
    rt_pin_attach_irq(UP_O, PIN_IRQ_MODE_FALLING, autoaddr_rx_cb, RT_NULL);
    rt_pin_irq_enable(UP_O, PIN_IRQ_ENABLE);
}

static uint8_t autoaddr_tx(uint8_t data)
{
    uint8_t i;
    uint8_t temp = (data<<2)|0x03;
    uint8_t ret=0;
    uint8_t tx_buf = 0;
    
    for(i=0;i<8;i++)
    {
        rt_pin_write(DOWN_O, (temp>>(7-i))&0x1);
        rt_thread_mdelay(1);
        rt_pin_write(DOWN_I, 0);
        rt_thread_mdelay(1);
        ret = rt_pin_read(DOWN_O);
        tx_buf = (tx_buf<<1)|ret;
        rt_pin_write(DOWN_I, 1);        
    }
    autoaddr_inst.txd = tx_buf;
    return ret;
}

uint8_t drv_autoaddr_get_addr(void)
{
    return autoaddr_inst.auto_addr;
}

uint8_t drv_autoaddr_get_fsm(void)
{
    return autoaddr_inst.fsm;
}

static void addr_out_test(int argc, char**argv)
{
    uint8_t ret;
    if (argc < 2)
    {
        rt_kprintf("Please input'addr_out_test <data>'\n");
        return;
    }    
    
    ret = autoaddr_tx(atoi(argv[1]));
    if((autoaddr_inst.txd&0x03) == 0x01)
        rt_kprintf("wr ok\n",ret);
    else
        rt_kprintf("wr fail\n",ret);
}

static void addr_init(void)
{
    rt_pin_mode(DOWN_I, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(DOWN_O, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(UP_I, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(UP_O, PIN_MODE_INPUT_PULLUP);
  
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
  
    rt_pin_write(DOWN_I, 1);
    rt_pin_write(DOWN_O, 1);
  
    inaddr_init();
  
    autoaddr_inst.cnt = 0;
    autoaddr_inst.sr = 0;
    autoaddr_inst.rxd = 0;
    autoaddr_inst.txd = 0;
    autoaddr_inst.fsm = FSM_INIT;
    autoaddr_inst.auto_addr = 0;
  
    rt_timer_init(&last_timer, "last_time",
                  last_timer_cb, 
                  RT_NULL, 
                  2, 
                  RT_TIMER_FLAG_ONE_SHOT); 
}

static void autoaddr_info(void)
{
    rt_kprintf("sr\trxd\ttxd\tcnt\tfsm\tauto_addr\t\n");
    rt_kprintf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t\n",autoaddr_inst.sr,autoaddr_inst.rxd,autoaddr_inst.txd,autoaddr_inst.cnt,autoaddr_inst.fsm,autoaddr_inst.auto_addr);
}

void drv_led_toggle(uint8_t period)
{
    static uint8_t timeout=0;
    if(timeout >= (period<<1))
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        timeout=0;
    }
    else if(timeout == period)
    {
        rt_pin_write(LED_PIN, PIN_LOW);
        timeout++;
    }
    else
        timeout++;
}

void led_thread_entry(void* parameter)
{	
		rt_thread_delay(20);
    while(1)
    {
        drv_led_toggle(autoaddr_inst.fsm);
        rt_thread_mdelay(100);;
    }
}

void autoaddr_thread_entry(void* parameter)
{	
		rt_thread_delay(200);
    addr_init();
    autoaddr_inst.fsm = FSM_INIT;
		while(1)
		{
        switch(autoaddr_inst.fsm)
        {
            case FSM_INIT:
            {                
                autoaddr_inst.fsm = FSM_PREP;
                rt_thread_mdelay(20);
                break;
            }
            case FSM_PREP:
            {
                if((rt_pin_read(UP_I) == 0)&&(rt_pin_read(UP_O) == 0))
                {
                    autoaddr_inst.auto_addr = 1;
                    rt_pin_irq_enable(UP_O, PIN_IRQ_DISABLE);
                    autoaddr_inst.fsm = FSM_PUSH;
                }
                else
                {
                    autoaddr_inst.fsm = FSM_SYNC;
                }
                rt_thread_mdelay(20);
                break;
            }
            case FSM_SYNC:
            {
                if(autoaddr_inst.rxd != 0)
                {
                    autoaddr_inst.auto_addr = autoaddr_inst.rxd;
                    rt_pin_irq_enable(UP_O, PIN_IRQ_DISABLE);
                    autoaddr_inst.fsm = FSM_PUSH;
                }
                else
                {
                    autoaddr_inst.cnt = 0;
                    autoaddr_inst.sr = 0;
                    autoaddr_inst.rxd = 0;
                    autoaddr_inst.fsm = FSM_SYNC;   
                }
                rt_thread_mdelay(100);
                break;
            }
            case FSM_PUSH:
            {
                autoaddr_tx(autoaddr_inst.auto_addr+1);
                if((autoaddr_inst.txd&0x03) != 0x01)
                    autoaddr_inst.fsm = FSM_PUSH;
                else
                    autoaddr_inst.fsm = FSM_SUCC;  
                rt_thread_mdelay(100);
                break;
            }
            case FSM_SUCC:
            {
                autoaddr_tx(autoaddr_inst.auto_addr+1);
                rt_thread_mdelay(1000);
                break;
            }
            default:
            {
                autoaddr_inst.auto_addr = 0;
                autoaddr_inst.fsm = FSM_INIT;
                rt_thread_mdelay(10);
                break;
            }
        }
				rt_thread_mdelay(100);
		}
}

MSH_CMD_EXPORT(autoaddr_info, show autoaddr info)
MSH_CMD_EXPORT(addr_out_test, auto addr test)
