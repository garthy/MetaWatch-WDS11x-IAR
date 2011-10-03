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
#include "Menus.h"
#include "IdlePageMain.h"
#include "IdlePageQrCode.h"
#include "IdlePageWatchStatus.h"
#include "IdlePageGameOfLife.h"


#define DISPLAY_TASK_QUEUE_LENGTH 8
#define DISPLAY_TASK_STACK_DEPTH	(configMINIMAL_STACK_DEPTH + 90)    
#define DISPLAY_TASK_PRIORITY     (tskIDLE_PRIORITY + 1)
extern const unsigned char pMetaWatchSplash[NUM_LCD_ROWS*NUM_LCD_COL_BYTES];
xTaskHandle DisplayHandle;

static void DisplayTask(void *pvParameters);
static void DisplayQueueMessageHandler(tHostMsg* pMsg);
static void SendMyBufferToLcd(unsigned char TotalRows);

static tHostMsg* pDisplayMsg;
static tTimerId IdleModeTimerId;
static tTimerId ApplicationModeTimerId;
static tTimerId NotificationModeTimerId;

/* Message handlers */

static void IdleUpdateHandler(void);
static void ChangeModeHandler(tHostMsg* pMsg);
static void ModeTimeoutHandler(tHostMsg* pMsg);
static void ListPairedDevicesHandler(void);
static void ConfigureIdleBuferSizeHandler(tHostMsg* pMsg);
static void ModifyTimeHandler(tHostMsg* pMsg);
static void MenuModeHandler(unsigned char MsgOptions);



static void ConnectionStateChangeHandler(void);

/******************************************************************************/
void DrawIdleScreen(void);
void DrawSimpleIdleScreen(void);
static void DrawConnectionScreen(void);
static void DisplayStartupScreen(void);
static void SetupSplashScreenTimeout(void);
static void AllocateDisplayTimers(void);
static void DetermineIdlePage(void);

static void MenuButtonHandler(unsigned char MsgOptions);
void DrawCommonMenuIcons(void);




//static unsigned char const* GetPointerForTimeDigit(unsigned char Digit,
                                                   //unsigned char Offset);

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

static unsigned char AllowConnectionStateChangeToUpdateScreen;

static void ConfigureIdleUserInterfaceButtons(void);

static void DontChangeButtonConfiguration(void);
static void DefaultApplicationAndNotificationButtonConfiguration(void);

/******************************************************************************/

static unsigned char LastMode = IDLE_MODE;
static unsigned char CurrentMode = IDLE_MODE;

//static unsigned char ReturnToApplicationMode;

/******************************************************************************/

static unsigned char pBluetoothAddress[12+1];
static unsigned char pBluetoothName[12+1];

/******************************************************************************/

#ifdef FONT_TESTING
static unsigned char gBitColumnMask;
static unsigned char gColumn;
static unsigned char gRow;

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
  AllocateDisplayTimers();
  SetupSplashScreenTimeout();

  DontChangeButtonConfiguration();
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
    IdleUpdateHandler(); 
    break;

  case ChangeModeMsg:
    ChangeModeHandler(pMsg);
    break;
    
  case ModeTimeoutMsg:
    ModeTimeoutHandler(pMsg);
    break;

  case WatchStatusMsg:
    IdlePageWatchStatusHandler(IdleModeTimerId);
    PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
    SendMyBufferToLcd(NUM_LCD_ROWS);
	CurrentIdlePage = WatchStatusPage;
	ConfigureIdleUserInterfaceButtons();
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
#else
          if(CurrentIdlePage != QrCodePage)
	  {
              //copy(lowerhalf to pMyBuffer)
          }
	  CurrentIdlePage = QrCodePage;
	  IdlePageGameOfLifeHandler(IdleModeTimerId, pMyBuffer);
