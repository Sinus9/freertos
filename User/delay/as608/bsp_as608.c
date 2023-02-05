/**
  ******************************************************************************
  * @file    bsp_as608.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ????????????
  ******************************************************************************
  * @attention
  *
  * ?????:???STM32 F103-??? ??????  
  * ???    :http://www.firebbs.cn
  * ???    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
	
#include "./as608/bsp_as608.h"
#include "./usart/rx_data_queue.h"
//RT�ӳٺ���
#include  "delay.h"
//#include "RT_Delay.h"

//#include "bsp_SysTick.h"
#include "./usart/bsp_usart.h" 

/*???????????*/
#define AS608_DELAY_MS(x)    delay_ms(x)

uint32_t AS608_Addr = 0xFFFFFFFF;             /*??????????????*/


static void NVIC_Configuration(void);
static void AS608_SendData(uint8_t data);
static void AS608_PackHead(void);
static void SendFlag(uint8_t flag);
static void SendLength(uint16_t length);
static void Sendcmd(uint8_t cmd);
static void SendCheck(uint16_t check);


 /**
  * @brief  ????????????��??????NVIC
  * @param  ??
  * @retval ??
  */	
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* ????????��??????????? */
 // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  /* ????USART??��?? */
  NVIC_InitStructure.NVIC_IRQChannel = AS608_USART_IRQ;
  /* ?????????*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
  /* ??????? */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* ????��? */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ?????????NVIC */
  NVIC_Init(&NVIC_InitStructure);
  /* ?????��????TouchOut?? */
  NVIC_InitStructure.NVIC_IRQChannel = AS608_TouchOut_INT_EXTI_IRQ;
    /* ?????????*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
  /* ??????????? */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  /* ?????????NVIC */
  NVIC_Init(&NVIC_InitStructure); 
}


  /**
  * @brief  AS608_TouchOut????
  * @param  ??
  * @retval ??
  */
