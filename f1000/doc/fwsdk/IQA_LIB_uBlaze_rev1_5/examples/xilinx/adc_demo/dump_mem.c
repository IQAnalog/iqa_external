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
// File        : dump_mem.c                                                     *
// Author      : ND, RL                                                     	*
// Created     : 06/20/2019                                                     *
//                                                                              *
// Description : Display data read from the capture memory.                     *
//                                                                              *
//*******************************************************************************

#include <stdlib.h>

#include "iqa_f1000_api.h"

#define MAX_MEM_CHUNK_SIZE_IN_BYTES 1024*32

char Buf[MAX_MEM_CHUNK_SIZE_IN_BYTES*sizeof(char)];
char output_ascii_buffer[512*sizeof(char)];

int dump_mem_data(int f1000_id, int ep_id, uint32_t adc_id, uint32_t samples)
{

	int         status = IQA_SUCCESS;
	int         offset;
	uint32_t    i;
	int16_t     *pPtr;
	int16_t     *pTemp;
	int         x;
	char        *pBuf;
	int         samples_read;
	int         k = 0;
	FAR char    *output_ascii_buf;
	int         ascii_val_length;
	char        ascii_val[32];
	FAR int16_t *mem_sample;
	FAR char    *temp_ascii_buf;
	bool        data_present_in_buf = false;

    // Data read from memory is being printed in this example.
    // Alternatively, they can be saved into a file.

	pBuf = &Buf[0];
	output_ascii_buf = &output_ascii_buffer[0];

	memset(pBuf, 0, MAX_MEM_CHUNK_SIZE_IN_BYTES*sizeof(char));
	memset(output_ascii_buf, 0, 512*sizeof(char));

	// Get the input parameters
    offset = 0;
    printf("Reading %ld samples\n", samples);

    //Store the address for future access
    temp_ascii_buf = output_ascii_buf;

    // Read data up to 8K at a time
    while (samples > 0)
    {
    	i = samples > 8192 ? 8192 : samples;

        // Read samples from F1000 memory
        status = iqa_read_cap_memory (f1000_id, ep_id, adc_id == 1 ? MEM_ADC_1 : MEM_ADC_2,
                                      offset, 1, (char *) pBuf, i);

        if (IQA_SUCCESS != status)
    	{
    		printf("Error iqa_read_cap_memory() fails: (%d)\r\n", status);
    		return status;
    	}

        samples -= i;
        offset += i*2;
        samples_read = i;

        // Extend the sign bit if it's a negative number
        pPtr  = (int16_t *) (pBuf);
        pTemp = pPtr;

        for (x = 0; x < i*2; x+= 2)
        {
          if ((*pTemp) & 0x200)
            (*pTemp) |= 0xFC00;
          pTemp++;
        }

        // On the other hand if the output is ASCII, we need to do the conversion
        mem_sample = (FAR int16_t*)pPtr ;
        k = 0;
        if(output_ascii_buf != NULL)
        {
            int ascii_val_in_buf = 0;
            while (k < samples_read)
            {
                ascii_val_length = sprintf(ascii_val,"%d,",*(mem_sample + k));
                if ((output_ascii_buf + ascii_val_length) < (temp_ascii_buf + 512))
                {
                    strcat(output_ascii_buf,ascii_val);
                    output_ascii_buf += ascii_val_length;
                    ascii_val_in_buf += ascii_val_length;
                    k++;
                    data_present_in_buf = true;
                }
                else
                // ascii_buf is full write data out
                {
                    output_ascii_buf = temp_ascii_buf;
                    printf("%s\n", output_ascii_buf);
                    ascii_val_in_buf = 0;
                    *output_ascii_buf = 0;
                    data_present_in_buf = false;
                }
            }

            // Write the remaining data
            if (data_present_in_buf)
            {
                output_ascii_buf = temp_ascii_buf;
                printf("%s\n", output_ascii_buf);
                ascii_val_in_buf = 0;
                *output_ascii_buf = 0;
                data_present_in_buf = false;
            }
        }
    }

  return status;
}
