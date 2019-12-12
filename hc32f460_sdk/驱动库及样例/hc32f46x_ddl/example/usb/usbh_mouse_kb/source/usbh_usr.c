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
/** \file usbh_usr.c
 **
 ** A detailed description is available at
 ** @link
        This file includes the user application layer.
    @endlink
 **
 **   - 2019-12-10  1.0  zhangxl First version for USB demo.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "usbh_usr.h"
#include "usbh_hid_mouse.h"
#include "usbh_hid_keybd.h"
#include "hc32_ddl.h"

/** @addtogroup USBH_USER
* @{
*/

/** @addtogroup USBH_HID_DEMO_USER_CALLBACKS
* @{
*/

/** @defgroup USBH_USR
* @brief This file is the Header file for usbh_usr.c
* @{
*/


/** @defgroup USBH_CORE_Exported_TypesDefinitions
* @{
*/
#define KYBRD_FIRST_COLUMN               (uint16_t)319
#define KYBRD_LAST_COLUMN                (uint16_t)7
#define KYBRD_FIRST_LINE                 (uint8_t)120
#define KYBRD_LAST_LINE                  (uint8_t)200


/**
* @}
*/
uint8_t  KeybrdCharXpos           = 0;
uint16_t KeybrdCharYpos           = 0;
extern  int16_t  x_loc, y_loc;
extern __IO int16_t  prev_x, prev_y;

/** @addtogroup USER
* @{
*/

/** @defgroup USBH_USR
* @brief    This file includes the user application layer
* @{
*/

/** @defgroup USBH_CORE_Exported_Types
* @{
*/



/** @defgroup USBH_USR_Private_Defines
* @{
*/
/**
* @}
*/


/** @defgroup USBH_USR_Private_Macros
* @{
*/
/**
* @}
*/

/** @defgroup USBH_USR_Private_Variables
* @{
*/
extern USB_OTG_CORE_HANDLE           USB_OTG_Core_dev;
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USR_cb =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  NULL,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};

/**
* @}
*/

/** @defgroup USBH_USR_Private_Constants
* @{
*/
/*--------------- LCD Messages ---------------*/
const uint8_t MSG_HOST_INIT[]          = "> Host Library Initialized\n";
const uint8_t MSG_DEV_ATTACHED[]       = "> Device Attached\n";
const uint8_t MSG_DEV_DISCONNECTED[]   = "> Device Disconnected\n";
const uint8_t MSG_DEV_ENUMERATED[]     = "> Enumeration completed\n";
const uint8_t MSG_DEV_HIGHSPEED[]      = "> High speed device detected\n";
const uint8_t MSG_DEV_FULLSPEED[]      = "> Full speed device detected\n";
const uint8_t MSG_DEV_LOWSPEED[]       = "> Low speed device detected\n";
const uint8_t MSG_DEV_ERROR[]          = "> Device fault \n";

const uint8_t MSG_MSC_CLASS[]          = "> Mass storage device connected\n";
const uint8_t MSG_HID_CLASS[]          = "> HID device connected\n";

const uint8_t USB_HID_MouseStatus[]    = "> HID Demo Device : Mouse\n";
const uint8_t USB_HID_KeybrdStatus[]   = "> HID Demo Device : Keyboard\n";
const uint8_t MSG_UNREC_ERROR[]        = "> UNRECOVERED ERROR STATE\n";
/**
* @}
*/



/** @defgroup USBH_USR_Private_FunctionPrototypes
* @{
*/
/**
* @}
*/


/** @defgroup USBH_USR_Private_Functions
* @{
*/





/**
* @brief  USBH_USR_Init
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBH_USR_Init(void)
{
  static uint8_t startup = 0;
//  RCC_ClocksTypeDef SYS_Clocks;
  if(startup == 0 )
  {
//	STM_EVAL_LEDInit(LED1);
//    STM_EVAL_LEDInit(LED2);
//    STM_EVAL_LEDInit(LED3);
//    STM_EVAL_LEDInit(LED4);
//
//    startup = 1;
//
//    STM_EVAL_PBInit(BUTTON_KEY, BUTTON_MODE_GPIO);
//
//#if defined (USE_STM324xG_EVAL)
//  STM324xG_LCD_Init();
//#else
// #error "Missing define: Evaluation board (ie. USE_STM322xG_EVAL)"
//#endif
//
//    LCD_LOG_Init();
//
//#ifdef USE_USB_OTG_HS
//    LCD_LOG_SetHeader(" USB OTG HS HID Host");
//#else
//    LCD_LOG_SetHeader(" USB OTG FS HID Host");
//#endif
///**/
//	RCC_GetClocksFreq(&SYS_Clocks);
//	LCD_UsrLog("> SYSCLK : %dM\n",(char ) (SYS_Clocks.SYSCLK_Frequency/1000000));
//	LCD_UsrLog("> HCLK : %dM\n", (char )(SYS_Clocks.HCLK_Frequency/1000000));
//	LCD_UsrLog("> PCLK1 : %dM\n",(char ) (SYS_Clocks.PCLK1_Frequency/1000000));
//	LCD_UsrLog("> PCLK2 : %dM\n", (char )(SYS_Clocks.PCLK2_Frequency/1000000));
///**/
//	LCD_UsrLog("> USB Host library started.\n");
//    LCD_LOG_SetFooter ("     USB Host Library v2.1.0" );
  }
}

