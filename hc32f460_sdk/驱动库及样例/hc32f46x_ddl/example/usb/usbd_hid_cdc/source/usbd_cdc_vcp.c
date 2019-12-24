/******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co.,Ltd. All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co.,Ltd ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 */
/******************************************************************************/
/** \file usbd_cdc_vcp.c
 **
 ** \brief  Generic media access Layer.
 **
 **   - 2019-6-3  1.0  zhangxl First version for USB CDC VCP demo.
 **
 ******************************************************************************/
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "usbd_cdc_vcp.h"
#include "usb_conf.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
LINE_CODING linecoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/* These are external variables imported from CDC core to be used for IN
   transfer management. */
extern uint8_t  APP_Rx_Buffer [APP_RX_DATA_SIZE]; /* Write CDC received data in this buffer.
                                     These data will be sent over USB IN endpoint
                                     in the CDC core functions. */
extern uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static uint16_t VCP_Init     (void);
static uint16_t VCP_DeInit   (void);
static uint16_t VCP_Ctrl     (uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataTx   (uint32_t Len);
static uint16_t VCP_DataRx   (uint8_t* Buf, uint32_t Len);
static uint16_t VCP_COMConfig(void);  /* MISRAC 2004*/

static void UsartErrIrqCallback(void);
static void UsartRxIrqCallback(void);

CDC_IF_Prop_TypeDef VCP_fops =
{
    &VCP_Init,
    &VCP_DeInit,
    &VCP_Ctrl,
    &VCP_DataTx,
    &VCP_DataRx
};

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
  * @brief  VCP_Init
  *         Initializes the Media
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{
    stc_usart_uart_init_t stcInitCfg;
    stc_irq_regi_conf_t stcIrqRegiCfg;

    MEM_ZERO_STRUCT(stcInitCfg);
    MEM_ZERO_STRUCT(stcIrqRegiCfg);

    /* PC13 --> RX, PH02 --> TX for full-duplex */
    PORT_SetFunc(PortC, Pin13, Func_Usart4_Rx, Disable); //RX
    PORT_SetFunc(PortH, Pin02, Func_Usart4_Tx, Disable); //TX
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART4, Enable);
    stcInitCfg.enClkMode        = UsartIntClkCkNoOutput;
    stcInitCfg.enClkDiv         = UsartClkDiv_1;
    stcInitCfg.enDataLength     = UsartDataBits8;
    stcInitCfg.enDirection      = UsartDataLsbFirst;
    stcInitCfg.enStopBit        = UsartOneStopBit;
    stcInitCfg.enParity         = UsartParityNone;
    stcInitCfg.enSampleMode     = UsartSamleBit16;
    stcInitCfg.enDetectMode     = UsartStartBitFallEdge;
    stcInitCfg.enHwFlow         = UsartRtsEnable;
    USART_UART_Init(CDC_COMM, &stcInitCfg);
    if(Ok == USART_SetBaudrate(CDC_COMM, 500000ul))
    {
        /* Set USART RX IRQ */
        stcIrqRegiCfg.enIRQn = Int000_IRQn;
        stcIrqRegiCfg.pfnCallback = &UsartRxIrqCallback;
        stcIrqRegiCfg.enIntSrc = INT_USART4_RI;
        enIrqRegistration(&stcIrqRegiCfg);
        NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
        NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
        NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
        /* Set USART RX error IRQ */
        stcIrqRegiCfg.enIRQn = Int001_IRQn;
        stcIrqRegiCfg.pfnCallback = &UsartErrIrqCallback;
        stcIrqRegiCfg.enIntSrc = INT_USART4_EI;
        enIrqRegistration(&stcIrqRegiCfg);
        NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
        NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
        NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

        USART_FuncCmd(CDC_COMM, UsartTx, Enable);
        USART_FuncCmd(CDC_COMM, UsartRx, Enable);
        USART_FuncCmd(CDC_COMM, UsartRxInt, Enable);
    }
    else
    {
        return USBD_FAIL;
    }
    return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{
    return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl (uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
    switch (Cmd)
    {
        case SEND_ENCAPSULATED_COMMAND:
        /* Not  needed for this driver */
        break;

        case GET_ENCAPSULATED_RESPONSE:
        /* Not  needed for this driver */
        break;

        case SET_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

        case GET_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

        case CLEAR_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

        case SET_LINE_CODING:
            linecoding.bitrate = ((uint32_t)Buf[0] | ((uint32_t)Buf[1] << 8u) | ((uint32_t)Buf[2] << 16u) | ((uint32_t)Buf[3] << 24u));
            linecoding.format = Buf[4];
            linecoding.paritytype = Buf[5];
            linecoding.datatype = Buf[6];
            /* Set the new configuration */
            VCP_COMConfig();  /* MISRAC 2004*/
        break;
        case GET_LINE_CODING:
            Buf[0] = (uint8_t)(linecoding.bitrate);
            Buf[1] = (uint8_t)(linecoding.bitrate >> 8u);
            Buf[2] = (uint8_t)(linecoding.bitrate >> 16u);
            Buf[3] = (uint8_t)(linecoding.bitrate >> 24u);
            Buf[4] = linecoding.format;
            Buf[5] = linecoding.paritytype;
            Buf[6] = linecoding.datatype;
        break;
        case SET_CONTROL_LINE_STATE:
        /* Not  needed for this driver */
        break;
        case SEND_BREAK:
        /* Not  needed for this driver */
        break;
        default:
        break;
    }
    return USBD_OK;
}

/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in
  *         this function.
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataTx (uint32_t Len)
{
    if (linecoding.datatype == 7u)
    {
        APP_Rx_Buffer[APP_Rx_ptr_in] = (uint8_t)USART_RecData(CDC_COMM) & 0x7Fu;
    }
    else if (linecoding.datatype == 8u)
    {
        APP_Rx_Buffer[APP_Rx_ptr_in] = (uint8_t)USART_RecData(CDC_COMM);
    }
    else
    {
        //
    }

    APP_Rx_ptr_in++;

    /* To avoid buffer overflow */
    if(APP_Rx_ptr_in == APP_RX_DATA_SIZE)
    {
        APP_Rx_ptr_in = 0u;
    }

    return USBD_OK;
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataRx (uint8_t* Buf, uint32_t Len)
{
    uint32_t i;

    for (i = 0ul; i < Len; i++)
    {
        while(Set != USART_GetStatus(CDC_COMM, UsartTxEmpty))
        {
            ;
        }
        USART_SendData(CDC_COMM, (uint16_t)*(Buf + i));
    }
    return USBD_OK;
}

uint16_t VCP_COMConfigDefault(void)
{
    stc_usart_uart_init_t stcInitCfg;
    uint16_t u16Res = USBD_OK;
    uint8_t u8Cnt;
    MEM_ZERO_STRUCT(stcInitCfg);

    stcInitCfg.enClkMode        = UsartIntClkCkNoOutput;
    stcInitCfg.enClkDiv         = UsartClkDiv_1;
    stcInitCfg.enDataLength     = UsartDataBits8;
    stcInitCfg.enDirection      = UsartDataLsbFirst;
    stcInitCfg.enStopBit        = UsartOneStopBit;
    stcInitCfg.enParity         = UsartParityNone;
    stcInitCfg.enSampleMode     = UsartSamleBit16;
    stcInitCfg.enDetectMode     = UsartStartBitFallEdge;
    stcInitCfg.enHwFlow         = UsartRtsEnable;
    USART_UART_Init(CDC_COMM, &stcInitCfg);
    for (u8Cnt=0u; u8Cnt < 4u; u8Cnt++)
    {
        if(Ok == USART_SetBaudrate(CDC_COMM, 500000ul))
        {
            USART_FuncCmd(CDC_COMM, UsartTx, Enable);
            USART_FuncCmd(CDC_COMM, UsartRx, Enable);
            USART_FuncCmd(CDC_COMM, UsartRxInt, Enable);
            break;
        }
        else
        {
            USART_SetClockDiv(CDC_COMM, (en_usart_clk_div_t)u8Cnt);
        }
    }
    if (u8Cnt == 4u)
    {
        u16Res = USBD_FAIL;
    }
    return u16Res;
}
/**
  * @brief  VCP_COMConfig
  *         Configure the COM Port with default values or values received from host.
  * @param  None  MISRAC 2004
  *
  * @retval None.
  */
static uint16_t VCP_COMConfig(void)
{
    stc_usart_uart_init_t stcInitCfg;
    uint8_t u8Cnt;
    MEM_ZERO_STRUCT(stcInitCfg);
    uint16_t u16Res = USBD_OK;
    uint8_t u8ReturnFlag = 0u;

    /* set the Stop bit*/
    switch (linecoding.format)
    {
        case 0u:
            stcInitCfg.enStopBit = UsartOneStopBit;
            break;
        case 1u:
            stcInitCfg.enStopBit = UsartOneStopBit;
            break;
        case 2u:
            stcInitCfg.enStopBit = UsartTwoStopBit;
            break;
        default :
            VCP_COMConfigDefault();
            u16Res = USBD_FAIL;
            u8ReturnFlag = 1u;
            break;
    }

    if(1u != u8ReturnFlag)
    {
        /* set the parity bit*/
        switch (linecoding.paritytype)
        {
            case 0u:
                stcInitCfg.enParity = UsartParityNone;
                break;
            case 1u:
                stcInitCfg.enParity = UsartParityEven;
                break;
            case 2u:
                stcInitCfg.enParity = UsartParityOdd;
                break;
            default :
                VCP_COMConfigDefault();
                u16Res = USBD_FAIL;
                u8ReturnFlag = 1u;
                break;
        }

        if(1u != u8ReturnFlag)
        {
            /*set the data type : only 8bits and 9bits is supported */
            switch (linecoding.datatype)
            {
                case 0x07u:
                    /* With this configuration a parity (Even or Odd) should be set */
                    stcInitCfg.enDataLength = UsartDataBits8;
                    break;
                case 0x08u:
                    if (stcInitCfg.enParity == UsartParityNone)
                    {
                        stcInitCfg.enDataLength = UsartDataBits8;
                    }
                    else
                    {
                        stcInitCfg.enDataLength = UsartDataBits9;
                    }
                    break;
                default :
                    VCP_COMConfigDefault();
                    u16Res = USBD_FAIL;
                    u8ReturnFlag = 1u;
                    break;
            }

            if(1u != u8ReturnFlag)
            {
                /* Configure and enable the USART */
                stcInitCfg.enHwFlow = UsartRtsEnable;
                USART_UART_Init(CDC_COMM, &stcInitCfg);

                for (u8Cnt=0u; u8Cnt < 4u; u8Cnt++)
                {
                    if(Ok == USART_SetBaudrate(CDC_COMM, 500000ul))
                    {
                        USART_FuncCmd(CDC_COMM, UsartTx, Enable);
                        USART_FuncCmd(CDC_COMM, UsartRx, Enable);
                        USART_FuncCmd(CDC_COMM, UsartRxInt, Enable);
                        break;
                    }
                    else
                    {
                        USART_SetClockDiv(CDC_COMM, (en_usart_clk_div_t)u8Cnt);
                    }
                }
                if (u8Cnt == 4u)
                {
                    u16Res = USBD_FAIL;
                }
            }
        }
    }
    return u16Res;
}

/**
 *******************************************************************************
 ** \brief USART RX irq callback function.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void UsartRxIrqCallback(void)
{
    if (Set == USART_GetStatus(CDC_COMM, UsartRxNoEmpty))
    {
        /* Send the received data to the PC Host*/
        VCP_DataTx (0u);
    }
}

/**
 *******************************************************************************
 ** \brief USART RX error irq callback function.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void UsartErrIrqCallback(void)
{
    if (Set == USART_GetStatus(CDC_COMM, UsartFrameErr))
    {
        USART_ClearStatus(CDC_COMM, UsartFrameErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(CDC_COMM, UsartParityErr))
    {
        USART_ClearStatus(CDC_COMM, UsartParityErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(CDC_COMM, UsartOverrunErr))
    {
        USART_ClearStatus(CDC_COMM, UsartOverrunErr);
    }
    else
    {
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
