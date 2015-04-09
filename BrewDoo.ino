/*
    BrewDoo: Displays count-up timers on a Hitachi HD44780 LCD for use as a
    way of keeping track of when pots of coffee were last brewed.  Whenever
    the 'brew' button is pressed, the timer associated with that button is
    reset.  So, really, it can be used to time anything.
    
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

/// Time, in minutes, beyond which a pot is considered old.
#define BREW_TIME_LIMIT 4*60 

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



/// Guaranteed to consume no more than 4 positions in gDisplayText.
int minutes_to_time_str( int minutes, int row, int start_index, int limit )
{
   if ( start_index >= limit )
      return start_index;

   if ( minutes < 0 )
   {
      gDisplayText[row][start_index++] = '?';
      return start_index;
   }

   if ( minutes > BREW_TIME_LIMIT )
   {
      gDisplayText[row][start_index++] = 'O';
      if ( start_index < limit ) gDisplayText[row][start_index++] = 'L';
      if ( start_index < limit ) gDisplayText[row][start_index++] = 'D';
      return start_index;
   }
   
#if (BREW_TIME_LIMIT >= 10*60)
#error This code assumes a brew time limit of less than 10 hours.
#endif
   int hour( minutes / 60 );
   int minute( minutes % 60 );

   gDisplayText[row][start_index++] = '0'+hour;
   if ( start_index < limit ) gDisplayText[row][start_index++] = ':';
   if ( start_index < limit ) gDisplayText[row][start_index++] = '0'+minute/10;
   if ( start_index < limit ) gDisplayText[row][start_index++] = '0'+minute%10;

   return start_index;
}



#if (POT_COUNT == 2)
void update_display()
{
   int i;
#if (LCD_ROWS == 1)
#if (LCD_COLS<14)
#error Display too small for information to be displayed.
#endif
   // Need to squeeze everything on one line
   i = 0;
   gDisplayText[0][i++] = 'A';
   gDisplayText[0][i++] = ' ';
   i = minutes_to_time_str( gTimeSinceBrew[0], 0, i, LCD_COLS );
   gDisplayText[0][i++] = ' ';
   gDisplayText[0][i++] = ' ';
   gDisplayText[0][i++] = 'B';
   gDisplayText[0][i++] = ' ';
   i = minutes_to_time_str( gTimeSinceBrew[1], 1, i, LCD_COLS );
   for ( ; i <= LCD_COLS; ++i )
      gDisplayText[0][i] = 0;
#else
#if (LCD_COLS<10)
#error Display too small for information to be displayed.
#endif
   // One line per pot
   i = 0;
   gDisplayText[0][i++] = 'P';
   gDisplayText[0][i++] = 'o';
   gDisplayText[0][i++] = 't';
   gDisplayText[0][i++] = ' ';
   gDisplayText[0][i++] = 'A';
   gDisplayText[0][i++] = ' ';
   i = minutes_to_time_str( gTimeSinceBrew[0], 0, i, LCD_COLS );
   for ( ; i <= LCD_COLS; ++i )
      gDisplayText[0][i] = 0;
   i = 0;
   gDisplayText[1][i++] = 'P';
   gDisplayText[1][i++] = 'o';
   gDisplayText[1][i++] = 't';
   gDisplayText[1][i++] = ' ';
   gDisplayText[1][i++] = 'B';
   gDisplayText[1][i++] = ' ';
   i = minutes_to_time_str( gTimeSinceBrew[1], 1, i, LCD_COLS );
   for ( ; i <= LCD_COLS; ++i )
      gDisplayText[1][i] = 0;
#endif
   display_lines(false);
}
#else
#error Need to implement update_display() for POT_COUNT value
#endif

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

      update_display();
   }

   // Check for resets
   for ( int i = 0; i < POT_COUNT; ++i )
   {
      if ( digitalRead( gBrewResetPin[i] ) == LOW )
         gTimeSinceBrew[i] = 0;
   }

}