/**
* @brief  USBH_USR_DeviceAttached
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBH_USR_DeviceAttached(void)
{
//  LCD_UsrLog ((void*)MSG_DEV_ATTACHED);
}

/**
* @brief  USBH_USR_UnrecoveredError
* @param  None
* @retval None
*/
void USBH_USR_UnrecoveredError (void)
{
//  LCD_ErrLog((void*)MSG_UNREC_ERROR);
}

/**
* @brief  USBH_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval None
*/
void USBH_USR_DeviceDisconnected (void)
{

//  LCD_SetBackColor(Black);
//  LCD_SetTextColor(Black);
//
//  LCD_LOG_ClearTextZone();
//
//  LCD_DrawRect(MOUSE_WINDOW_X,
//               MOUSE_WINDOW_Y,
//               MOUSE_WINDOW_HEIGHT,
//               MOUSE_WINDOW_WIDTH);
//  LCD_SetTextColor(White);
//
//  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "                                   ");
//  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "                                   ");
//
//  LCD_ErrLog((void *)MSG_DEV_DISCONNECTED);
}

/**
* @brief  USBH_USR_ResetUSBDevice
*         Reset USB Device
* @param  None
* @retval None
*/
void USBH_USR_ResetDevice(void)
{
  /* Users can do their application actions here for the USB-Reset */
}


/**
* @brief  USBH_USR_DeviceSpeedDetected
*         Displays the message on LCD for device speed
* @param  Devicespeed : Device Speed
* @retval None
*/
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
//  if(DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED)
//  {
//    LCD_UsrLog((void *)MSG_DEV_HIGHSPEED);
//  }
//  else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
//  {
//    LCD_UsrLog((void *)MSG_DEV_FULLSPEED);
//  }
//  else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
//  {
//    LCD_UsrLog((void *)MSG_DEV_LOWSPEED);
//  }
//  else
//  {
//    LCD_UsrLog((void *)MSG_DEV_ERROR);
//  }
}

/**
* @brief  USBH_USR_Device_DescAvailable
*         Displays the message on LCD for device descriptor
* @param  DeviceDesc : device descriptor
* @retval None
*/
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
//  uint8_t temp[50];
//  USBH_DevDesc_TypeDef *hs;
//  hs = DeviceDesc;
//
//
//  sprintf((char *)temp , "VID : %04Xh\n" , (uint32_t)(*hs).idVendor);
//  LCD_UsrLog((void *)temp);
//
//
//  sprintf((char *)temp , "PID : %04Xh\n" , (uint32_t)(*hs).idProduct);
//  LCD_UsrLog((void *)temp);
}

/**
* @brief  USBH_USR_DeviceAddressAssigned
*         USB device is successfully assigned the Address
* @param  None
* @retval None
*/
void USBH_USR_DeviceAddressAssigned(void)
{

}


/**
* @brief  USBH_USR_Conf_Desc
*         Displays the message on LCD for configuration descriptor
* @param  ConfDesc : Configuration descriptor
* @retval None
*/
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
    USBH_InterfaceDesc_TypeDef *id;

    id = itfDesc;

    if((*id).bInterfaceClass  == 0x08)
    {
        printf("%s", MSG_MSC_CLASS);
    }
    else if((*id).bInterfaceClass  == 0x03)
    {
        printf("%s", MSG_HID_CLASS);
    }
}

/**
* @brief  USBH_USR_Manufacturer_String
*         Displays the message on LCD for Manufacturer String
* @param  ManufacturerString : Manufacturer String of Device
* @retval None
*/
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
//  char temp[100];
//  sprintf(temp, "Manufacturer : %s\n", (char *)ManufacturerString);
//  LCD_UsrLog((void *)temp);

}

/**
* @brief  USBH_USR_Product_String
*         Displays the message on LCD for Product String
* @param  ProductString : Product String of Device
* @retval None
*/
void USBH_USR_Product_String(void *ProductString)
{
//  char temp[100];
//  sprintf((char *)temp, "Product : %s\n", (char *)ProductString);
//  LCD_UsrLog((void *)temp);

}

/**
* @brief  USBH_USR_SerialNum_String
*         Displays the message on LCD for SerialNum_String
* @param  SerialNumString : SerialNum_String of device
* @retval None
*/
void USBH_USR_SerialNum_String(void *SerialNumString)
{
//  uint8_t temp[100];
//  sprintf((char *)temp, "Serial Number : %s\n", (char *)SerialNumString);
//  LCD_UsrLog((void *)temp);
}

/**
* @brief  EnumerationDone
*         User response request is displayed to ask for
*         application jump to class
* @param  None
* @retval None
*/
void USBH_USR_EnumerationDone(void)
{
//  /* Enumeration complete */
//  LCD_UsrLog((void *)MSG_DEV_ENUMERATED);
//  LCD_SetTextColor(Green);
//  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 45, "To start the HID class operations: " );
//  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "Press USER KEY....." );
//  LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR);
}

