/*
    BrewDoo: Displays count-up timers on a Hitachi HD44780 LCD for use as a
    way of keeping track of when pots of coffee were last brewed.  Whenever
    the 'brew' button is pressed, the timer associated with that button is
    reset.
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   
    Copyright 2015 Paul Kerchen
*/

#include <LiquidCrystal.h>

const char* VersionString = "BrewDoo 1.0";

/// Defines number of timers (pots of coffee) to keep track of.
#define POT_COUNT 2

/// Keeps track of number of minutes since each pot was brewed.
int gTimeSinceBrew[POT_COUNT] = { 0 };

/// Digital inputs (w/pullups) 
int gBrewResetPin[POT_COUNT] = { 14, 15 };  /// TODO: Figure out real pins for reset buttons

/// Defines the layout of the LCD's character cells.
#define LCD_ROWS 2
#define LCD_COLS 16

/// A buffer used for conveniently writing spaces to an entire line.
char gPad[LCD_COLS+1];

/// This holds the text that is sent to the display.
char gDisplayText[LCD_ROWS][LCD_COLS+1] = {0};

/// Pins for Hitachi LCD; any of the Arduino digital pins should work
// interchangeably for any of these LCD connections.  I chose these to
// simplify the PCB layout.
int gLCD_RS_Pin = 12;
int gLCD_E_Pin = 11;
int gLCD_D4_Pin = 8;
int gLCD_D5_Pin = 7;
int gLCD_D6_Pin = 6;
int gLCD_D7_Pin = 5;


/// Initialize the LCD display with correct pin assignments.
LiquidCrystal gLCDDisplay(gLCD_RS_Pin, gLCD_E_Pin, gLCD_D4_Pin, gLCD_D5_Pin, gLCD_D6_Pin, gLCD_D7_Pin);


// A crude string length function.
int strlen( char* line )
{
   int cnt=0;

   while( line[cnt] != 0 )
      cnt++;
   
   return cnt;
}



void display_lines( bool center )
{
   for ( int i = 0; i < LCD_ROWS; ++i )
   {
      gLCDDisplay.setCursor(0, i);
      gLCDDisplay.print(gPad);
      if ( center )
      {
         int offset = (LCD_COLS - strlen(gDisplayText[i]))/2;
         gLCDDisplay.setCursor( offset, i );
      }
      else
      {
         gLCDDisplay.setCursor(0, i);
      }
      gLCDDisplay.print(gDisplayText[i]);
   }
}


void scroll_line( const char* line, int row )
{
   // TODO: Use library scrolling functionality?
   for ( int i = 0; i < LCD_COLS; ++i )
   {
      gLCDDisplay.setCursor( 0, row );
      gLCDDisplay.print(gPad);
      gLCDDisplay.setCursor( LCD_COLS-i-1, row );
      gLCDDisplay.print(line);
      delay(300);
   }
}



/// One-time setup before entering the main loop.
void setup() 
{
   // Set up the LCD's number of columns and rows.
   gLCDDisplay.begin(LCD_COLS, LCD_ROWS);

   // Init our padding string
   for ( int i = 0; i < LCD_COLS; ++i )
      gPad[i] = ' ';
   gPad[LCD_COLS] = 0;

   for ( int i = 0 ; i < POT_COUNT; ++i )
   {
      gTimeSinceBrew[i] = -1;
      pinMode( gBrewResetPin[i], INPUT_PULLUP );
   }

   // Display an initial message on all rows
   for ( int i = 0; i < LCD_ROWS; ++i )
   {
      scroll_line( VersionString, i );
   }
}


unsigned long gLastTime = 0;

/// The main loop.  Release the hounds!
void loop()
{
   unsigned long time = millis();

   if ( time < gLastTime )
   {
      // Deal with rollover
      time = gLastTime - time;
      gLastTime = 0;
   }

   // See if a minute's gone by
   if ( time - gLastTime > 1000 * 60 )
   {
      gLastTime = time;
      for ( int i = 0; i < POT_COUNT; ++i )
      {
         ++gTimeSinceBrew[i];
      }
      // Update the display
      //for ( i = 0; i < gIncomingIndex && i < LCD_COLS; ++i )
      //{
         //gDisplayText[gCurrentRow][i] = gIncomingBuffer[i];
      //}
      //for ( ; i < LCD_COLS; ++i )
         //gDisplayText[gCurrentRow][i] = 0;

      //display_lines(true);
   }

   // Check for resets
   for ( int i = 0; i < POT_COUNT; ++i )
   {
      if ( digitalRead( gBrewResetPin[i] ) == LOW )
         gTimeSinceBrew[i] = 0;
   }

}