#endif
	  // And this should be moved too
	  /* display entire buffer */
	  PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
	  SendMyBufferToLcd(NUM_LCD_ROWS);
    break;
    
  case ListPairedDevicesMsg:
    ListPairedDevicesHandler();
    break;
  
  case WatchDrawnScreenTimeout:
    IdleUpdateHandler();
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
    IdleUpdateHandler();
    break;
  
  case SplashTimeoutMsg:
    AllowConnectionStateChangeToUpdateScreen = 1;
    IdleUpdateHandler();
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
static void IdleUpdateHandler(void)
{
  StopAllDisplayTimers();
  
  /* select between 1 second and 1 minute */
  int IdleUpdateTime;
  if ( GetDisplaySeconds() )
  {
    IdleUpdateTime = ONE_SECOND;
  }
  else
  {
    IdleUpdateTime = ONE_SECOND * 60;
  }
  
  /* setup a timer to determine when to draw the screen again */
  SetupOneSecondTimer(IdleModeTimerId,
                      IdleUpdateTime,
                      REPEAT_FOREVER,
                      IdleUpdate,
                      NO_MSG_OPTIONS);
  
  StartOneSecondTimer(IdleModeTimerId);

  /* determine if the bottom of the screen should be drawn by the watch */  
  if ( QueryFirstContact() )
  {
    /* 
     * immediately update the screen
     */ 
    if ( nvIdleBufferConfig == WATCH_CONTROLS_TOP )
    {
      /* only draw watch part */
      FillMyBuffer(STARTING_ROW,WATCH_DRAWN_IDLE_BUFFER_ROWS,0x00);
      DrawIdleScreen();
      PrepareMyBufferForLcd(STARTING_ROW,WATCH_DRAWN_IDLE_BUFFER_ROWS);
      SendMyBufferToLcd(WATCH_DRAWN_IDLE_BUFFER_ROWS);
    }
  
    /* now update the remainder of the display */
    /*! make a dirty flag for the idle page drawn by the phone
     * set it whenever watch uses whole screen
     */
    tHostMsg* pOutgoingMsg;
       BPL_AllocMessageBuffer(&pOutgoingMsg);
       pOutgoingMsg->Type = UpdateDisplay;
       pOutgoingMsg->Options = IDLE_MODE | DONT_ACTIVATE_DRAW_BUFFER;
       RouteMsg(&pOutgoingMsg);
   
    CurrentIdlePage = NormalPage;
    ConfigureIdleUserInterfaceButtons();
    
  }
  else
  {
    DetermineIdlePage();

    FillMyBuffer(STARTING_ROW,NUM_LCD_ROWS,0x00);
    DrawSimpleIdleScreen();
    DrawConnectionScreen();
    
    PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
    SendMyBufferToLcd(NUM_LCD_ROWS);

    ConfigureIdleUserInterfaceButtons();

  }
   
}

