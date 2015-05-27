
#include "hal_mcu.h"
#include "hal_uart.h"

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "gatt.h"
#include "simpleGATTprofile.h"
#include "peripheral.h"
#include "gapbondmgr.h"

#include "serialCom.h"
#include "simpleBLEPeripheral.h"
#include "npi.h"

uint16 g_uwIndSendFlag = 1;

 uint8 g_aucSerialComRxBuf[SERCOM_UART_BUF_LEN];
 uint8 SerialComRxBuf[SERCOM_UART_BUF_LEN];
 

int16 g_wSerialComRxCnt = 0;
int16 SerialComRxCnt = 0;
int16 g_wSerialComBleFlag = 0;




uint8 serialCom_TaskID;
uint8 g_ucSerComUartTxCnt = 0;

uint8 WAKE_UP_OK = 0;

uint8 USART_TX_Buf[30] ;


extern uint8 simpleBLEPeripheral_TaskID;
extern gaprole_States_t gapProfileState;


/********dealy****************/
void Delay( uint32 nCount)
{
  while(nCount--)
  {
  }
}

void Uart_Start( void)
{
	NPI_InitTransport( serialCom_Callback );
}

#if (defined (DMA_PM)  &&(FALSE == DMA_PM))
/**************************************************************************************************
 * @fn      PortX Interrupt Handler
 *
 * @brief   This function is the PortX interrupt service routine.
 *
 * @param   None.
 *
 * @return  None.
 *************************************************************************************************/
HAL_ISR_FUNCTION(port0Isr, P0INT_VECTOR)
{
  HAL_ENTER_ISR();

  P0IFG = 0;
  P0IF = 0;

  P0_6 = 0 ;
  WAKE_UP_OK = 1 ;
  

  osal_set_event(serialCom_TaskID, START_UART); 
  
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();

  
}
#endif



/*********************************************************************
 * @fn      serialCom_init
 *
 * @brief   初始化配置串口
 *
 * @param   None
 *
 * @return  none
 */


void serialCom_init( uint8 task_id )
{

    serialCom_TaskID = task_id;

    Uart_Start();

#if (defined (DMA_PM)  && (FALSE == DMA_PM))
    //配置P0.4为输入中断
    P0SEL &= ~(0x1 << 4);
	P0DIR &= ~(0x1 << 4);
    P0IEN |= 0x1 << 4;
    PICTL |= 0x1;  //falling edge

	//P2INP |= 0x1 << 5;
   
    P0IFG = 0;
	P0IF = 0;

	IEN1 |= 0x20;
#endif

}


/*********************************************************************
 * @fn      serialCom_Callback
 *
 * @brief   串口回调函数
 *
 * @param   None
 *
 * @return  none
 */

void serialCom_Callback(uint8 port, uint8 event)
{
    uint16 uwRxCnt;
    uwRxCnt = Hal_UART_RxBufLen(port);
    if (0 < uwRxCnt)
    {
	if (0 == g_wSerialComBleFlag)
	{
	  g_wSerialComBleFlag = 1;
          
          osal_stop_timerEx( serialCom_TaskID , UART_DATELOSE  ); //关闭超时定时器
          
          osal_set_event(serialCom_TaskID, SERCOM_START_DEVICE_EVT);
	}
		
	HalUARTRead (port, g_aucSerialComRxBuf + g_wSerialComRxCnt, uwRxCnt);
        
        g_wSerialComRxCnt = g_wSerialComRxCnt + uwRxCnt;
        uwRxCnt = 0;
    }
}



/*********************************************************************
 * @fn      serialCom_Write
 *
 * @brief   通过串口写数据
 *
 * @param   pBuffer - 待发送数据缓冲区指针
            length - 数据长度
 *
 * @return  none
 */
void serialCom_Write(uint8 *pBuffer, uint16 length)
{
   
    uint8 i =  0 ;

    P0_7 = 0 ;
    g_ucSerComUartTxCnt = length ;
    
    for(i=0;i<g_ucSerComUartTxCnt;i++)
    {
      USART_TX_Buf[i] = *pBuffer;
      pBuffer++;
    }
           
    osal_start_timerEx( serialCom_TaskID, UART_TX_DEALY, 10 );
    
    
      
}

/*********************************************************************
 * @fn      serialCom_ProcessEvent
 *
 * @brief   serialCom任务处理函数
 *
 * @param   task_id - 任务ID号
            events - 事件
 *
 * @return  none
 */
