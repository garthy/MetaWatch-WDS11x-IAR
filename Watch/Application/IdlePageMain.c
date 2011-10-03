#include "Messages.h"
#include "hal_rtc.h"
#include "hal_battery.h"
#include "hal_lcd.h"
#include "OneSecondTimers.h"
#include "Adc.h"
#include "OSAL_Nv.h"
#include "NvIds.h"
#include "Icons.h"
#include "Fonts.h"
#include "Display.h"
#include "LcdBuffer.h"
#include "Bluetooth.h"

const unsigned char Am[10*4];
const unsigned char Pm[10*4];
const unsigned char DaysOfWeek[7][10*4];


static void DisplayAmPm(void);
static void DisplayDayOfWeek(void);
static void DisplayDate(void);

void DisplayDataSeparator(unsigned char RowOffset,
                                 unsigned char ColumnOffset);


unsigned char nvDisplaySeconds = 0;
void InitializeDisplaySeconds(void)
{
  nvDisplaySeconds = 0;
  OsalNvItemInit(NVID_DISPLAY_SECONDS,
                 sizeof(nvDisplaySeconds),
                 &nvDisplaySeconds);
}

int GetDisplaySeconds(void)
{
	return nvDisplaySeconds;
}

void ToggleSecondsHandler(void)
{
	if ( nvDisplaySeconds == 0 )
	{
		nvDisplaySeconds = 1;
	}
	else
	{
		nvDisplaySeconds = 0;
	}
}

void SaveDisplaySeconds(void)
{
	  osal_nv_write(NVID_DISPLAY_SECONDS,
	                NV_ZERO_OFFSET,
	                sizeof(nvDisplaySeconds),
	                &nvDisplaySeconds);
}



void DrawIdleScreen(void)
{
  unsigned char msd;
  unsigned char lsd;

  unsigned char Row = 6;
  unsigned char Col = 0;

  /* display hour */
  int Hour = GetRTCHOUR();

  /* if required convert to twelve hour format */
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    if ( Hour == 0 )
    {
      Hour = 12;
    }
    else if ( Hour > 12 )
    {
      Hour -= 12;
    }
  }

  msd = Hour / 10;
  lsd = Hour % 10;

  /* if first digit is zero then leave location blank */
  if ( msd != 0 )
  {
    WriteTimeDigit(msd,Row,Col,LEFT_JUSTIFIED);
  }
  Col += 1;
  WriteTimeDigit(lsd,Row,Col,RIGHT_JUSTIFIED);
  Col += 2;

  /* the colon takes the first 5 bits on the byte*/
  WriteTimeColon(Row,Col,RIGHT_JUSTIFIED);
  Col+=1;

  /* display minutes */
  int Minutes = GetRTCMIN();
  msd = Minutes / 10;
  lsd = Minutes % 10;
  WriteTimeDigit(msd,Row,Col,RIGHT_JUSTIFIED);
  Col += 2;
  WriteTimeDigit(lsd,Row,Col,LEFT_JUSTIFIED);

  if ( nvDisplaySeconds )
  {
    /* the final colon's spacing isn't quite the same */
    int Seconds = GetRTCSEC();
    msd = Seconds / 10;
    lsd = Seconds % 10;

    Col +=2;
    WriteTimeColon(Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(msd,Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(lsd,Row,Col,RIGHT_JUSTIFIED);

  }
  else /* now things starting getting fun....*/
  {
    DisplayAmPm();

    if ( QueryBluetoothOn() == 0 )
    {
      CopyColumnsIntoMyBuffer(pBluetoothOffIdlePageIcon,
                              IDLE_PAGE_ICON_STARTING_ROW,
                              IDLE_PAGE_ICON_SIZE_IN_ROWS,
                              IDLE_PAGE_ICON_STARTING_COL,
                              IDLE_PAGE_ICON_SIZE_IN_COLS);
    }
    else if ( QueryPhoneConnected() == 0 )
    {
      CopyColumnsIntoMyBuffer(pPhoneDisconnectedIdlePageIcon,
                              IDLE_PAGE_ICON_STARTING_ROW,
                              IDLE_PAGE_ICON_SIZE_IN_ROWS,
                              IDLE_PAGE_ICON_STARTING_COL,
                              IDLE_PAGE_ICON_SIZE_IN_COLS);
    }
    else
    {
      if ( QueryBatteryCharging() )
      {
        CopyColumnsIntoMyBuffer(pBatteryChargingIdlePageIconType2,
                                IDLE_PAGE_ICON2_STARTING_ROW,
                                IDLE_PAGE_ICON2_SIZE_IN_ROWS,
                                IDLE_PAGE_ICON2_STARTING_COL,
                                IDLE_PAGE_ICON2_SIZE_IN_COLS);
      }
      else
      {
        unsigned int bV = ReadBatterySenseAverage();

        if ( bV < 3500 )
        {
          CopyColumnsIntoMyBuffer(pLowBatteryIdlePageIconType2,
                                  IDLE_PAGE_ICON2_STARTING_ROW,
                                  IDLE_PAGE_ICON2_SIZE_IN_ROWS,
                                  IDLE_PAGE_ICON2_STARTING_COL,
                                  IDLE_PAGE_ICON2_SIZE_IN_COLS);
        }
        else
        {
          DisplayDayOfWeek();
          DisplayDate();
        }
      }
    }
  }
}

static void DisplayAmPm(void)
{
  /* don't display am/pm in 24 hour mode */
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    int Hour = GetRTCHOUR();

    unsigned char const *pFoo;

    if ( Hour >= 12 )
    {
      pFoo = Pm;
    }
    else
    {
      pFoo = Am;
    }

    WriteFoo(pFoo,0,8);
  }

}


