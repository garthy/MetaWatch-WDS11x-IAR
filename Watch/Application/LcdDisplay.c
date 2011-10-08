//==============================================================================
//  Copyright 2011 Meta Watch Ltd. - http://www.MetaWatch.org/
// 
//  Licensed under the Meta Watch License, Version 1.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.MetaWatch.org/licenses/license-1.0.html
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//==============================================================================

/******************************************************************************/
/*! \file LcdDisplay.c
 *
 */
/******************************************************************************/

#include "FreeRTOS.h"           
#include "task.h"               
#include "queue.h"              

#include "Messages.h"
#include "BufferPool.h"         

#include "hal_board_type.h"
#include "hal_rtc.h"
#include "hal_battery.h"
#include "hal_lcd.h"
#include "hal_lpm.h"
       
#include "DebugUart.h"      
#include "Messages.h"
#include "Utilities.h"
#include "LcdTask.h"
#include "SerialProfile.h"
#include "MessageQueues.h"
#include "SerialRam.h"
#include "OneSecondTimers.h"
#include "Adc.h"
#include "Buttons.h"
#include "Statistics.h"
#include "OSAL_Nv.h"
#include "Background.h"
#include "NvIds.h"
#include "Icons.h"
#include "Fonts.h"
#include "Display.h"
#include "LcdDisplay.h"
#include "LcdBuffer.h"
#include "Bluetooth.h"
#include "LinkAlarm.h"
#include "Menu.h"
#include "MenuApp.h"
#include "Menus.h"
#include "IdlePage.h"
#include "IdlePageMain.h"
#include "IdlePageQrCode.h"
#include "IdlePageWatchStatus.h"
#include "IdlePageGameOfLife.h"
#include "IdlePageListPairedDevices.h"


#define DISPLAY_TASK_QUEUE_LENGTH 8
#define DISPLAY_TASK_STACK_DEPTH	(configMINIMAL_STACK_DEPTH + 90)    
#define DISPLAY_TASK_PRIORITY     (tskIDLE_PRIORITY + 1)
extern const unsigned char pMetaWatchSplash[NUM_LCD_ROWS*NUM_LCD_COL_BYTES];
xTaskHandle DisplayHandle;

static void DisplayTask(void *pvParameters);
static void DisplayQueueMessageHandler(tHostMsg* pMsg);
void SendMyBufferToLcd(unsigned char TotalRows);

static tHostMsg* pDisplayMsg;
static tTimerId IdleModeTimerId;
static tTimerId ApplicationModeTimerId;
static tTimerId NotificationModeTimerId;

/* Message handlers */

static void ChangeModeHandler(tHostMsg* pMsg);
static void ModeTimeoutHandler(tHostMsg* pMsg);
static void ConfigureIdleBuferSizeHandler(tHostMsg* pMsg);
static void ModifyTimeHandler(tHostMsg* pMsg);
static void MenuModeHandler(unsigned char MsgOptions);
static void ConnectionStateChangeHandler(void);

/******************************************************************************/
void DrawIdleScreen(void);
void DrawSimpleIdleScreen(void);
static void DisplayStartupScreen(void);
static void SetupSplashScreenTimeout(void);
static void AllocateDisplayTimers(void);

static void MenuButtonHandler(unsigned char MsgOptions);

/* the internal buffer */
#define STARTING_ROW                  ( 0 )
#define WATCH_DRAWN_IDLE_BUFFER_ROWS  ( 30 )
#define PHONE_IDLE_BUFFER_ROWS        ( 66 )

 tLcdLine pMyBuffer[NUM_LCD_ROWS];

/******************************************************************************/

static unsigned char nvIdleBufferConfig;
static unsigned char nvIdleBufferInvert;

static void InitialiazeIdleBufferConfig(void);
static void InitializeIdleBufferInvert(void);

void SaveIdleBufferInvert(void);


