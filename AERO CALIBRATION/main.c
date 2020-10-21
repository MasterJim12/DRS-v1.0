#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 300 // I am going to be using 300Hz because the website says that 333hz is the max refresh

// Need a function that changes the PWM s

int main(void)
{


    /*First when the system is turned on. I want the servo to move up first across a wide range of PWM Values. Once Switch it hit, record PWM Upper and stop.
    * Then I would like it to move downwards fully until it hits a switch, then record the value as PWMLower
    * If the switch is not hit at during the upper or lower bounds, and the pwm is maxed out. The max value will become the new upper and/or lower bound.
    * The upper and lower bound is the n divided by two, to produce the middle bound.
    */


        volatile uint32_t maxPWM = 0; // This variable is made to hold the MAX PWM value when it is found.
        volatile uint32_t minPWM = 0;// This variable is made to hold the minimum PWM value when it is found.

        /* Our servo has a frequency of 300Hz. The period based on this is .003333 Seconds. Or .3333mS
         * Dividing this by 1000 gives a resolution of .000003333S or 3.333uS
         * The median for most servos is at 1.5ms
         * So .000003333 * 450.045 gives us the median of 1.5ms
         */
       volatile uint32_t ui8Adjust = 450;

        ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);//This runs the CPU at 40Mhz

        ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64); // THE PWM is clocked through a divider that has a range from 2 to 64. This runs the PWM clock at 625kHz.

        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); // This enables the PWM Generator 1 Which controls PWM on Pins on Port D
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);// This enables pins on PortD, we need this for the pin that will output the PWM
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);// This enables pint on PortF, we need this for the pin that will take input from the buttons.

        ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0); // This sets our pin to to PWM mode
        ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0); // This configures PWM Generator 0 to handle the signal for our pin

        HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
        HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
        HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0; //The previous lines unlock the GPIO commit Control register
        ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4| GPIO_PIN_0, GPIO_DIR_MODE_IN); // This sets the PF0 and PF4 as inputs
        ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4| GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // This configures the pull up resistors for PF0 and PF4 we need this to tell when the limit switches are pressed.
        ROM_GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // This configures the pull up resistors for PD2 so that we can read when the DRS button is pushed.

        volatile uint32_t ui32PWMClock = SysCtlClockGet() / 64;
        volatile uint32_t ui32Load = (ui32PWMClock / PWM_FREQUENCY ) -1;
        PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
        PWMGenPeriodSet (PWM1_BASE, PWM_GEN_0, ui32Load);

        ROM_PWMPulseWidthSet (PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000); //This sets the min resolution of the servo.
        ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);// This sets the PWM generator to output its signal
        ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0); // This enables the PWM Generator to run


        // This section actually controls the initial startup sequence.
        // This code needs to be reworked in order to accommodate for the bounce of the switch or button.
            while ( minPWM == 0){
                    if (ROM_GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0) == 0x00 ){
                        minPWM = ui8Adjust - 5 ;
                    }
                    else ui8Adjust++;
                    if (ui8Adjust > 600 )
                    {
                        ui8Adjust = 600;
                        minPWM = ui8Adjust;
                    }
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
                    ROM_SysCtlDelay(100000);
                }

            while ( maxPWM == 0){
                           if (ROM_GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4) == 0x00 ){
                               maxPWM = ui8Adjust + 5 ;
                           }
                           else ui8Adjust--;
                           if (ui8Adjust < 300 )
                           {
                               ui8Adjust = 300;
                               maxPWM = ui8Adjust;
                           }
                           ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
                           ROM_SysCtlDelay(100000);
                       }

            volatile uint32_t Done = 0; // This variable is used to create an infinite while loop
            while ( Done == 0){

                // This first part of the while loop searches to see if the DRS button, which should be connected to pin PD2 is being pressed down.
                // If it is the wings will open.
                if (ROM_GPIOPinRead(GPIO_PORTD_BASE,GPIO_PIN_2) != 0x00 ){
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, maxPWM * ui32Load / 1000);
                }
                // This second part scans to see if the DRS button is not being pressed down.
                // If it is not then the wings will close.
                else ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, minPWM * ui32Load / 1000);
                //ROM_SysCtlDelay(10);
            }

}