uint16 serialCom_ProcessEvent( uint8 task_id, uint16 events )
{
    static uint32 uwPtr = 0, uwLen = 0, uwRectryCnt = 0;
    static int16 wPreRxCnt1 = 0;
    static int16 wPreRxCnt2 = 0;
    
    attHandleValueInd_t tInd;

    if ( events & SERCOM_INDICATION_EVT)
    { 
        if (gapProfileState == GAPROLE_CONNECTED)
        {
            if ((1 == g_uwIndSendFlag) || (25 <= uwRectryCnt))  //发送成功, 或者超过重传次数
            {                    
                  uwRectryCnt = 0;
                  
                  SerialComRxCnt -= uwLen;
                  uwPtr += uwLen;
                  if(0 < SerialComRxCnt)  
                  {
                      if (20 < SerialComRxCnt)
                      {
                        
                          g_uwIndSendFlag = 0;
                          uwLen = 20;
                          tInd.handle = 0x0028;
                          tInd.len = uwLen;
                          osal_memcpy(tInd.value, SerialComRxBuf + uwPtr, uwLen);
                          GATT_Indication(0, &tInd, FALSE, simpleBLEPeripheral_TaskID);
                          //serialCom_Write("1", 1);
                       
        
                          return (events | SERCOM_INDICATION_EVT);
                      }
                      
                      else 
                      {
                          g_uwIndSendFlag = 0;

                          uwLen = SerialComRxCnt;
                          tInd.handle = 0x0028;
                          tInd.len = uwLen;
                          osal_memcpy(tInd.value, SerialComRxBuf + uwPtr, uwLen);
                          GATT_Indication(0, &tInd, FALSE, simpleBLEPeripheral_TaskID);
                          
                                
                          return (events | SERCOM_INDICATION_EVT);
                        
                      }
                  
                  
                  }
                  else
                  {
                      g_wSerialComBleFlag = 0;
                      SerialComRxCnt = 0;
                      
                      WAKE_UP_OK = 0 ;
                      g_uwIndSendFlag = 1 ;
                      uwRectryCnt = 0 ;
                     
                     return (events ^ SERCOM_INDICATION_EVT);
                                                     
                  }                                                                                                 
                   
         
              }
              else //重传
              {             
                 uwRectryCnt ++ ;
                 osal_start_timerEx( serialCom_TaskID, SERCOM_INDICATION_EVT, 5);
                 //serialCom_Write("2", 1);
                 return (events ^ SERCOM_INDICATION_EVT);

              }
        }
        else
        {
            g_wSerialComBleFlag = 0;
            SerialComRxCnt = 0;

            WAKE_UP_OK = 0 ;
            g_uwIndSendFlag = 1 ;
            uwRectryCnt = 0 ;

            
                                   
        }  
        
     return (events ^ SERCOM_INDICATION_EVT);        
    }
    
    //接收串口数据, 蓝牙转发
    //缓冲作用，等待串口接收数据完全
    if ( events & SERCOM_START_DEVICE_EVT)  
    {
        if (wPreRxCnt1 != g_wSerialComRxCnt)
        {
            wPreRxCnt1 = g_wSerialComRxCnt;
            osal_set_event(serialCom_TaskID, SERCOM_START_DEVICE_EVT);
        }
        else  
        {
          if (wPreRxCnt2 != g_wSerialComRxCnt)
          {
            wPreRxCnt2 = g_wSerialComRxCnt;
            osal_set_event(serialCom_TaskID, SERCOM_START_DEVICE_EVT);
          }
          else
          {
                uwPtr = 0;
                uwLen = 0;
                wPreRxCnt1 = 0;
                wPreRxCnt2 = 0;
                
                SerialComRxCnt = g_wSerialComRxCnt ;
                
                osal_memcpy(SerialComRxBuf, g_aucSerialComRxBuf, SerialComRxCnt);
                
            //    osal_memcpy(g_aucSerialComRxBuf, Serial0, SERCOM_UART_BUF_LEN);                
                g_wSerialComRxCnt = 0 ;  
                g_wSerialComBleFlag = 0;       
                
                
                osal_set_event(serialCom_TaskID, SERCOM_INDICATION_EVT);                                    
                      
          }                      
        }            
        return (events ^ SERCOM_START_DEVICE_EVT);
    }


    //write任务
    if ( events & SERCOM_TX_UART_EVT)
    {
      
        if (0 == g_ucSerComUartTxCnt)
        { 
            P0_7 = 1 ;
            return (events ^ SERCOM_TX_UART_EVT);            
        }
        else
            return (events | SERCOM_TX_UART_EVT);
    }
    
    
    
    if ( events & START_UART)
    {
        
        Uart_Start();
        osal_set_event(simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT);
        osal_start_timerEx( serialCom_TaskID, UART_DATELOSE, 100 );

        return (events ^ START_UART);
    }
    
    if ( events & UART_DATELOSE)
    {
        
       WAKE_UP_OK = 0 ;

        return (events ^ UART_DATELOSE);
    }
    
    if ( events & UART_TX_DEALY)
    {
        
        osal_set_event(serialCom_TaskID, SERCOM_TX_UART_EVT); 
        HalUARTWrite (SERCOM_UART_PORT, USART_TX_Buf, g_ucSerComUartTxCnt);

        return (events ^ UART_TX_DEALY);
    }
    
    return 0;
}