/******************************************************************************/
#if 0
typedef enum
{
  ReservedPage,
  NormalPage,
  /* the next three are only used on power-up */
  RadioOnWithPairingInfoPage,
  RadioOnWithoutPairingInfoPage,
  BluetoothOffPage,
  MenuPage,
  ListPairedDevicesPage,
  WatchStatusPage,
  QrCodePage,
  
} etIdlePageMode;

static etIdlePageMode CurrentIdlePage;
static etIdlePageMode LastIdlePage = ReservedPage;
#endif
static unsigned char AllowConnectionStateChangeToUpdateScreen;
static void DefaultApplicationAndNotificationButtonConfiguration(void);

/******************************************************************************/

static unsigned char LastMode = IDLE_MODE;
static unsigned char CurrentMode = IDLE_MODE;

//static unsigned char ReturnToApplicationMode;

/******************************************************************************/



/******************************************************************************/
#ifdef FONT_TESTING


static void WriteFontCharacter(unsigned char Character);
static void WriteFontString(unsigned char* pString);
#endif

/******************************************************************************/

/*! Initialize the LCD display task
 *
 * Initializes the display driver, clears the display buffer and starts the
 * display task
 *
 * \return none, result is to start the display task
 */
void InitializeDisplayTask(void)
{
  InitMyBuffer();
  menus_init();

  QueueHandles[DISPLAY_QINDEX] = 
    xQueueCreate( DISPLAY_TASK_QUEUE_LENGTH, MESSAGE_QUEUE_ITEM_SIZE  );
  
  // task function, task name, stack len , task params, priority, task handle
  xTaskCreate(DisplayTask, 
              "DISPLAY", 
              DISPLAY_TASK_STACK_DEPTH, 
              NULL, 
              DISPLAY_TASK_PRIORITY, 
              &DisplayHandle);
  
    
  ClearShippingModeFlag();

}



/*! LCD Task Main Loop
 *
 * \param pvParameters
 *
 */
static void DisplayTask(void *pvParameters)
{
  if ( QueueHandles[DISPLAY_QINDEX] == 0 )
  {
    PrintString("Display Queue not created!\r\n");  
  }
  
  DisplayStartupScreen();
  
  InitialiazeIdleBufferConfig();
  InitializeIdleBufferInvert();
  InitializeDisplaySeconds();
  InitializeLinkAlarmEnable();
  InitializeTimeFormat();
  InitializeDateFormat();
  BluetoothSniffModeInitialize();
  AllocateDisplayTimers();
  
  InitIdlePage(IdleModeTimerId,
	       pMyBuffer);
  
  SetupSplashScreenTimeout();
  
  // FIXME?
  //DontChangeButtonConfiguration();
  DefaultApplicationAndNotificationButtonConfiguration();
  
  for(;;)
  {
    if( pdTRUE == xQueueReceive(QueueHandles[DISPLAY_QINDEX], 
                                &pDisplayMsg, portMAX_DELAY) )
    {
      DisplayQueueMessageHandler(pDisplayMsg);
      
      BPL_FreeMessageBuffer(&pDisplayMsg);
      
      CheckStackUsage(DisplayHandle,"Display");
    }
  }
}
  
/*! Display the startup image or Splash Screen */
static void DisplayStartupScreen(void)
{
  CopyRowsIntoMyBuffer(pMetaWatchSplash,STARTING_ROW,NUM_LCD_ROWS);

  PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
  SendMyBufferToLcd(NUM_LCD_ROWS);
}

