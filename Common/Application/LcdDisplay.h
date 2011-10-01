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
/*! \file LcdDisplay.h
 *
 * This task draws to a local buffer which is then sent to the LCD by the LCD
 * task.  This is how the time is drawn.
 *
 * Menu and button control logic determines what Idle sub-mode the watch is in.
 *
 * When the watch draws into buffers those messages go to the SerialRam task
 * because this task is more of a control task.  This caused some duplication
 * of messages to preserve draw order.
 */
/******************************************************************************/

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#define NUM_LCD_ROWS  96
#define NUM_LCD_COL   96

/*! Create task and queue for display task. Call from main or another task. */
void InitializeDisplayTask(void);

#define WATCH_CONTROLS_TOP       ( 0 )
#define PHONE_CONTROLS_TOP       ( 1 )
#define IDLE_BUFFER_CONFIG_MASK  ( BIT0 )

/*! The phone can control all of the idle buffer or the bottom 2/3.
 * 
 * \return character indicating who controls top third of screen.  When it is
 * zero the watch controls the top.
 */
unsigned char GetIdleBufferConfiguration(void);


#define NORMAL_DISPLAY ( 0 )
#define INVERT_DISPLAY ( 1 )

/*! 
 * \return 1 when display should be inverted 
 */
unsigned char QueryInvertDisplay(void);

/*! Get pointer to an LCD screen template
 *
 *\return NULL - NOT IMPLEMENTED - end result is to have phone be able to program
 * templates into the watch.
 */
unsigned char* GetTemplatePointer(unsigned char TemplateSelect);

/*!
 * \return 1 if the idle page being display is the normal idle page and
 * that a display update from the phone is allowed, 0 otherwise
 */
unsigned char QueryIdlePageNormal(void);

/* the internal buffer */
#define STARTING_ROW                  ( 0 )
#define WATCH_DRAWN_IDLE_BUFFER_ROWS  ( 30 )
#define PHONE_IDLE_BUFFER_ROWS        ( 66 )


unsigned char WriteString(unsigned char* pString,
                          unsigned char RowOffset,
                          unsigned char ColumnOffset,
                          unsigned char AddSpace);

#define ADD_SPACE_AT_END      ( 1 )
#define DONT_ADD_SPACE_AT_END ( 0 )

void AddDecimalPoint8w10h(unsigned char RowOffset,
                                 unsigned char ColumnOffset);

void WriteSpriteDigit(unsigned char Digit,
                      unsigned char RowOffset,
                      unsigned char ColumnOffset,
                      signed char ShiftAmount);

void WriteSpriteChar(unsigned char Char,
					 unsigned char RowOffset,
                     unsigned char ColumnOffset);

// Called before anyupdates to the screen
void StopAllDisplayTimers(void);

void FillMyBuffer(unsigned char StartingRow,
                         unsigned char NumberOfRows,
                         unsigned char FillValue);

void CopyRowsIntoMyBuffer(unsigned char const* pImage,
                                 unsigned char StartingRow,
                                 unsigned char NumberOfRows);

void CopyColumnsIntoMyBuffer(unsigned char const* pImage,
                                    unsigned char StartingRow,
                                    unsigned char NumberOfRows,
                                    unsigned char StartingColumn,
                                    unsigned char NumberOfColumns);

#endif /* LCD_DISPLAY_H */
