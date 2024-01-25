#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_types.h" //could be removed (not sure)
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/systick.h"
#include "types.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "tm4c123gh6pm.h"
#include "driverlib/uart.h"
#include "MCAL/DIO.h"
#include "MCAL/timer.h"
#include "HAL/BT.h"
#define SYSTICK_FREQ 16000000
#define PRO
bool youcanRead = false;
bool Alarms = true;
bool isSystemOn = false;
bool muted = false;


void SystickHandler()
{
    // timer for 5 seconds
    static int32 count = 0;
    count++;
    if (count >= 10)
    {
        muted = true;
        count = 0;
        // printf("cnt is 7");
    }
}

void Timer0_Init(void);
uint32 arr[] = {1, 1};
int idx = 0;
void Timer0AHandler()
{

    youcanRead = false;
    arr[idx] = TimerValueGet(TIMER0_BASE, TIMER_A);
    idx++;

    if (idx == 2)
    {

        idx = 0;
        youcanRead = true;
    }
    TimerIntClear(TIMER0_BASE,
                  TIMER_CAPA_EVENT);
}

void Timer0_Init(void)
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }

    TimerDisable(TIMER0_BASE, TIMER_A);

    TimerControlStall(TIMER0_BASE, TIMER_A, true);
    TIMER0_TAV_R = 0;
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_SPLIT_PAIR);

    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);

    TimerLoadSet(TIMER0_BASE, TIMER_A, 0xFFFF);

    TimerIntClear(TIMER0_BASE,
                  TIMER_CAPA_EVENT);
    IntPrioritySet(INT_TIMER0A, 1);
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AHandler);

    IntMasterEnable();
    TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT);

    TimerEnable(TIMER0_BASE, TIMER_A);
}

void PortFIntHandler()
{
    uint32_t status = 0;
    status = GPIOIntStatus(GPIO_PORTF_BASE, true);

    if ((status & GPIO_INT_PIN_4) == GPIO_INT_PIN_4)
    {
        if (isSystemOn)
        {
            // make the systick count 5 seconds then it changes muted bool to true
            SysTickDisable();
            SysTickPeriodSet(SYSTICK_FREQ / 2 - 1); // count 1/2 second
            NVIC_ST_CURRENT_R = 0;                  // to start counting from 0
            SysTickIntEnable();
            SysTickIntRegister(SystickHandler);
            SysTickEnable();
        }
        else
        {
            isSystemOn = true;
        }
    }

    if ((status & GPIO_INT_PIN_0) == GPIO_INT_PIN_0)
    {
        // stop the system
        isSystemOn = false;
    }

    GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4 | GPIO_INT_PIN_0);
    GPIOIntClear(GPIO_PORTF_BASE, status);
}

void Bluetooth_Write_String(char *str);   // Transmit a string to HC-05 over Tx5 pin

void Delay(unsigned long counter)
{
    unsigned long i = 0;

    for (i = 0; i < counter; i++)
        ;
}



void Send_Message(char *str, uint32 delay_between_signals, uint32 first_signal_duration, uint32 second_signal_duration, uint32 third_signal_duration, bool BT_msg)
{
    if (BT_msg)
    {
        Bluetooth_Write_String(str);
    }
    TIMER1A_DELAY_MS(delay_between_signals); // delay of turn off
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);

    TIMER1A_DELAY_MS(first_signal_duration);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, ~GPIO_PIN_4);

    TIMER1A_DELAY_MS(delay_between_signals); // delay of turn off

    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);
    TIMER1A_DELAY_MS(second_signal_duration);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, ~GPIO_PIN_4);

    TIMER1A_DELAY_MS(delay_between_signals); // delay of turn off

    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);
    TIMER1A_DELAY_MS(third_signal_duration);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
}









#ifdef PRO