/*! Handle the messages routed to the display queue */
static void DisplayQueueMessageHandler(tHostMsg* pMsg)
{
  unsigned char Type = pMsg->Type;
      
  switch(Type)
  {
  
  case IdleUpdate:
	  IdlePageHandler(&IdlePageMain);
	  break;

  case ChangeModeMsg:
    ChangeModeHandler(pMsg);
    break;
    
  case ModeTimeoutMsg:
    ModeTimeoutHandler(pMsg);
    break;

  case WatchStatusMsg:
    IdlePageHandler(&IdlePageWatchStatus);
    break;
 
  case BarCode:

#ifdef SPACEINV

	  if(CurrentIdlePage != QrCodePage)
	  {
		  // Setup every thing.
		  IdlePageQrCodeInit(QrCodePage);
		  CurrentIdlePage = QrCodePage;
		  ConfigureIdleUserInterfaceButtons();
	  }

	  IdlePageQrCodeButtonHandler(pMsg->Options);
//#else
          if(CurrentIdlePage != QrCodePage)
	  {
              //copy(lowerhalf to pMyBuffer)
              IdlePageStart(&IdlePageGameOfLife);
          }
	  CurrentIdlePage = QrCodePage;
          IdlePageHandler(&IdlePageGameOfLife);
	  //IdlePageGameOfLifeHandler(IdleModeTimerId, pMyBuffer);

	  // And this should be moved too
	  /* display entire buffer */
	  PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
	  SendMyBufferToLcd(NUM_LCD_ROWS);
#endif
    break;
    
  case ListPairedDevicesMsg:
    IdlePageHandler(&IdlePageListPairedDevices);
    break;
  
  case WatchDrawnScreenTimeout:
	  IdlePageHandler(&IdlePageMain);
    break;
    
  case ConfigureMode:
    break;
  
  case ConfigureIdleBufferSize:
    ConfigureIdleBuferSizeHandler(pMsg);
    break;

  case ConnectionStateChangeMsg:
    ConnectionStateChangeHandler();
    break;
    
  case ModifyTimeMsg:
    ModifyTimeHandler(pMsg);
    break;
   
  case MenuModeMsg:
    MenuModeHandler(pMsg->Options);
    break;
  
  case MenuButtonMsg:
    MenuButtonHandler(pMsg->Options);
    break;
      
  case ToggleSecondsMsg:
    ToggleSecondsHandler();
	  IdlePageHandler(&IdlePageMain);
    break;
  
  case SplashTimeoutMsg:
    AllowConnectionStateChangeToUpdateScreen = 1;
	  IdlePageHandler(&IdlePageMain);
    break;
    
  case LinkAlarmMsg:
    if ( QueryLinkAlarmEnable() )
    {
      GenerateLinkAlarm();  
    }
    break;
  default:
    PrintStringAndHex("<<Unhandled Message>> in Lcd Display Task: Type 0x", Type);
    break;
  }

}

/*! Allocate ids and setup timers for the display modes */
static void AllocateDisplayTimers(void)
{
  IdleModeTimerId = AllocateOneSecondTimer();

  ApplicationModeTimerId = AllocateOneSecondTimer();

  NotificationModeTimerId = AllocateOneSecondTimer();

}

static void SetupSplashScreenTimeout(void)
{
  SetupOneSecondTimer(IdleModeTimerId,
                      ONE_SECOND*3,
                      NO_REPEAT,
                      SplashTimeoutMsg,
                      NO_MSG_OPTIONS);

  StartOneSecondTimer(IdleModeTimerId);
  
  AllowConnectionStateChangeToUpdateScreen = 0;
  
}

void StopAllDisplayTimers(void)
{
  StopOneSecondTimer(IdleModeTimerId);
  StopOneSecondTimer(ApplicationModeTimerId);
  StopOneSecondTimer(NotificationModeTimerId);

}

/*! Draw the Idle screen and cause the remainder of the display to be updated
 * also
 */

