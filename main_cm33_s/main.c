/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Hello World!
*              for ModusToolbox.
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
#include "cybsp.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "cy_system_ppca_init.h"
#include "cybsp.h"
#include <stdio.h>
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
********************************************************************************/

/* These are the addresses where the core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START
#define CORE1_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START
#define PPCA0_IMAGE_SIZE       CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE       CYMEM_CM33_0_S_ppca1_code_SIZE

/* These are the addresses that the other application running on PPCA cores should be using to update. */
#define PPCA_M1_VAR_ADDRESS   0x43020400
#define PPCA_M3_VAR_ADDRESS   0x43040800

#define LOOP_DELAY_1000MS 1000
/*******************************************************************************
* Global Variables
********************************************************************************/
/* For the Retarget-IO (Debug UART) usage */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* DEBUG_UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug DEBUG_UART HAL object */

const cy_stc_ppca_cnfg_ppcaout_input_selector_t ppca_input_btn1 = {
    .inputSelSrc = PPCAIN_SEL_SRC_0,
    .disSynchronizerStage = true,
};

const cy_stc_ppca_cnfg_ppcaout_input_selector_t ppca_input_btn2 = {
    .inputSelSrc = PPCAIN_SEL_SRC_1,
    .disSynchronizerStage = true,
};

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CPU.
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
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the debug UART */
    result = Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    /* UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL DEBUG_UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);
    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Clear terminal and print application banner */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: Hello World\r\n");
    printf("************************************************************\r\n\n");

    /* Enable the PPCA configuration block so the PPCA boundary is
       connected to the physical GPIO pins for button inputs */
    Cy_PPCA_CNFG_Init(CNFG_BTN_IN_HW, &CNFG_BTN_IN_config);
    Cy_PPCA_Enable(CNFG_BTN_IN_HW);

    /* Route USER-BTN1 (SW3) to PPCA input 0 */
    Cy_PPCA_CNFG_PPCA_Input_Selector(CNFG_BTN_INCNFG_HW, &ppca_input_btn1, 0);
    /* Route USER-BTN2 (SW4) to PPCA input 1 */
    Cy_PPCA_CNFG_PPCA_Input_Selector(CNFG_BTN_INCNFG_HW, &ppca_input_btn2, 1);

    /* Enable the EPU so it can process the button input events */
    Cy_PPCA_EPU_Enable(EPU_BLK_HW);

     /* Configure T2 processing units to detect edges on BTN1 and BTN2 */
    Cy_PPCA_EPU_PU_T2_Configure(BTN1_PU_T2_HW, BTN1_PU_T2_INDEX, &BTN1_PU_T2_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(BTN1_PU_T2_HW, BTN1_PU_T2_INDEX, BTN1_PU_T2_ENABLE_MODE);
    Cy_PPCA_EPU_Combo_Configure(BTN1_IN_COMBINER_HW, BTN1_IN_COMBINER_INDEX, &BTN1_IN_COMBINER_combo_config);

    /* Configure the GPIO output T2 unit for LED2 (driven by PPCA Core 0) */
    Cy_PPCA_EPU_PU_T2_Configure(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, &PU_T2_2_GPIO1_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, PU_T2_2_GPIO1_ENABLE_MODE);
    Cy_PPCA_EPU_Combo_Configure(COMBINER_IO1_HW, COMBINER_IO1_INDEX, &COMBINER_IO1_combo_config);

    /* Wire EPU IRQ 0 to the BTN1 detection event and unmask the interrupt */
    Cy_PPCA_EPU_InterruptSourceSelect(EPU_BLK_EPU_IRQ0_HW, false, epuIrqSrc0);
    Cy_PPCA_EPU_SetInterruptMask(EPU_BLK_EPU_IRQ0_HW);

    /* Configure the GPIO output T2 unit for LED3 (driven by PPCA Core 1) */
    Cy_PPCA_EPU_PU_T2_Configure(BTN2_PU_T2_HW, BTN2_PU_T2_INDEX, &BTN2_PU_T2_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(BTN2_PU_T2_HW, BTN2_PU_T2_INDEX, BTN2_PU_T2_ENABLE_MODE);
    Cy_PPCA_EPU_Combo_Configure(BTN2_IN_COMBINER_HW, BTN2_IN_COMBINER_INDEX, &BTN2_IN_COMBINER_combo_config);

    Cy_PPCA_EPU_PU_T2_Configure(PU_T2_3_GPIO2_HW, PU_T2_3_GPIO2_INDEX, &PU_T2_3_GPIO2_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(PU_T2_3_GPIO2_HW, PU_T2_3_GPIO2_INDEX, PU_T2_3_GPIO2_ENABLE_MODE);
    Cy_PPCA_EPU_Combo_Configure(COMBINER_IO2_HW, COMBINER_IO2_INDEX, &COMBINER_IO2_combo_config);

    /* Wire EPU IRQ 1 to the BTN2 detection event and unmask the interrupt */
    Cy_PPCA_EPU_InterruptSourceSelect(EPU_BLK_EPU_IRQ1_HW, false, epuIrqSrc1);
    Cy_PPCA_EPU_SetInterruptMask(EPU_BLK_EPU_IRQ1_HW);

    /* Enable global interrupts before booting PPCA cores */
    __enable_irq();

    /* Boot PPCA Core 0 and Core 1 with their LED-blink applications */
    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    printf("\rMain Core: Hello World!!\r\n\n");
    printf("\rPress the USER BTN1 (SW3) or USER BTN2 (SW4) to pause or resume the blinking of LED2 (D9) and LED3 (D10)\r\n\n");

    for (;;)
    {
         /* Main core toggles its own LED (LED1, D8) every 1000 ms while PPCA
            cores handle LED2 (D9) and LED3 (D10) via EPU button events */
         Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN);
         Cy_SysLib_Delay(LOOP_DELAY_1000MS);
    }
}
/* [] END OF FILE */