/******************************************************************************
* File Name:   main.c (PPCA Core0)
*
* Description: This is the source code for the OOB Demo for ModusToolbox.
*              
*
* Related Document: See README.md
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>

/*******************************************************************************
* Macros
********************************************************************************/

#define LOOP_DELAY_400MS 400

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Flag to track the button press (paused/running) state; toggled by the BTN1 EPU ISR */
bool button_pressed = true;

/* led_state: true = LED2 blinking is paused; false = LED2 is actively blinking.
   Toggled by the EPU IRQ 0 interrupt on every BTN1 press. */
bool led_state = true;

const cy_stc_sysint_t BTN1_IN_COMBINER_Interrupt_Config =
{
     .intrSrc = EPU_BLK_IRQ_EPU_0, //running on PPCA core value is 5
     .intrPriority = 2
};

void BTN1_IN_COMBINER_Interrupt_Handler(void)
{
    /* Clear the EPU IRQ 0 interrupt flag */
    Cy_PPCA_EPU_ClearInterrupt(EPU_BLK_EPU_IRQ0_HW);
    /* Toggle the LED pause state: running <-> paused */
    led_state = !led_state;
}
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for PPCA CPU0.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    /* led_toggle tracks the current EPU SW event output: true = pin high, false = pin low */
    static bool led_toggle = false;
        
    /* Enable global interrupts */
    __enable_irq();

    NVIC_ClearPendingIRQ((IRQn_Type)BTN1_IN_COMBINER_Interrupt_Config.intrSrc);
    Cy_SysInt_Init(&BTN1_IN_COMBINER_Interrupt_Config, &BTN1_IN_COMBINER_Interrupt_Handler);
    NVIC_EnableIRQ((IRQn_Type) BTN1_IN_COMBINER_Interrupt_Config.intrSrc);

    Cy_SysLib_Delay(LOOP_DELAY_400MS);

    for(;;)
     {
          /* led_state == false: LED is running (not paused by button press) */
        if (!led_state) 
        {
            if (led_toggle == false) 
            {
                /* Drive LED2 pin high via EPU software event */
                led_toggle = true;
                Cy_PPCA_EPU_PU_T2_Generate_SW_Event(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, led_toggle);
            } 
            else 
            {
                /* Drive LED2 pin low via EPU software event */
                led_toggle = false;
                Cy_PPCA_EPU_PU_T2_Generate_SW_Event(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, led_toggle);
            }

            /* Wait 400 ms before the next toggle */
            Cy_SysLib_Delay(LOOP_DELAY_400MS);
        } 
        else 
        {
            /* LED is paused: hold the current pin state and keep polling */
            if (led_toggle == false) 
            {
                Cy_PPCA_EPU_PU_T2_Generate_SW_Event(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, led_toggle);
            } 
            else 
            {
                Cy_PPCA_EPU_PU_T2_Generate_SW_Event(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, led_toggle);
            }

            Cy_SysLib_Delay(LOOP_DELAY_400MS);
        }

    }
}
/* [] END OF FILE */