#if 0
static void DetermineIdlePage(void)
{
	// mostly this just set the icon to draw for the screen if
	// We're not connected
	// If we are connected we should switch to Main Page
  etConnectionState cs = QueryConnectionState(); 
  
  switch (cs) 
  {
  case Initializing:       CurrentIdlePage = BluetoothOffPage;              break;
  case ServerFailure:      CurrentIdlePage = BluetoothOffPage;              break;
  case RadioOn:            CurrentIdlePage = RadioOnWithoutPairingInfoPage; break;
  case Paired:             CurrentIdlePage = RadioOnWithPairingInfoPage;    break;
  case Connected:          CurrentIdlePage = NormalPage;                    break;
  case RadioOff:           CurrentIdlePage = BluetoothOffPage;              break;
  case RadioOffLowBattery: CurrentIdlePage = BluetoothOffPage;              break;
  case ShippingMode:       CurrentIdlePage = BluetoothOffPage;              break;
  default:                 CurrentIdlePage = BluetoothOffPage;              break;  
  }
  
  /* if the radio is on but hasn't paired yet then don't show the pairing icon */
  if ( CurrentIdlePage == RadioOnWithoutPairingInfoPage )
  {
    if ( QueryValidPairingInfo() )
    {
      CurrentIdlePage = RadioOnWithPairingInfoPage;  
    }
  }
}
#endif

static void ConnectionStateChangeHandler(void)
{
  if ( AllowConnectionStateChangeToUpdateScreen )
  {
    /* certain pages should not be exited when a change in the 
     * connection state has occurred 
     */
	  // Should be replaced with update idle page


	  //etConnectionState cs = QueryConnectionState();

	  //switch (cs)
	  //{
	  //case RadioOn:            CurrentIdlePage = RadioOnWithoutPairingInfoPage; break;
	  //case Paired:             CurrentIdlePage = RadioOnWithPairingInfoPage;    break;
	  //case Connected:          CurrentIdlePage = NormalPage;                    break;
	  //case RadioOff:           CurrentIdlePage = BluetoothOffPage;              break;
	  //case RadioOffLowBattery: CurrentIdlePage = BluetoothOffPage;              break;
	  //case ShippingMode:       CurrentIdlePage = BluetoothOffPage;              break;
	  //default:                 CurrentIdlePage = BluetoothOffPage;              break;
    //case Initializing:       CurrentIdlePage = BluetoothOffPage;              break;
    //case ServerFailure:      CurrentIdlePage = BluetoothOffPage;              break;

  //  IdlePageUpdate();
  IdlePageHandler(IdlePageCurrent());


/*    switch ( cs )
    {
    case Connected:
    case Paired:
    case RadioOn:
    case RadioOff:
    case RadioOffLowBattery:
    case ShippingMode:
    default:
    case Initializing:
    case ServerFailure:
      // Switch to main page
      IdlePageHandler(&IdlePageMain);
  	  CurrentIdlePage = NormalPage;
  	  ConfigureIdleUserInterfaceButtons();
      break;
    case MenuPage:

      //MenuModeHandler(MENU_MODE_OPTION_UPDATE_CURRENT_PAGE);
      //break;
    
    case ListPairedDevicesPage:
      //IdlePageHandler(&IdlePageListPairedDevices);
      //break;
    
    case WatchStatusPage:
      ///IdlePageHandler(&IdlePageWatchStatus);
      //break;
      IdlePageUpdate();
      break;
    case QrCodePage:
      break;
      
    default:
      break;
    }*/
  }
}

unsigned char QueryDisplayMode(void)
{
  return CurrentMode;  
}