/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void)
{
    printf("> Device not supported.\n");

}


/**
* @brief  USBH_USR_UserInput
*         User Action for application state entry
* @param  None
* @retval USBH_USR_Status : User response for key button
*/
USBH_USR_Status USBH_USR_UserInput(void)
{

  USBH_USR_Status usbh_usr_status;

  usbh_usr_status = USBH_USR_NO_RESP;

  /*Key B3 is in polling mode to detect user action */
//  if(STM_EVAL_PBGetState(Button_KEY) == RESET)
//  {
//
//    usbh_usr_status = USBH_USR_RESP_OK;
//
//  }


  return usbh_usr_status;

}

/**
* @brief  USBH_USR_OverCurrentDetected
*         Device Overcurrent detection event
* @param  None
* @retval None
*/
void USBH_USR_OverCurrentDetected (void)
{
//   LCD_ErrLog ("Overcurrent detected.\n");

}

/**
* @brief  USR_MOUSE_Init
*         Init Mouse window
* @param  None
* @retval None
*/
void USR_MOUSE_Init	(void)
{
    HID_MOUSE_ButtonReleased(0);
    HID_MOUSE_ButtonReleased(1);
    HID_MOUSE_ButtonReleased(2);
    x_loc  = 0;
    y_loc  = 0;
    prev_x = 0;
    prev_y = 0;
}

/**
* @brief  USR_MOUSE_ProcessData
*         Process Mouse data
* @param  data : Mouse data to be displayed
* @retval None
*/
void USR_MOUSE_ProcessData(HID_MOUSE_Data_TypeDef *data)
{

    uint8_t idx = 1u;
    static uint8_t b_state[3u] = { 0u, 0u , 0u};

    if ((data->x != 0u) && (data->y != 0u))
    {
        HID_MOUSE_UpdatePosition(data->x , data->y);
    }

    for ( idx = 0u ; idx < 3u ; idx ++)
    {

        if(data->button & (0x01u << idx))
        {
            if(b_state[idx] == 0u)
            {
                HID_MOUSE_ButtonPressed (idx);
                b_state[idx] = 1u;
            }
        }
        else
        {
            if(b_state[idx] == 1u)
            {
                HID_MOUSE_ButtonReleased (idx);
                b_state[idx] = 0u;
            }
        }
    }
}

/**
* @brief  USR_KEYBRD_Init
*         Init Keyboard window
* @param  None
* @retval None
*/
void  USR_KEYBRD_Init (void)
{

//  LCD_UsrLog((void*)USB_HID_KeybrdStatus);
//  LCD_UsrLog("> Use Keyboard to tape characters: \n\n");
//  LCD_UsrLog("\n\n\n\n\n\n");
//  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "                                   ");
//  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "                                   ");
//
//  LCD_SetTextColor(Green);


    KeybrdCharXpos = KYBRD_FIRST_LINE;
    KeybrdCharYpos = KYBRD_FIRST_COLUMN;
}


/**
* @brief  USR_KEYBRD_ProcessData
*         Process Keyboard data
* @param  data : Keyboard data to be displayed
* @retval None
*/
void  USR_KEYBRD_ProcessData (uint8_t data)
{


//  if(data == '\n')
//  {
//    KeybrdCharYpos = KYBRD_FIRST_COLUMN;
//
//    /*Increment char X position*/
//    KeybrdCharXpos+=SMALL_FONT_LINE_WIDTH;
//
//  }
//  else if (data == '\r')
//  {
//    /* Manage deletion of charactter and upadte cursor location*/
//    if( KeybrdCharYpos == KYBRD_FIRST_COLUMN)
//    {
//      /*First character of first line to be deleted*/
//      if(KeybrdCharXpos == KYBRD_FIRST_LINE)
//      {
//        KeybrdCharYpos =KYBRD_FIRST_COLUMN;
//      }
//      else
//      {
//        KeybrdCharXpos-=SMALL_FONT_LINE_WIDTH;
//        KeybrdCharYpos =(KYBRD_LAST_COLUMN+SMALL_FONT_COLUMN_WIDTH);
//      }
//    }
//    else
//    {
//      KeybrdCharYpos +=SMALL_FONT_COLUMN_WIDTH;
//
//    }
////    LCD_DisplayChar(KeybrdCharXpos,KeybrdCharYpos, ' ');
//  }
//  else
//  {
////    LCD_DisplayChar(KeybrdCharXpos,KeybrdCharYpos, data);
//    /* Update the cursor position on LCD */
//
//    /*Increment char Y position*/
//    KeybrdCharYpos -=SMALL_FONT_COLUMN_WIDTH;
//
//    /*Check if the Y position has reached the last column*/
//    if(KeybrdCharYpos == KYBRD_LAST_COLUMN)
//    {
//      KeybrdCharYpos = KYBRD_FIRST_COLUMN;
//
//      /*Increment char X position*/
//      KeybrdCharXpos+=SMALL_FONT_LINE_WIDTH;
//
//    }
//  }
    printf("%c", (char)data);
}

/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/
void USBH_USR_DeInit(void)
{
}


/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
