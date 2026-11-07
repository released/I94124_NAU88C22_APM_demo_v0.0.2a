#include "colsonic_apm.h"
#include <stdio.h>
#include "I94100.h"
#include "hwh_uart.h"
#include "string.h"
#include "colsonic_def.h"
/**/
#if 0
void Main_Console_Handle()
{
}
	
#else

static uint32_t on_line_para[200];
enum
{
	NON_ERR= 0X00,
	CHECK_ERR=0X01,
	LEN_ERR=0x03,
	
};

enum
{
	WRITE_ID=0,
	READ_ID,
	
	SESIAL_ID_H_LOC=2,
	SESIAL_ID_L_LOC,
	COMMAND_LOC ,
	
	TOTAL_LEN_LOC=7,
	VALID_DATA_LOC=17,
	
	VALID_GAP=75,
}recieve_data_t;

 
#define CRC_MINUS_LEN     5 //总长度去掉这个值，就是CRC校验的长度
#define DATA_MINUS_LEN    (VALID_DATA_LOC+3)
#define WR_ANSWER_CRC_LEN  4


uint8_t sum_check(uint8_t* p_data,uint16_t len);
uint32_t  setpara_proc(uint32_t* ptr);

uint32_t set_flag=0; 
void getpara(void);

uint8_t  check_result ;
uint8_t* p_rx = NULL;
uint16_t rlen = 0;
uint32_t total_len;

/*
 * Active foreground UART0 tuning parser (the empty #if 0 stub is unused).
 * UART IRQ code collects bytes; this handler checks framed packets and
 * dispatches parameter reads/writes to the legacy APM parameter API.
 * This is a binary tuning protocol, not a text console or PCM transport.
 * printf shares UART0 and can mix text with protocol replies.
 */
void Main_Console_Handle()
{
	p_rx=ap_uart0.uart_rx_data;
	rlen =ap_uart0.uart_rx_len;
	uint32_t err=0; 
	//if the starter and the ending  is right,if not then return
	
	if(p_rx[rlen-2]==0x55&&p_rx[rlen-1]==0xAA&&p_rx[0]==0xAA&&p_rx[1]==0x55 )
	{
		// into the checksum 
		check_result = sum_check(&(p_rx[2]),rlen-CRC_MINUS_LEN);
		//right or wrong,all need to tetern message to the uart send
		if( p_rx[rlen-3] == check_result )//two heads two ends and sum itself
		{
			//if the len matchs 
			total_len= ((uint32_t*)(&p_rx[TOTAL_LEN_LOC]))[0] ; 
			if(total_len!=(rlen-DATA_MINUS_LEN ) )
			{
				err = LEN_ERR;
			}
			else
			{
				err=NON_ERR;
				//into the geting/setting function
				if(p_rx[COMMAND_LOC]==WRITE_ID)
				{
					 //setpara_proc((uint32_t*)(&p_rx[VALID_DATA_LOC]) );
					 memcpy(on_line_para,&p_rx[VALID_DATA_LOC],sizeof(char)*total_len);
					 set_flag=1;
				}
				else  
				{
					getpara();
					
				}
			}
			
		}
		else
		{
			err=CHECK_ERR;
		}
		ap_uart0.uart_rx_len = 0;
		memset(ap_uart0.uart_rx_data,0,sizeof(char)*(ap_uart0.uart_rx_len) );
		//send the output 
		{
			uint16_t index=0;
			ap_uart0.uart_tx_data[index++] = 0xAA;
			ap_uart0.uart_tx_data[index++] = 0x55;
			ap_uart0.uart_tx_data[index++] = p_rx[SESIAL_ID_H_LOC];
			ap_uart0.uart_tx_data[index++] = p_rx[SESIAL_ID_L_LOC];
			ap_uart0.uart_tx_data[index++] = p_rx[COMMAND_LOC];
			ap_uart0.uart_tx_data[index++] = err;
			ap_uart0.uart_tx_data[index++] = sum_check(&p_rx[SESIAL_ID_H_LOC],WR_ANSWER_CRC_LEN);
			ap_uart0.uart_tx_data[index++] = 0x55;
			ap_uart0.uart_tx_data[index++] = 0xAA;
			UART_int_send(ap_uart0.uart_tx_data, index);
		}
	}
	if(1 == set_flag)
	{
		set_flag=0;
		colsonic_setpara_proc(on_line_para );
		memset(on_line_para,0,150*sizeof(int));
	}
	
}

uint8_t sum_check(uint8_t* p_data,uint16_t len)
{
	uint8_t ret=0;
	uint32_t i;
	for(i=0;i<len;i++)
	{
		ret += *(p_data++);
	}
	ret= ~ret;
	return ret;
}


 
void getpara(void)
{
}




#endif