static void DisplayDayMonth(int First, int Second,  unsigned char row)
{
	/* shift bit so that it lines up with AM/PM and Day of Week */
	WriteSpriteDigit(First / 10, row, 8, -1);
	/* shift the bits so we can fit a / in the middle */
	WriteSpriteDigit(First % 10, row, 9, -1);
	WriteSpriteDigit(Second / 10, row, 10, 1);
	WriteSpriteDigit(Second % 10, row, 11, 0);
	DisplayDataSeparator(row, 9);
}


static void DisplayYear(int year, unsigned char row, unsigned char col)
{
	  unsigned int bar = 1000;
	  unsigned int temp = 0;

	  for ( unsigned char i = 0; i < 4; i++ )
	  {
	    temp = year / bar;
	    WriteSpriteDigit(temp,row,col+i,0);
	    year = year % bar;
	    bar = bar / 10;
	  }
}

static void DisplayDayOfWeek(void)
{
  int DayOfWeek = GetRTCDOW();

  unsigned char const *pFoo = DaysOfWeek[DayOfWeek];
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    WriteFoo(pFoo,10,8);
  }
  else
  {
	// move it up so we can fit the year
    WriteFoo(pFoo,0,8);
  }
}

static void DisplayDate(void)
{
  if ( QueryFirstContact() )
  {
    int First;
    int Second;

    /* determine if month or day is displayed first */
    if ( GetDateFormat() == MONTH_FIRST )
    {
      First = GetRTCMON();
      Second = GetRTCDAY();
    }
    else
    {
      First = GetRTCDAY();
      Second = GetRTCMON();
    }
    if ( GetTimeFormat() == TWELVE_HOUR )
    {
    	DisplayDayMonth(First, Second, 20);
    }
    else
    {
    	int year = GetRTCYEAR();
    	DisplayDayMonth(First, Second, 10);

    	/* Write the year */
    	DisplayYear(year, 20, 8);
    }
  }
}




