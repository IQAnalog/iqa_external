//*******************************************************************************
//      __  __                                                                  *
//     /  \/  \                                                                 *
//         \   \                                                                *
//    I Q - A N A L O G                                                         *
//           \   \            IQ-Analog Corp                                    *
//            \__/\__/       www.iqanalog.com                                   *
//                                                                              *
//------------------------------------------------------------------------------*
//                                                                              *
// Copyright (C) 2018-2019 IQ-Analog Corp. All rights reserved.                 *
//                                                                              *
//------------------------------------------------------------------------------*
// IQ-Analog CONFIDENTIAL                                                       *
//------------------------------------------------------------------------------*
// This file is released with "Government Purpose Rights" as defined            *
// in DFARS SUBPART 227.71, clause 252.227-7013.                                *
//------------------------------------------------------------------------------*
//                                                                              *
// File        : main.c                                                         *
// Author      : SS, RL                                                         *
// Created     : 06/10/2019                                                     *
//                                                                              *
// Description : Application to exercise calling IQ-Analog library's APIs.      *
//                                                                              *
//*******************************************************************************

/***************************** Include Files *********************************/
#include <stdio.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xspi.h"        /* SPI device driver */


/*
 *  This is the size of the buffer to be transmitted/received in this example.
 */
#define BUFFER_SIZE 14
#define DELAY 1


/**************************** Type Definitions *******************************/
/*
 * The following data type is used to send and receive data on the SPI
 * interface.
 */
int spi_phase = 0;  // 0 -> rising edge; 1 -> falling edge

/************************** Function Prototypes ******************************/

void clear_SPI_buffers(void);
u32 f1000_spi_write(u32 addr, u32 data);
u32 f1000_spi_read(u32 addr, u32 data);

/************************** Variable Definitions *****************************/

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int SPI_TransferInProgress;

/*
 * The following variables are used to read and write to the  Spi device, they
 * are global to avoid having large buffers on the stack.
 */
u8 ReadBuffer[BUFFER_SIZE];
u8 WriteBuffer[BUFFER_SIZE];
XSpi SpiInstance;     /* The instance of the SPI device */