void AS608_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	EXTI_InitTypeDef EXTI_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
	/*????????GPIO??????*/
	RCC_APB2PeriphClockCmd(AS608_TouchOut_INT_GPIO_CLK,ENABLE);
  AS608_USART_GPIO_APBxClkCmd(AS608_USART_GPIO_CLK, ENABLE);
  /*?????????????*/
	AS608_USART_APBxClkCmd(AS608_USART_CLK, ENABLE);
												
	/* ???? NVIC ?��?*/
	NVIC_Configuration();
  
	/* TouchOut???????GPIO */	
  GPIO_InitStructure.GPIO_Pin = AS608_TouchOut_INT_GPIO_PIN;
  /* ????????????? */	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(AS608_TouchOut_INT_GPIO_PORT, &GPIO_InitStructure);

	/* ???EXTI?????? */
  GPIO_EXTILineConfig(AS608_TouchOut_INT_EXTI_PORTSOURCE, AS608_TouchOut_INT_EXTI_PINSOURCE); 
  EXTI_InitStructure.EXTI_Line = AS608_TouchOut_INT_EXTI_LINE;
	
	/* EXTI??��??? */
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	/* ????/??????��? */
  EXTI_InitStructure.EXTI_Trigger =EXTI_Trigger_Rising_Falling;
  /* ????��? */	
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  /*??USART Tx??GPIO?????????????*/
  GPIO_InitStructure.GPIO_Pin = AS608_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(AS608_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  /*??USART Rx??GPIO???????????????*/
	GPIO_InitStructure.GPIO_Pin = AS608_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(AS608_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	/*???????????????*/
	/*???��?????*/
	USART_InitStructure.USART_BaudRate = AS608_USART_BAUDRATE;
	/*???? ?????????*/
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	/*??????��*/
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	/*????��??��*/
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	/*?????????????*/
	USART_InitStructure.USART_HardwareFlowControl = 
	                                USART_HardwareFlowControl_None;
	/*???��?????????????*/
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/*???????????????*/
	USART_Init(AS608_USART, &USART_InitStructure);
	
	/*??????????��?*/
	USART_ITConfig(AS608_USART, USART_IT_RXNE, ENABLE);	
	USART_ITConfig(AS608_USART, USART_IT_IDLE, ENABLE ); //??????????????��? 	
	
	/*??????*/
	USART_Cmd(AS608_USART, ENABLE);	
 	
}



/**
  * @brief  AS608_USART???????????��?????? 
  * @param  data;?????????
  */
static void AS608_SendData(uint8_t data)
{
  USART_SendData(AS608_USART,data);
  while(USART_GetFlagStatus(AS608_USART,USART_FLAG_TXE) == RESET);
}


/**
  * @brief  AS608_USART????????????????????? 
  * @param  ??
  */
static void AS608_PackHead(void)
{ 
  /*???*/
  AS608_SendData(0xEF);
  AS608_SendData(0x01);	
  
  /*????????*/
  AS608_SendData(AS608_Addr>>24);
  AS608_SendData(AS608_Addr>>16);	
  AS608_SendData(AS608_Addr>>8);
  AS608_SendData(AS608_Addr);	
}


/**
  * @brief  ????????
  * @param  flag:?????��
  * @retval ??
  */
static void SendFlag(uint8_t flag)
{
  AS608_SendData(flag);
}


/**
  * @brief  ?????????
  * @param  length:??????
  * @retval ??
  */
static void SendLength(uint16_t length)
{
	AS608_SendData(length>>8);
  AS608_SendData(length);
}


/**
  * @brief  ?????????
  * @param  cmd;?????
  * @retval ??
  */
static void Sendcmd(uint8_t cmd)
{
	AS608_SendData(cmd);
}


/**
  * @brief  ????��???
  * @param  check:???��
  * @retval ??
  */
static void SendCheck(uint16_t check)
{
	AS608_SendData(check>>8);
	AS608_SendData(check);
}


/**
  * @brief  ????????????????
  * @param  *i:????????????
  * @retval ??
  */
uint16_t  ReturnFlag( uint16_t *i)
{
   QUEUE_DATA_TYPE *rx_data;
  
   rx_data = cbRead(&rx_queue);  /*??????????????????��???*/
    
   if(rx_data != NULL)           /*??????��??*/
  { 
		/*??????��??????????*/
    QUEUE_DEBUG_ARRAY((uint8_t*)rx_data->head,rx_data->len);
		
    *i=((uint16_t)(*(rx_data->head+9)));     /*?????*/
		
	  cbReadFinish(&rx_queue);    /*???????????????cbReadFinish????????*/
	  
		return *i;
  }
	else
  {	
	  *i=0xff;
		
	  return *i;
  }
}


/**
  * @brief   ????????????????????????? ImageBuffer
  * @param   ??
  * @retval  ?????=00H ???????????????=01H ???????��???
             ?????=02H ???????????????????????=03H ????????? 
  */
uint16_t PS_GetImage(void)
{
  uint16_t temp;
  uint16_t sure,p=0;
	
	AS608_DELAY_MS(3000);       /*?????????????????*/
	
	AS608_PackHead();
	SendFlag(0x01);             /*????????*/
	SendLength(0x03);
	Sendcmd(0x01);              /*???????*/
  temp=0x01+0x03+0x01;
	SendCheck(temp);
	
  AS608_DELAY_MS(500);        /*???????????��??????*/
  
	sure=ReturnFlag(&p);
  
  return  sure;
}


/**
  * @brief  ??ImageBuffer?��????????????????????????CHARBUFFER1??CHARBUFFER2
  * @param  BufferID(????????????)
  * @retval ?????=00H ?????????????????????=01H ???????��???
            ?????=06H ??????????????????????????????=07H ???????????????????????????????????????
            ?????=15H ????????????????��??????????????
  */
uint16_t PS_GenChar(uint8_t BufferID)
{
  uint16_t temp;
  uint16_t  sure,p=0;
 
  AS608_PackHead();
  SendFlag(0x01);          
  SendLength(0x04);
  Sendcmd(0x02);              /*???????????*/   
  AS608_SendData(BufferID);
  temp = 0x01+0x04+0x02+BufferID;
  SendCheck(temp);
	
  AS608_DELAY_MS(400);
	
  sure=ReturnFlag(&p);
  
  return  sure;
}


/**
  * @brief  ???STM32????????????????
  * @param  PS_Addr????????
  * @retval ?????0?????;1??????????
  */
uint16_t PS_Connect(uint32_t *PS_Addr)
{
	QUEUE_DATA_TYPE *rx_data;	
	
	AS608_PackHead();
	AS608_SendData(0X01);
	AS608_SendData(0X00);
	AS608_SendData(0X00);
	//printf ("11\r\n");
  AS608_DELAY_MS(100);
	//printf ("222\r\n");
	rx_data = cbRead(&rx_queue);  /*??????????????????��???*/ 
	if(rx_data != NULL)           /*??????��??*/
	{ 
		/*??????��??????????*/
		QUEUE_DEBUG_ARRAY((uint8_t*)rx_data->head,rx_data->len);
		
		if(/*?��????????��???????*/
   			*(rx_data->head)	== 0XEF
				  && *(rx_data->head+1)==0X01
				  && *(rx_data->head+6)==0X07)  
		  {
			AS608_INFO("��ַ:0x%x%x%x%x\r\n",*(rx_data->head+2),
																						      *(rx_data->head+3),
																						      *(rx_data->head+4),
																					   	    *(rx_data->head+5));
		  }
			cbReadFinish(&rx_queue);  /*???????????????cbReadFinish????????*/
		  
			return 0;	
	}	
	
  return 1;		
}


/**
  * @brief  ?????? CHARBUFFER1??CHARBUFFER2?��????????
  * @param  ??
  * @retval ?????=00H ??????????????=01H ???????��????????=08H ??????????
  */
uint16_t PS_Match(void)
{
	uint16_t temp;
  uint16_t  sure,p=0;
	
	AS608_PackHead();
	SendFlag(0x01);
	SendLength(0x03);
	Sendcmd(0x03);                /*?????????*/
	temp = 0x01+0x03+0x03;
	SendCheck(temp);
	
  AS608_DELAY_MS(500);
	
  sure=ReturnFlag(&p);
  
  return  sure;

}


/**
  * @brief  ??CHARBUFFER1??CHARBUFFER2 ?��?????????????????��
            ???????CHARBUFFER1??CHARBUFFER2??
  * @param  ??
  * @retval ?????=00H ????????????????=01H ???????��???
            ?????=0aH ????????????????????????????
  */
uint16_t PS_RegModel(void)
{
	uint16_t temp;
  uint16_t sure,p=0;
  	
	AS608_PackHead();
	SendFlag(0x01);
	SendLength(0x03);
	Sendcmd(0x05);                /*??????????*/
	temp = 0x01+0x03+0x05;
	SendCheck(temp);
	
  AS608_DELAY_MS(500);
	
  sure=ReturnFlag(&p);
  
  return  sure;

}


/**
  * @brief  ????????ID
  * @param  ??
  * @retval ?????????
  */
uint32_t GET_NUM(void)
{
	uint32_t num;
	//printf("id:%d", num);
	
	AS608_INFO("ID:num=%d\r\n",num);
	return num;
}


/**
  * @brief  ?? CHARBUFFER1 ?? CHARBUFFER2 ?��????????�� PageID ??flash ?????��?��?
  * @param  BufferID:?????????
  * @param 	PageID:????��?��?
  * @retval ?????=00H ????????????????=01H ???????��???
            ?????=0bH ??? PageID ????????��???????=18H ???�� FLASH ????
  */
uint16_t PS_StoreChar(uint8_t BufferID,uint16_t PageID)
{
	uint16_t temp;
  uint16_t sure,p=0;
 
	AS608_PackHead();
	SendFlag(0x01);
	SendLength(0x06);
	Sendcmd(0x06);                /*?��??????*/
	AS608_SendData(BufferID);
  AS608_SendData(PageID>>8);
	AS608_SendData(PageID);
	temp = 0x01+0x06+0x06+BufferID
	       +(PageID>>8)+(uint8_t)PageID;
	SendCheck(temp);
	
	AS608_DELAY_MS(500);
	
  sure=ReturnFlag(&p);
  
  return  sure;

	
}

/**
  * @brief  ??CHARBUFFER1??CHARBUFFER2?��???????????????????????????
  * @param  BufferID:????????
  * @param  StartPage:????
  * @param  PageNum:???
  * @param  p:???????
  * @retval ensure:?????,*p:????????????��
  */
uint16_t PS_HighSpeedSearch(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,uint16_t *p)
{
	uint16_t temp;
  uint16_t ensure;
  QUEUE_DATA_TYPE *rx_data;
	
	AS608_PackHead();
	SendFlag(0x01);
	SendLength(0x08);
	Sendcmd(0x1b);                 /*???????????????*/
	AS608_SendData(BufferID);
	AS608_SendData(StartPage>>8);
	AS608_SendData(StartPage);
	AS608_SendData(PageNum>>8);
	AS608_SendData(PageNum);
	temp = 0x01+0x08+0x1b+BufferID
	       +(StartPage>>8)+(uint8_t)StartPage
	       +(PageNum>>8)+(uint8_t)PageNum;
	SendCheck(temp);
	
 	AS608_DELAY_MS(500);
	
  rx_data = cbRead(&rx_queue);  /*??????????????????��???*/
  if(rx_data != NULL)           /*??????��??*/
  {
		/*??????��??????????*/
	  QUEUE_DEBUG_ARRAY((uint8_t*)rx_data->head,rx_data->len);
    
    ensure=((uint16_t)(*(rx_data->head+9)));   /*?????*/
    
    /*???????????????????ID??*/
	  *p=((*(rx_data->head+10))<<8)+(*(rx_data->head+11));
    
	  cbReadFinish(&rx_queue);    /*???????????????cbReadFinish????????*/
    
	  return ensure;
  }
	else
	{	
	  ensure=0xff;
    
	  return ensure;
	}
}



/**
  * @brief  ??? flash ??????????ID??????N????????
  * @param  PageID:????????
  * @param  N:???????????
  * @retval ?????=00H ??????????????????=01H ???????��???
            ?????=10H ????????????
  */
uint16_t PS_DeletChar(uint16_t PageID,uint16_t N)
{
	uint16_t temp;
  uint16_t sure,p=0;
 
	
	AS608_PackHead();
	SendFlag(0x01);//????????
	SendLength(0x07);
	Sendcmd(0x0C);                /*???????????????*/
	AS608_SendData(PageID>>8);
  AS608_SendData(PageID);
	AS608_SendData(N>>8);
	AS608_SendData(N);
	temp = 0x01+0x07+0x0C
	       +(PageID>>8)+(uint8_t)PageID
	       +(N>>8)+(uint8_t)N;
	SendCheck(temp);
	
	AS608_DELAY_MS(400);
	
	sure=ReturnFlag(&p);
  
  return  sure;

	
}
/**
  * @brief  ??? flash ?????????????????
  * @param  ??
  * @retval ?????=00H ???????????????=01H ???????��????????=11H ?????????
  */
uint16_t  PS_Empty(void)
{
	uint16_t temp;
  uint16_t sure,p=0;
 
	
	AS608_PackHead();
	SendFlag(0x01);//????????
	SendLength(0x03);
	Sendcmd(0x0D);
	temp = 0x01+0x03+0x0D;
	SendCheck(temp);
	
	AS608_DELAY_MS(400);
	
  sure=ReturnFlag(&p);
  
  return  sure;

}


/**
  * @brief  ??????????
  * @param  ensure:?????
  * @retval ??
  */
void  ShowErrMessage( uint16_t  ensure) 
{
  switch(ensure)
	{
    case  0x00:
			AS608_INFO("OK\r\n");
		break;
		
	  case  0x01:
			AS608_INFO("????????????\r\n");
		break;
		
	  case  0x02:
	    AS608_INFO("????????��??????\r\n");
		break;
		
	  case  0x03:
	    AS608_INFO("????????????\r\n\r\n");
		break;
		
	  case  0x04:
	    AS608_INFO("????????��????????????????\r\n\r\n");
		break;
	  
		case  0x05:
	    AS608_INFO("?????????????????????????\r\n\r\n");
		break;
		
	  case  0x06:
	    AS608_INFO("????????????????????\r\n\r\n");
		break;
		
	  case  0x07:
	    AS608_INFO("??????????????????????????????��??????????????\r\n");
		break;
		
	  case  0x08:
	    AS608_INFO("???????\r\n\r\n");
		break;
		
	  case  0x09:
      AS608_INFO("???????????????????????\r\n\r\n");
		break;
		
		case  0x0a:
	    AS608_INFO("??????????\r\n");
		break;
		
		case  0x0b:
      AS608_INFO("??????????????????????��\r\n");
		break;
		
		case  0x10:
	    AS608_INFO("?????????\r\n");
		break;
		
		case  0x11:
      AS608_INFO("??????????\r\n");
		break;	
		
		case  0x15:
		  AS608_INFO("?????????????��??????????????\r\n");
		break;
		
		case  0x18:
		  AS608_INFO("??�� FLASH ????\r\n");
		break;
		
		case  0x19:
	    AS608_INFO("��???????\r\n");
		break;
		
		case  0x1a:
	    AS608_INFO("??��???????\r\n");
		break;
		
		case  0x1b:
			AS608_INFO("??????څ???????\r\n");
		break;
		
		case  0x1c:
		  AS608_INFO("???��???????????\r\n");
		break;
		
		case  0x1f:
      AS608_INFO("??????\r\n");
		break;
		
		case  0x20:
      AS608_INFO("???????\r\n");
		break;
		
		default :
      AS608_INFO("??��???????????\r\n");
		break;	
  }

}






/*********************************************END OF FILE**********************/