static void ChangeModeHandler(tHostMsg* pMsg)
{
  LastMode = CurrentMode;    
  CurrentMode = (pMsg->Options & MODE_MASK);
  
  switch ( CurrentMode )
  {
  
  case IDLE_MODE:

    /* this check is so that the watch apps don't mess up the timer */
    if ( LastMode != CurrentMode )
    {
      /* idle update handler will stop all display clocks */
  	  IdlePageHandler(&IdlePageMain);
      PrintString("Changing mode to Idle\r\n");
    }
    else
    {
      PrintString("Already in Idle mode\r\n");
    }
    break;
  
  case APPLICATION_MODE:
    
    StopAllDisplayTimers();
    
    SetupOneSecondTimer(ApplicationModeTimerId,
                        QueryApplicationModeTimeout(),
                        NO_REPEAT,
                        ModeTimeoutMsg,
                        APPLICATION_MODE);
    
    /* don't start the timer if the timeout == 0 
     * this invites things that look like lock ups...
     * it is preferred to make this a large value
     */
    if ( QueryApplicationModeTimeout() )
    {
      StartOneSecondTimer(ApplicationModeTimerId);
    }
    
    PrintString("Changing mode to Application\r\n");
    break;
  
  case NOTIFICATION_MODE:
    StopAllDisplayTimers();
    
    SetupOneSecondTimer(NotificationModeTimerId,
                        QueryNotificationModeTimeout(),
                        NO_REPEAT,
                        ModeTimeoutMsg,
                        NOTIFICATION_MODE);
        
    if ( QueryNotificationModeTimeout() )
    {
      StartOneSecondTimer(NotificationModeTimerId);
    }
    
    PrintString("Changing mode to Notification\r\n");
    break;
  
  default:
    break;
  }
  
  /* 
   * send a message to the Host indicating buffer update / mode change 
   * has completed (don't send message if it is just watch updating time ).
   */
  if ( LastMode != CurrentMode )
  {
    tHostMsg* pOutgoingMsg;
    BPL_AllocMessageBuffer(&pOutgoingMsg);
    unsigned char data = (unsigned char)eScUpdateComplete;
    UTL_BuildHstMsg(pOutgoingMsg, StatusChangeEvent, IDLE_MODE, &data, sizeof(data));
    RouteMsg(&pOutgoingMsg);
    
//    if ( LastMode == APPLICATION_MODE )
//    {    
//      ReturnToApplicationMode = 1;
//    }
//    else
//    {
//      ReturnToApplicationMode = 0;  
//    }
  }
  
}

static void ModeTimeoutHandler(tHostMsg* pMsg)
{
  tHostMsg* pOutgoingMsg;
  
  switch ( CurrentMode )
  {
  
  case IDLE_MODE:
    break;
  
  case APPLICATION_MODE:
  case NOTIFICATION_MODE:
  case SCROLL_MODE: 
    /* go back to idle mode */
	  IdlePageHandler(&IdlePageMain);
    break;

  default:
    break;
  }  
  
  /* send a message to the host */
  BPL_AllocMessageBuffer(&pOutgoingMsg);
  unsigned char data = (unsigned char)eScModeTimeout;
  UTL_BuildHstMsg(pOutgoingMsg, 
                  StatusChangeEvent, 
                  NO_MSG_OPTIONS, 
                  &data, sizeof(data));
  
  RouteMsg(&pOutgoingMsg);
  
}
  








static void ConfigureIdleBuferSizeHandler(tHostMsg* pMsg)
{
  nvIdleBufferConfig = pMsg->pPayload[0] & IDLE_BUFFER_CONFIG_MASK;
  
  if ( nvIdleBufferConfig == WATCH_CONTROLS_TOP )
  {
	  IdlePageHandler(&IdlePageMain);
  }
}

static void ModifyTimeHandler(tHostMsg* pMsg)
{
  int time;
  switch (pMsg->Options)
  {
  case MODIFY_TIME_INCREMENT_HOUR:
    /*! todo - make these functions */
    time = GetRTCHOUR();
    time++; if ( time == 24 ) time = 0;
    SetRTCHOUR(time);
    break;
  case MODIFY_TIME_INCREMENT_MINUTE:
    time = GetRTCMIN();
    time++; if ( time == 60 ) time = 0;
    SetRTCMIN(time);
    break;
  case MODIFY_TIME_INCREMENT_DOW:
    /* modify the day of the week (not the day of the month) */
    time = GetRTCDOW();
    time++; if ( time == 7 ) time = 0;
    SetRTCDOW(time);
    break;
  }
  
  /* now redraw the screen */
  IdlePageHandler(&IdlePageMain);
  
}