int spi_init(void)
{
    XSpi_Config *SPI_ConfigPtr;

    int Status;

    // Initialize the SPI driver so that it is ready to use.
    SPI_ConfigPtr = XSpi_LookupConfig(XPAR_AXI_QUAD_SPI_0_DEVICE_ID);
    if (SPI_ConfigPtr == NULL)
        return XST_DEVICE_NOT_FOUND;

    Status = XSpi_CfgInitialize(&SpiInstance, SPI_ConfigPtr, SPI_ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    // Reset the SPI peripheral
    XSpi_Reset(&SpiInstance);

    // Initialize the Interrupt controller so that it is ready to use.
    XSpi_IntrGlobalDisable(&SpiInstance);

    // Perform a self-test to ensure that the hardware was built correctly.
    Status = XSpi_SelfTest(&SpiInstance);
    if (Status != XST_SUCCESS)
    {
        printf ("SPI Self Test Failure, Status = %d\n\r",Status);
        return XST_FAILURE;
    }

    // Set the SPI device to the Master mode for this application
    Status = XSpi_SetOptions(&SpiInstance, XSP_MASTER_OPTION + XSP_MANUAL_SSELECT_OPTION );

    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    spi_phase = 0;

    // Select the SPI Slave.  This asserts the correct SS bit on the SPI bus
    XSpi_SetSlaveSelect(&SpiInstance, 0x01);

    // Start the SPI driver so that interrupts and the device are initialized.
    XSpi_Start(&SpiInstance);
    XSpi_IntrGlobalDisable(&SpiInstance);

    XSpi_SetSlaveSelect(&SpiInstance, 0x01);

    return XST_SUCCESS;
}

void clear_SPI_buffers(void)
{
    int SPI_Count,dly;

    // Initialize the write buffer and read buffer to zero
    for (SPI_Count = 0; SPI_Count < BUFFER_SIZE; SPI_Count++)
    {
        WriteBuffer[SPI_Count] = 0;
        ReadBuffer[SPI_Count] = 0;
    }

    for (dly = 0; dly < 5; dly++)
        ;

}

int set_spi_phase (int phase)
{
    int Status;

    if (phase != spi_phase)
    {
        if (phase == 0)
        {
            Status = XSpi_SetOptions(&SpiInstance, XSP_MASTER_OPTION + XSP_MANUAL_SSELECT_OPTION );
            if (Status != XST_SUCCESS)
            {
                printf("SPI Rising edge Phase set Failure\n\r");
                return XST_FAILURE;
            }
        }
        else
        {
            Status = XSpi_SetOptions(&SpiInstance, XSP_MASTER_OPTION + XSP_MANUAL_SSELECT_OPTION + XSP_CLK_PHASE_1_OPTION );
            if (Status != XST_SUCCESS)
            {
                printf("SPI Falling edge Phase set Failure\n\r");
                return XST_FAILURE;
            }
        }

        spi_phase = phase;
    }

    return XST_SUCCESS;
}

u32 f1000_spi_write(u32 addr, u32 data)
{
    u16 ret_val;
    int Status = 0;
    volatile int i;

    Status = set_spi_phase(0);
    if (Status != XST_SUCCESS) { printf("Pos Edge Set Failure\n\r");   return XST_FAILURE; }

    // Clear the SPI read and write buffers
    clear_SPI_buffers();

    // Put the commands in the write buffer
    WriteBuffer[0] = 0x02;
    WriteBuffer[1] = addr >> 24 ;
    WriteBuffer[2] = addr >> 16 ;
    WriteBuffer[3] = addr >> 8 ;
    WriteBuffer[4] = addr & 0xFF ;
    WriteBuffer[5] = data >> 24 ;
    WriteBuffer[6] = data >> 16 ;
    WriteBuffer[7] = data >> 8 ;
    WriteBuffer[8] = data & 0xFF ;

    // Transmit the data.
    SPI_TransferInProgress = TRUE;
    Status = XSpi_Transfer(&SpiInstance, WriteBuffer, ReadBuffer, 9);
    if (Status != XST_SUCCESS) { printf("Failure\n\r");   return XST_FAILURE; }

    for (i = 0; i < DELAY; i++)
            ;

    ret_val = 0;
    return (ret_val);
}

u32 f1000_spi_read(u32 addr, u32 data)
{
    u32 ret_val;
    int Status = 0;
    volatile int i;

    Status = set_spi_phase(0);
    if (Status != XST_SUCCESS) { printf("Pos Edge Set Failure  1\n\r");   return XST_FAILURE; }

    // Clear the SPI read and write buffers
    clear_SPI_buffers();

    // Put the commands in the write buffer
    WriteBuffer[0] = 0x0B;
    WriteBuffer[1] = addr >> 24 ;
    WriteBuffer[2] = addr >> 16 ;
    WriteBuffer[3] = addr >> 8 ;
    WriteBuffer[4] = addr & 0xFF ;
    WriteBuffer[5] = 0 ;
    WriteBuffer[6] = 0 ;
    WriteBuffer[7] = 0 ;
    WriteBuffer[8] = 0 ;
    WriteBuffer[9] = 0 ;
    WriteBuffer[10] = 1 ;
    WriteBuffer[11] = 2 ;
    WriteBuffer[12] = 3 ;
    WriteBuffer[13] = 4 ;

    // Transmit the data.
    SPI_TransferInProgress = TRUE;
    Status = XSpi_Transfer(&SpiInstance, WriteBuffer, ReadBuffer, 14);
    if (Status != XST_SUCCESS) { printf("Failure 2\n\r");   return Status; }

    for (i = 0; i < DELAY; i++)
            ;

    ret_val = (ReadBuffer[9] << 24) | (ReadBuffer[10] << 16) | (ReadBuffer[11] << 8) | (ReadBuffer[12]);
    ret_val = ((ret_val << 1) & 0xFFFFFFFE) | ((ReadBuffer[13] >> 7) & 0x01);
    return (ret_val);
}


void f1000_select_ep(int dev)
{
    // device value : 1 = Element Processos A, 2 = Element Processos B
    XSpi_SetSlaveSelect(&SpiInstance, dev);
}