void DrawSimpleIdleScreen(void)
{
  unsigned char msd;
  unsigned char lsd;

  unsigned char Row = 6;
  unsigned char Col = 0;

  /* display hour */
  int Hour = GetRTCHOUR();

  /* if required convert to twelve hour format */
  if ( GetTimeFormat() == TWELVE_HOUR )
  {
    if ( Hour == 0 )
    {
      Hour = 12;
    }
    else if ( Hour > 12 )
    {
      Hour -= 12;
    }
  }

  msd = Hour / 10;
  lsd = Hour % 10;

  /* if first digit is zero then leave location blank */
  if ( msd != 0 )
  {
    WriteTimeDigit(msd,Row,Col,LEFT_JUSTIFIED);
  }
  Col += 1;
  WriteTimeDigit(lsd,Row,Col,RIGHT_JUSTIFIED);
  Col += 2;

  /* the colon takes the first 5 bits on the byte*/
  WriteTimeColon(Row,Col,RIGHT_JUSTIFIED);
  Col+=1;

  /* display minutes */
  int Minutes = GetRTCMIN();
  msd = Minutes / 10;
  lsd = Minutes % 10;
  WriteTimeDigit(msd,Row,Col,RIGHT_JUSTIFIED);
  Col += 2;
  WriteTimeDigit(lsd,Row,Col,LEFT_JUSTIFIED);

  if ( nvDisplaySeconds )
  {
    /* the final colon's spacing isn't quite the same */
    int Seconds = GetRTCSEC();
    msd = Seconds / 10;
    lsd = Seconds % 10;

    Col +=2;
    WriteTimeColon(Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(msd,Row,Col,LEFT_JUSTIFIED);
    Col += 1;
    WriteTimeDigit(lsd,Row,Col,RIGHT_JUSTIFIED);

  }
  else
  {
    DisplayAmPm();
    DisplayDayOfWeek();
    DisplayDate();

  }

}



const unsigned char Am[10*4] =
{
0x00,0x00,0x9C,0xA2,0xA2,0xA2,0xBE,0xA2,0xA2,0x00,
0x00,0x00,0x08,0x0D,0x0A,0x08,0x08,0x08,0x08,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

const unsigned char Pm[10*4] =
{
0x00,0x00,0x9E,0xA2,0xA2,0x9E,0x82,0x82,0x82,0x00,
0x00,0x00,0x08,0x0D,0x0A,0x08,0x08,0x08,0x08,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

const unsigned char DaysOfWeek[7][10*4] =
{
0x00,0x00,0x9C,0xA2,0x82,0x9C,0xA0,0xA2,0x1C,0x00,
0x00,0x00,0x28,0x68,0xA8,0x28,0x28,0x28,0x27,0x00,
0x00,0x00,0x02,0x02,0x02,0x03,0x02,0x02,0x02,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x22,0xB6,0xAA,0xA2,0xA2,0xA2,0x22,0x00,
0x00,0x00,0x27,0x68,0xA8,0x28,0x28,0x28,0x27,0x00,
0x00,0x00,0x02,0x02,0x02,0x03,0x02,0x02,0x02,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xBE,0x88,0x88,0x88,0x88,0x88,0x08,0x00,
0x00,0x00,0xE8,0x28,0x28,0xE8,0x28,0x28,0xE7,0x00,
0x00,0x00,0x03,0x00,0x00,0x01,0x00,0x00,0x03,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xA2,0xA2,0xAA,0xAA,0xAA,0xAA,0x94,0x00,
0x00,0x00,0xEF,0x20,0x20,0x27,0x20,0x20,0xEF,0x00,
0x00,0x00,0x01,0x02,0x02,0x02,0x02,0x02,0x01,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xBE,0x88,0x88,0x88,0x88,0x88,0x88,0x00,
0x00,0x00,0x28,0x28,0x28,0x2F,0x28,0x28,0xC8,0x00,
0x00,0x00,0x7A,0x8A,0x8A,0x7A,0x4A,0x8A,0x89,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xBE,0x82,0x82,0x9E,0x82,0x82,0x82,0x00,
0x00,0x00,0xC7,0x88,0x88,0x87,0x84,0x88,0xC8,0x00,
0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x1C,0xA2,0x82,0x9C,0xA0,0xA2,0x9C,0x00,
0x00,0x00,0xE7,0x88,0x88,0x88,0x8F,0x88,0x88,0x00,
0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