unsigned char GetIdleBufferConfiguration(void)
{
  return nvIdleBufferConfig;  
}


void SendMyBufferToLcd(unsigned char TotalRows)
{
  tHostMsg* pOutgoingMsg;
  
#if 0
  BPL_AllocMessageBuffer(&pOutgoingMsg);
  ((tUpdateMyDisplayMsg*)pOutgoingMsg)->Type = ClearLcdSpecial;
  RouteMsg(&pOutgoingMsg);
#endif
  
  
  /* 
   * since my buffer is in MSP430 memory it can go directly to the Lcd BUT
   * to preserve draw order it goes to the sram queue first
   */
  BPL_AllocMessageBuffer(&pOutgoingMsg);
  ((tUpdateMyDisplayMsg*)pOutgoingMsg)->Type = UpdateMyDisplaySram;
  ((tUpdateMyDisplayMsg*)pOutgoingMsg)->TotalLines = TotalRows;
  ((tUpdateMyDisplayMsg*)pOutgoingMsg)->pMyDisplay = (unsigned char*)pMyBuffer;
  RouteMsg(&pOutgoingMsg);
}

static void MenuModeHandler(unsigned char MsgOptions)
{
  StopAllDisplayTimers();
  /* MENU MODE DOES NOT TIMEOUT */
  DrawMenu();
  //CurrentIdlePage = MenuPage;
  //ConfigureIdleUserInterfaceButtons();
  menu_config_buttons();

  /* only invert the part that was just drawn */
  PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
  SendMyBufferToLcd(NUM_LCD_ROWS);
}

static void MenuButtonHandler(unsigned char MsgOptions)
{
	StopAllDisplayTimers();

	if(menu_button_handler(MsgOptions))
	{
		MenuModeHandler(MENU_MODE_OPTION_UPDATE_CURRENT_PAGE);
	}
}



unsigned char* GetTemplatePointer(unsigned char TemplateSelect)
{
  return NULL;
}

#if 0
static void ConfigureIdleUserInterfaceButtons(void)
{
  if ( CurrentIdlePage != LastIdlePage )
  {
    LastIdlePage = CurrentIdlePage;
   // LOOK AT THIS
    //FIXME: HMMM
    /* only allow reset on one of the pages */
    DisableButtonAction(IDLE_MODE,
                        SW_F_INDEX,
                        BUTTON_STATE_PRESSED);
    
    switch ( CurrentIdlePage )
    {
    case NormalPage:
    case RadioOnWithPairingInfoPage:
    	IdlePageConfigButtons(&IdlePageMain);
    	break;
      
    case BluetoothOffPage:
    case RadioOnWithoutPairingInfoPage:
    	IdlePageConfigButtons(&IdlePageMain);
    	break;
      
    case MenuPage:
    	menu_config_buttons();
    	break;
      
    case ListPairedDevicesPage:
    	IdlePageConfigButtons(&IdlePageListPairedDevices);
    	break;

    case WatchStatusPage:
    	IdlePageConfigButtons(&IdlePageWatchStatus);
    	break;

    case QrCodePage:
#ifdef SPACEINV
    	IdlePageQrCodeConfigButtons();
//#else
//    	IdlePageGameOfLifeConfigButtons();
        IdlePageConfigButtons(&IdlePageGameOfLife);
#endif
      break;
    }
  }
}
#endif

/* the default is for all simple button presses to be sent to the phone */
static void DefaultApplicationAndNotificationButtonConfiguration(void)
{  
  unsigned char index = 0;
  
  /* 
   * this will configure the pull switch even though it does not exist 
   * on the watch
   */
  for(index = 0; index < NUMBER_OF_BUTTONS; index++)
  {
    if ( index == SW_UNUSED_INDEX )
    {
      index++;
    }
   
    EnableButtonAction(APPLICATION_MODE,
                       index,
                       BUTTON_STATE_PRESSED,
                       ButtonEventMsg,
                       NO_MSG_OPTIONS);

    EnableButtonAction(NOTIFICATION_MODE,
                       index,
                       BUTTON_STATE_PRESSED,
                       ButtonEventMsg,
                       NO_MSG_OPTIONS);
  
  }

}