static void DetermineIdlePage(void)
{
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


static void ConnectionStateChangeHandler(void)
{
  if ( AllowConnectionStateChangeToUpdateScreen )
  {
    /* certain pages should not be exited when a change in the 
     * connection state has occurred 
     */
    switch ( CurrentIdlePage )
    {
    case ReservedPage:
    case NormalPage:
    case RadioOnWithPairingInfoPage:
    case RadioOnWithoutPairingInfoPage:
    case BluetoothOffPage:
      IdleUpdateHandler();
      break;
    case MenuPage:
      MenuModeHandler(MENU_MODE_OPTION_UPDATE_CURRENT_PAGE);
      break;
    
    case ListPairedDevicesPage:
      ListPairedDevicesHandler();
      break;
    
    case WatchStatusPage:
      IdlePageWatchStatusHandler(IdleModeTimerId);
      PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
      SendMyBufferToLcd(NUM_LCD_ROWS);
	  CurrentIdlePage = WatchStatusPage;
	  ConfigureIdleUserInterfaceButtons();
      break;
    
    case QrCodePage:
      break;
      
    default:
      break;
    }
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
      IdleUpdateHandler();
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
    IdleUpdateHandler();
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
  





static void ListPairedDevicesHandler(void)
{  
  StopAllDisplayTimers();
  
  unsigned char row = 0;
  unsigned char col = 0;
  
  /* draw entire region */
  FillMyBuffer(STARTING_ROW,NUM_LCD_ROWS,0x00);
  
  for(unsigned char i = 0; i < 3; i++)
  {

    unsigned char j;
    pBluetoothName[0] = 'D';
    pBluetoothName[1] = 'e';
    pBluetoothName[2] = 'v';
    pBluetoothName[3] = 'i';
    pBluetoothName[4] = 'c';
    pBluetoothName[5] = 'e';
    pBluetoothName[6] = ' ';
    pBluetoothName[7] = 'N';
    pBluetoothName[8] = 'a';
    pBluetoothName[9] = 'm';
    pBluetoothName[10] = 'e';
    pBluetoothName[11] = '1' + i;
    
    for(j = 0; j < sizeof(pBluetoothAddress); j++)
    {
      pBluetoothAddress[j] = '0';
    }
    
    QueryLinkKeys(i,pBluetoothAddress,pBluetoothName,12);
    
    WriteString(pBluetoothName,row,col,DONT_ADD_SPACE_AT_END);
    row += 12;
    
    WriteString(pBluetoothAddress,row,col,DONT_ADD_SPACE_AT_END);
    row += 12+5;
      
  }
  
  PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
  SendMyBufferToLcd(NUM_LCD_ROWS);

  CurrentIdlePage = ListPairedDevicesPage;
  ConfigureIdleUserInterfaceButtons();

}


static void DrawConnectionScreen()
{
  
  /* this is part of the idle update
   * timing is controlled by the idle update timer
   * buffer was already cleared when drawing the time
   */
  
  unsigned char const* pSwash;
  
  switch (CurrentIdlePage)
  {
  case RadioOnWithPairingInfoPage:     
    pSwash = pBootPageConnectionSwash;  
    break;
  case RadioOnWithoutPairingInfoPage:
    pSwash = pBootPagePairingSwash;
    break;
  case BluetoothOffPage:
    pSwash = pBootPageBluetoothOffSwash;
    break;
  default:
    pSwash = pBootPageUnknownSwash;
    break;
  
  }
  
  CopyRowsIntoMyBuffer(pSwash,WATCH_DRAWN_IDLE_BUFFER_ROWS+1,32);
    
#ifdef FONT_TESTING
  
  gRow = 65;
  gColumn = 0;
  gBitColumnMask = BIT0;
  
  SetFont(MetaWatch5);
  WriteFontString("Peanut Butter");
  
  gRow = 72;
  gColumn = 0;
  gBitColumnMask = BIT0;
  
  SetFont(MetaWatch7);
  //WriteFontString("ABCDEFGHIJKLMNOP");
  WriteFontString("Peanut Butter W");
  
  gRow = 80;
  gColumn = 0;
  gBitColumnMask = BIT0;
  SetFont(MetaWatch16);
  WriteFontString("ABC pqr StuVw");

#else
  
  unsigned char row;
  unsigned char col;

  /* characters are 10h then add space of 2 lines */
  row = 65;
  col = 0;
  col = WriteString(GetLocalBluetoothAddressString(),row,col,DONT_ADD_SPACE_AT_END);

  /* add the firmware version */
  row = 75;
  col = 0;
  col = WriteString("App",row,col,ADD_SPACE_AT_END);
  col = WriteString(VERSION_STRING,row,col,ADD_SPACE_AT_END);
  
  /* and the stack version */
  row = 85;
  col = 0;
  col = WriteString("Stack",row,col,ADD_SPACE_AT_END);
  col = WriteString(GetStackVersion(),row,col,ADD_SPACE_AT_END);

#endif
  
}

static void ConfigureIdleBuferSizeHandler(tHostMsg* pMsg)
{
  nvIdleBufferConfig = pMsg->pPayload[0] & IDLE_BUFFER_CONFIG_MASK;
  
  if ( nvIdleBufferConfig == WATCH_CONTROLS_TOP )
  {
    IdleUpdateHandler();
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
  IdleUpdateHandler();
  
}

unsigned char GetIdleBufferConfiguration(void)
{
  return nvIdleBufferConfig;  
}


static void SendMyBufferToLcd(unsigned char TotalRows)
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
  
  /* draw entire region */
  FillMyBuffer(STARTING_ROW,PHONE_IDLE_BUFFER_ROWS,0x00);

  switch (MsgOptions)
  {

  case MENU_MODE_OPTION_PAGE1:
  case MENU_MODE_OPTION_PAGE2:
  case MENU_MODE_OPTION_PAGE3:
	CurrentIdlePage = MenuPage;
    DrawMenu();
    ConfigureIdleUserInterfaceButtons();
    break;
    
  case MENU_MODE_OPTION_UPDATE_CURRENT_PAGE:
    DrawMenu();

  }

  /* these icons are common to all menus */

  
  /* only invert the part that was just drawn */
  PrepareMyBufferForLcd(STARTING_ROW,NUM_LCD_ROWS);
  SendMyBufferToLcd(NUM_LCD_ROWS);

  /* MENU MODE DOES NOT TIMEOUT */
  
}

/* Menu 1 -> F
 * Menu 2 -> E
 * Menu 3 -> D
 * Menu 4 -> A
 */






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

static void DontChangeButtonConfiguration(void)
{
  /* assign LED button to all modes */
  for ( unsigned char i = 0; i < NUMBER_OF_MODES; i++ )
  {
    /* turn off led 3 seconds after button has been released */
    EnableButtonAction(i,
                       SW_D_INDEX,
                       BUTTON_STATE_PRESSED,
                       LedChange,
                       LED_START_OFF_TIMER);
    
    /* turn on led immediately when button is pressed */
    EnableButtonAction(i,
                       SW_D_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       LedChange,
                       LED_ON_OPTION);
    
    /* software reset is available in all modes */    
    EnableButtonAction(i,
                       SW_F_INDEX,
                       BUTTON_STATE_LONG_HOLD,
                       SoftwareResetMsg,
                       MASTER_RESET_OPTION);
  
  }

}

static void ConfigureIdleUserInterfaceButtons(void)
{
  if ( CurrentIdlePage != LastIdlePage )
  {
    LastIdlePage = CurrentIdlePage;
  
    /* only allow reset on one of the pages */
    DisableButtonAction(IDLE_MODE,
                        SW_F_INDEX,
                        BUTTON_STATE_PRESSED);
    
    switch ( CurrentIdlePage )
    {
    case NormalPage:
    case RadioOnWithPairingInfoPage:
          
      EnableButtonAction(IDLE_MODE,
                         SW_F_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         WatchStatusMsg,
                         RESET_DISPLAY_TIMER);
    
      EnableButtonAction(IDLE_MODE,
                         SW_E_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         ListPairedDevicesMsg,
                         NO_MSG_OPTIONS);
       
      DontChangeButtonConfiguration();
      
      EnableButtonAction(IDLE_MODE,
                         SW_C_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         MenuModeMsg,
                         MENU_MODE_OPTION_PAGE1);
      
      EnableButtonAction(IDLE_MODE,
                         SW_B_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         ToggleSecondsMsg,
                         TOGGLE_SECONDS_OPTIONS_UPDATE_IDLE);
   
      EnableButtonAction(IDLE_MODE,
                         SW_A_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         BarCode,
                         RESET_DISPLAY_TIMER);
      
      break;
      
    case BluetoothOffPage:
    case RadioOnWithoutPairingInfoPage:
   
      EnableButtonAction(IDLE_MODE,
                         SW_F_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         ModifyTimeMsg,
                         MODIFY_TIME_INCREMENT_HOUR);
      
      EnableButtonAction(IDLE_MODE,
                         SW_E_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         ListPairedDevicesMsg,
                         NO_MSG_OPTIONS);
      
      DontChangeButtonConfiguration();
      
      EnableButtonAction(IDLE_MODE,
                         SW_C_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         MenuModeMsg,
                         MENU_MODE_OPTION_PAGE1);
      
      EnableButtonAction(IDLE_MODE,
                         SW_B_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         ModifyTimeMsg,
                         MODIFY_TIME_INCREMENT_DOW);
   
      EnableButtonAction(IDLE_MODE,
                         SW_A_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         ModifyTimeMsg,
                         MODIFY_TIME_INCREMENT_MINUTE);
      break;
      
      
      


    case MenuPage:
    	menu_config_buttons();
      break;
      
    case ListPairedDevicesPage:
      
      EnableButtonAction(IDLE_MODE,
                         SW_F_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         WatchStatusMsg,
                         RESET_DISPLAY_TIMER);
    
      /* map this mode's entry button to go back to the idle mode */
      EnableButtonAction(IDLE_MODE,
                         SW_E_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         IdleUpdate,
                         NO_MSG_OPTIONS);
       
      DontChangeButtonConfiguration();
      
      EnableButtonAction(IDLE_MODE,
                         SW_C_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         MenuModeMsg,
                         MENU_MODE_OPTION_PAGE1);
      
      DisableButtonAction(IDLE_MODE,
                          SW_B_INDEX,
                          BUTTON_STATE_IMMEDIATE);
                          
      /* map this mode's entry button to go back to the idle mode */
      EnableButtonAction(IDLE_MODE,
                         SW_A_INDEX,
                         BUTTON_STATE_IMMEDIATE,
                         BarCode,
                         RESET_DISPLAY_TIMER);
      break;

    case WatchStatusPage:
    	 IdlePageWatchStatusConfigButtons();
    break;

    case QrCodePage:
#ifdef SPACEINV
    	IdlePageQrCodeConfigButtons();
#else
    	IdlePageGameOfLifeConfigButtons();
#endif
      break;
      
      
    }
  }
}

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


unsigned char QueryIdlePageNormal(void)
{
  if ( CurrentIdlePage == NormalPage )
  {
    return 1;
  }
  else
  {
    return 0;  
  };
  
}
