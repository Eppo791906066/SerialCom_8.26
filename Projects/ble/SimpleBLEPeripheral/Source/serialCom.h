

#ifndef _SERIAL_COM_H
#define _SERIAL_COM_H

//起始符标志
#define SERCOM_START_SYMBOL          0xA5A5

#define SERCOM_UART_PORT              HAL_UART_PORT_0

#define SERCOM_UART_BUF_LEN           256

#define SERCOM_BLE_BUF_DIM            32

#define SC_UART_FC                    FALSE
#define SC_UART_FC_THRESHOLD          20
#define SC_UART_RX_BUF_SIZE           128
#define SC_UART_TX_BUF_SIZE           128
#define SC_UART_IDLE_TIMEOUT          6
#define SC_UART_INT_ENABLE            TRUE
#define SC_UART_BR                    HAL_UART_BR_115200


// UxCSR - USART Control and Status Register.
#define SERCOM_CSR_MODE                   0x80
#define SERCOM_CSR_RE                     0x40
#define SERCOM_CSR_SLAVE                  0x20
#define SERCOM_CSR_FE                     0x10
#define SERCOM_CSR_ERR                    0x08
#define SERCOM_CSR_RX_BYTE                0x04
#define SERCOM_CSR_TX_BYTE                0x02
#define SERCOM_CSR_ACTIVE                 0x01

// UxUCR - USART UART Control Register.
#define SERCOM_UCR_FLUSH                  0x80
#define SERCOM_UCR_FLOW                   0x40
#define SERCOM_UCR_D9                     0x20
#define SERCOM_UCR_BIT9                   0x10
#define SERCOM_UCR_PARITY                 0x08
#define SERCOM_UCR_SPB                    0x04
#define SERCOM_UCR_STOP                   0x02
#define SERCOM_UCR_START                  0x01

#define UTX0IE                     0x04
#define UTX1IE                     0x08

#define SERCOM_P2DIR_PRIPO                0xC0

#define SERCOM_UART0_PRIPO                0x00     //UART0 over UART1
#define SERCOM_UART1_PRIPO                0x40     //UART1 over UART0

// Incompatible redefinitions result with more than one UART driver sub-module.
#if (SERCOM_UART_PORT == 0)
#define PxOUT                      P0
#define PxIN                       P0
#define PxDIR                      P0DIR
#define PxSEL                      P0SEL
#define UxCSR                      U0CSR
#define UxUCR                      U0UCR
#define UxDBUF                     U0DBUF
#define UxBAUD                     U0BAUD
#define UxGCR                      U0GCR
#define URXxIE                     URX0IE
#define URXxIF                     URX0IF
#define UTXxIE                     UTX0IE
#define UTXxIF                     UTX0IF

#define PxIFG                      P0IFG
#define PxIF                       P0IF
#define PxIEN                      P0IEN
#define PICTL_BIT                  0x01
#define IENx                       IEN2
#define IEN_BIT                    0x20

#else
#define PxOUT                      P1
#define PxIN                       P1
#define PxDIR                      P1DIR
#define PxSEL                      P1SEL
#define UxCSR                      U1CSR
#define UxUCR                      U1UCR
#define UxDBUF                     U1DBUF
#define UxBAUD                     U1BAUD
#define UxGCR                      U1GCR
#define URXxIE                     URX1IE
#define URXxIF                     URX1IF
#define UTXxIE                     UTX1IE
#define UTXxIF                     UTX1IF

#define PxIFG                      P1IFG
#define PxIF                       P1IF
#define PxIEN                      P1IEN
#define PICTL_BIT                  0x04
#define IENx                       IEN2
#define IEN_BIT                    0x10
#endif

#if (SERCOM_UART_PORT == 0)
#define SERCOM_UART_PERCFG_BIT        0x01         // USART0 on P0, Alt-1; so clear this bit.
#define SERCOM_UART_Px_RX_TX          0x0C         // Peripheral I/O Select for Rx/Tx.
#define SERCOM_UART_Px_RX             0x04         // Peripheral I/O Select for Rx.
#define SERCOM_UART_Px_RTS            0x20         // Peripheral I/O Select for RTS.
#define SERCOM_UART_Px_CTS            0x10         // Peripheral I/O Select for CTS.
#define SERCOM_UART_PRIPO             0x00         // USART0 priority over UART1.

#else
#define SERCOM_UART_PERCFG_BIT        0x02         // USART1 on P1, Alt-2; so set this bit.
#define SERCOM_UART_Px_RTS            0x20         // Peripheral I/O Select for RTS.
#define SERCOM_UART_Px_CTS            0x10         // Peripheral I/O Select for CTS.
#define SERCOM_UART_Px_RX_TX          0xC0         // Peripheral I/O Select for Rx/Tx.
#define SERCOM_UART_Px_RX             0x80         // Peripheral I/O Select for Rx.
#define SERCOM_UART_PRIPO             0x40         // USART1 priority over UART0.
#endif


// serial Com Task Events
#define SERCOM_START_DEVICE_EVT       0x0001
#define SERCOM_IDLE_EVT               0x0002
#define SERCOM_CTL_UART_EVT           0x0004
#define SERCOM_INDICATION_EVT         0x0008
#define SERCOM_TX_UART_EVT            0x0010
#define START_UART                    0x0020
#define UART_DATELOSE                 0x0040
#define UART_TX_DEALY                 0x0080

#define SERCOM_PERIODIC_EVT_PERIOD    1   //ms
#define SERCOM_IDLE_EVT_PERIOD        20  //ms
#define SERCOM_CTL_UART_PERIOD        10  //ms


extern void serialCom_Callback(uint8 port, uint8 event);
extern void serialCom_init( uint8 task_id );
extern uint8 serialCom_sendNoti(uint8 *pBuffer,uint16 length);
extern void serialCom_Write(uint8 *pBuffer, uint16 length);
extern uint16 serialCom_ProcessEvent( uint8 task_id, uint16 events );

#endif //end of serialCom.h