/******************************************************************************/

static void InitialiazeIdleBufferConfig(void)
{
  nvIdleBufferConfig = WATCH_CONTROLS_TOP;
  OsalNvItemInit(NVID_IDLE_BUFFER_CONFIGURATION, 
                 sizeof(nvIdleBufferConfig), 
                 &nvIdleBufferConfig);    
}

static void InitializeIdleBufferInvert(void)
{
  nvIdleBufferInvert = 0;
  OsalNvItemInit(NVID_IDLE_BUFFER_INVERT, 
                 sizeof(nvIdleBufferInvert), 
                 &nvIdleBufferInvert);   
}

void ToggleIdleBufferInvert(void)
{
	if ( nvIdleBufferInvert == 1 )
	{
		nvIdleBufferInvert = 0;
	}
	else
	{
		nvIdleBufferInvert = 1;
	}
}
unsigned char QueryInvertDisplay(void)
{
  return nvIdleBufferInvert;
}

void SaveIdleBufferInvert(void)
{
  osal_nv_write(NVID_IDLE_BUFFER_INVERT,
                NV_ZERO_OFFSET,
                sizeof(nvIdleBufferInvert), 
                &nvIdleBufferInvert);   
}




#if 0
static void SaveIdleBufferConfig(void)
{
  osal_nv_write(NVID_IDLE_BUFFER_CONFIGURATION,
                NV_ZERO_OFFSET,
                sizeof(nvIdleBufferConfig),
                &nvIdleBufferConfig);
}
#endif



#ifdef FONT_TESTING
static unsigned char gBitColumnMask;
static unsigned char gColumn;
static unsigned char gRow;
static unsigned int CharacterMask;
static unsigned char CharacterRows;
static unsigned char CharacterWidth;
static unsigned int bitmap[MAX_FONT_ROWS];

/* fonts can be up to 16 bits wide */
static void WriteFontCharacter(unsigned char Character)
{
  CharacterMask = BIT0;
  CharacterRows = GetCharacterHeight();
  CharacterWidth = GetCharacterWidth(Character);
  GetCharacterBitmap(Character,(unsigned int*)&bitmap);
  
  if ( gRow + CharacterRows > NUM_LCD_ROWS )
  {
    PrintString("Not enough rows to display character\r\n");
    return;  
  }
  
  /* do things bit by bit */
  unsigned char i = 0;
  for ( ; i < CharacterWidth && gColumn < NUM_LCD_COL_BYTES; i++ )
  {
    for(unsigned char row = 0; row < CharacterRows; row++)
    {
      if ( (CharacterMask & bitmap[row]) != 0 )
      {
        pMyBuffer[gRow+row].Data[gColumn] |= gBitColumnMask;  
      }
    }
    
    /* the shift direction seems backwards... */
    CharacterMask = CharacterMask << 1;
    gBitColumnMask = gBitColumnMask << 1;
    if ( gBitColumnMask == 0 )
    {
      gBitColumnMask = BIT0;
      gColumn++;
    }
  
  }
  
  /* add spacing between characters */
  unsigned char FontSpacing = GetFontSpacing();
  for(i = 0; i < FontSpacing; i++)
  {
    gBitColumnMask = gBitColumnMask << 1;
    if ( gBitColumnMask == 0 )
    {
      gBitColumnMask = BIT0;
      gColumn++;
    }  
  }
       
}

void WriteFontString(unsigned char *pString)
{
  unsigned char i = 0;
   
  while (pString[i] != 0 && gColumn < NUM_LCD_COL_BYTES)
  {
    WriteFontCharacter(pString[i++]);    
  }

}

#endif


