/******************************************************************************
 * @file     main.c
 * @brief
 *           Demonstrate how to implement a composite device. (HID Transfer and keyboard)
 *           Transfer data between USB device and PC through USB HID interface.
 *           A windows tool is also included in this sample code to connect with USB device.
 *           
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NUC2201.h"
#include "HID_Transfer_and_Keyboard.h"


void EnableCLKO(uint32_t u32ClkSrc, uint32_t u32ClkDiv)
{
    /* CLKO = clock source / 2^(u32ClkDiv + 1) */
    CLK->FRQDIV = CLK_FRQDIV_DIVIDER_EN_Msk | u32ClkDiv;

    /* Enable CLKO clock source */
    CLK->APBCLK |= CLK_APBCLK_FDIV_EN_Msk;

    /* Select CLKO clock source */
    CLK->CLKSEL2 = (CLK->CLKSEL2 & (~CLK_CLKSEL2_FRQDIV_S_Msk)) | u32ClkSrc;
}

/*--------------------------------------------------------------------------*/
void SYS_Init(void)
{

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184 MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12 MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock */
    CLK_SetCoreClock(72000000);

    /* Enable module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(USBD_MODULE);

    /* Select module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));
    CLK_SetModuleClock(USBD_MODULE, 0, CLK_CLKDIV_USB(3));


    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set GPB multi-function pins for UART0 RXD and TXD, and Clock Output */
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD | SYS_GPB_MFP_PB8_CLKO);
    SYS->ALT_MFP |=  SYS_ALT_MFP_PB8_CLKO;

    /* Enable CLKO (PB.8) for monitor HCLK. CLKO = HCLK/8 Hz*/
    EnableCLKO((2 << CLK_CLKSEL2_FRQDIV_S_Pos), 2);
}


void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS->IPRSTC2 |=  SYS_IPRSTC2_UART0_RST_Msk;
    SYS->IPRSTC2 &= ~SYS_IPRSTC2_UART0_RST_Msk;

    /* Configure UART0 and set UART0 Baudrate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HXT, 115200);
    UART0->LCR = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    /*
        This sample code demonstrate how to use HID interface to transfer data
        between PC and USB device.
        A demo window tool is also included in "WindowsTool" directory with this
        sample code. User can use it to test data transfer with this sample code.
    */

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init system and multi-function I/O */
    SYS_Init();

    /* Init UART for debug message */
    UART0_Init();

    printf("\n");
    printf("+--------------------------------------------------------+\n");
    printf("|        NuMicro USB Composite Device Sample Code        |\n");
    printf("|               (USB HID Transfer + Keyboard)            |\n");
    printf("+--------------------------------------------------------+\n");
    printf("If PB.15 = 0, just report key 'a'.\n");
    /* Set PB.15 as Quasi-bidirectional mode */
    PB->PMD = (PB->PMD & ~GPIO_PMD_PMD15_Msk) | (GPIO_PMD_QUASI << GPIO_PMD_PMD15_Pos);

    /* Open USB controller */
    USBD_Open(&gsInfo, HID_ClassRequest, NULL);

    /* Endpoint configuration */
    HID_Init();
    /* Start USB device */
    USBD_Start();
    NVIC_EnableIRQ(USBD_IRQn);

    while(1)
    {
        HID_UpdateKbData();
    }
}



/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/

