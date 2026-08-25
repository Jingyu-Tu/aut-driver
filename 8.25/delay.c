#include "delay.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"
#endif

void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD
    OS_ERR err; 
    OSSchedLock(&err);
#else
    OSSchedLock();
#endif
}

void delay_osschedunlock(void)
{	
#ifdef CPU_CFG_CRITICAL_METHOD
    OS_ERR err; 
    OSSchedUnlock(&err);
#else
    OSSchedUnlock();
#endif
}

void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
    OS_ERR err; 
    OSTimeDly(ticks, OS_OPT_TIME_PERIODIC, &err);
#else
    OSTimeDly(ticks);
#endif 
}

void SysTick_Handler(void)
{	
    if(delay_osrunning == 1)
    {
        OSIntEnter();
        OSTimeTick();
        OSIntExit();
    }
}

void delay_init(void)
{
#if SYSTEM_SUPPORT_OS
    u32 reload;
#endif
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
#if SYSTEM_SUPPORT_OS
    reload = SystemCoreClock / 8000000;
    reload *= 1000000 / delay_ostickspersec;
    fac_ms = 1000 / delay_ostickspersec;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = reload;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
#endif
}

#if SYSTEM_SUPPORT_OS

void delay_us(u32 nus)
{		
    u32 ticks;
    u32 told, tnow, tcnt = 0;
    u32 reload = SysTick->LOAD;
    ticks = nus * fac_us;
    tcnt = 0;
    delay_osschedlock();
    told = SysTick->VAL;
    while(1)
    {
        tnow = SysTick->VAL;	
        if(tnow != told)
        {	    
            if(tnow < told) tcnt += told - tnow;
            else tcnt += reload - tnow + told;	    
            told = tnow;
            if(tcnt >= ticks) break;
        }  
    };
    delay_osschedunlock();									    
}

void delay_ms(u16 nms)
{	
    if(delay_osrunning && delay_osintnesting == 0)
    {		 
        if(nms >= fac_ms)
        { 
            delay_ostimedly(nms / fac_ms);
        }
        nms %= fac_ms;
    }
    delay_us((u32)(nms * 1000));
}

#else

void delay_us(uint16_t nus)
{
    uint16_t i;
    while(nus--)
    {
        i = 8;
        while(i--);
    }
}

void delay_ms(u16 nms)
{
    uint16_t i;
    while(nms--)
    {
        i = 7243;
        while(i--);
    }
}

#endif