int main()
{
    // enable port  B
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    // enable port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
    }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }
    PortFInit();

    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0); // Disable interrupts initially
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_FALLING_EDGE);

    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
    // note that the interrupt is on the whole port so you have to check in the handler which pin
    // fired it
    GPIOIntRegister(GPIO_PORTF_BASE, PortFIntHandler);
    IntEnable(INT_GPIOF);

    // MAKE PORT_B7 AS Output
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,
                          GPIO_PIN_7 | GPIO_PIN_4);

    // make port 6 and 3 as an input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_5);

    // use port B pin 6 as a input src to clk
    GPIOPinConfigure(GPIO_PB6_T0CCP0);             // configure for peripherals
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_6); // enable the GPIO pins for timer

    HC05_init();
    Timer0_Init();
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, ~GPIO_PIN_4);

    while (1)
    {
        if (isSystemOn)
        {
           
            while (isSystemOn)
            {
                bool sonic = false;
                bool flame = false;
                bool magnetic = false;
                // port B 4 for the led
                // port B 3 is for the flame sensor
                // port B 5 for the magnetic sensor
                //  if flame sensor detects flame
                if (GPIOPinRead(GPIO_PORTB_BASE, (GPIO_PIN_3)) == 0)
                {
                    flame = true;
                }

                // if magnetic sensor senses magnets

                if (GPIOPinRead(GPIO_PORTB_BASE, (GPIO_PIN_5)) == 0)
                {
                    magnetic = true;
                }
                // wait for 10 micro second
                //  trigger the ultra-sonic to get a reading
                TIMER1A_DELAY_MS(10);
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_PIN_7);
                TIMER1A_DELAY_MS(10);
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0);
                // calc the distance
                uint32 res = 1000;
                if (youcanRead)
                {
                    res = (((arr[1] - arr[0]) * 10625) / 10000000);
                    if (res < 80)
                    {
                        sonic = true;
                    }
                    youcanRead = false;
                }

                if (sonic || magnetic || flame || muted)
                {
                    Alarms = true;
                    TIMER1A_DELAY_MS(1e6 / 2); // delay of turn off
                    // send the reciver a pulse of 200 milli second to start listening
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);
                    TIMER1A_DELAY_MS(1e6 / 6); // delay ~200 milli
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
                    if (muted)
                    {
                        muted = false;
                        SysTickDisable();      // disable the timer
                        NVIC_ST_CURRENT_R = 0; // to start counting from 0
                        Send_Message("", 1e6 / 2, 1e6 / 8, 1e6 / 8, 1e6 / 8, false);
                    }
                    else if (flame && sonic && magnetic)
                    {
                        Send_Message("ZZZZ", 1e6 / 2, 1e6 / 4, 1e6 / 4, 1e6 / 4,true);
                    }

                    else if (flame && sonic)
                    {
                        Send_Message("BBBB", 1e6 / 2, 1e6 / 4, 1e6 / 8, 1e6 / 8,true);
                    }

                    else if (flame && magnetic)
                    {
                        Send_Message("AAAA", 1e6 / 2, 1e6 / 4, 1e6 / 8, 1e6 / 4,true);
                    }

                    else if (magnetic && sonic)
                    {
                        Send_Message("CCCC", 1e6 / 2, 1e6 / 4, 1e6 / 4, 1e6 / 8,true);
                    }

                    else if (sonic)
                    {
                        Send_Message("UUUU", 1e6 / 2, 1e6 / 8, 1e6 / 4, 1e6 / 8,true);
                    }

                    else if (magnetic)
                    {
                        Send_Message("MMMM", 1e6 / 2, 1e6 / 8, 1e6 / 4, 1e6 / 4,true);
                    }

                    else if (flame)
                    {
                        Send_Message("FFFF", 1e6 / 2, 1e6 / 8, 1e6 / 8, 1e6 / 4,true);
                    }
                    // turn the led off
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
                }
                else if (Alarms)
                {
                    Bluetooth_Write_String("NNNN");
                    Alarms = false;
                }
            }
        }
        else
        {
            while (isSystemOn == false)
            {
            
                __asm("  wfi");
            } /*System is Stopped */
            
            
        }
    }
}
#endif




void Bluetooth_Write_String(char *str)
{
    while (*str)
    {
        Bluetooth_Write(*(str++));
    }
}

