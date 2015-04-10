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
/// For an UNO, 2 is the maximum number possible.
#define POT_COUNT 2

/// Keeps track of number of seconds since each pot was brewed.
int gTimeSinceBrew[POT_COUNT] = { 0 };

/// Time, in seconds, beyond which a pot is considered old.
#define BREW_TIME_LIMIT 4*60*60

/// Digital inputs (w/pullups) 
int gBrewResetPin[POT_COUNT] = { 0, 1 };

/// Pins for RGB LEDs
struct RGB 
{
   byte red;
   byte green;
   byte blue;
};
RGB gLEDPin[POT_COUNT] = {
   { 9, 10, 11 },
   { 3, 5, 6 }
};

/// Defines the layout of the LCD's character cells.
#define LCD_ROWS 2
#define LCD_COLS 16 

const char* GreetingString[LCD_ROWS] = {
"Life's too short",
"for old coffee!"
};

/// A buffer used for conveniently writing spaces to an entire line.
char gPad[LCD_COLS+1];

/// This holds the text that is sent to the display.
char gDisplayText[LCD_ROWS][LCD_COLS+1] = {0};

/// Pins for Hitachi LCD; any of the Arduino digital pins should work
// interchangeably for any of these LCD connections.  I chose these to
// simplify the PCB layout.
int gLCD_RS_Pin = 12;
int gLCD_E_Pin = 13;
int gLCD_D4_Pin = 8;
int gLCD_D5_Pin = 7;
int gLCD_D6_Pin = 4;
int gLCD_D7_Pin = 2;


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
      delay(100);
   }
}



// led is index into gLEDPin array; color components in the range [0..255]
void set_color(int led, int red, int green, int blue)
{
#ifdef COMMON_ANODE
   red = 255 - red;
   green = 255 - green;
   blue = 255 - blue;
#endif
   analogWrite(gLEDPin[led].red, red);
   analogWrite(gLEDPin[led].green, green);
   analogWrite(gLEDPin[led].blue, blue);
}



/// Guaranteed to consume no more than 7 positions in gDisplayText.
int seconds_to_time_str( int seconds, int row, int start_index, int limit )
{
   if ( start_index >= limit )
      return start_index;

   if ( seconds < 0 )
   {
      gDisplayText[row][start_index++] = '?';
      return start_index;
   }

   if ( seconds > BREW_TIME_LIMIT )
   {
      gDisplayText[row][start_index++] = 'O';
      if ( start_index < limit ) gDisplayText[row][start_index++] = 'L';
      if ( start_index < limit ) gDisplayText[row][start_index++] = 'D';
      return start_index;
   }
   
#if (BREW_TIME_LIMIT >= 10*60*60)
#error This code assumes a brew time limit of less than 10 hours.
#endif
   int hour( seconds / 60 / 60 );
   int minute( ( seconds - hour * 60 * 60 ) / 60 );
   int second( seconds % 60 );

   gDisplayText[row][start_index++] = '0'+hour;
   if ( start_index < limit ) gDisplayText[row][start_index++] = ':';
   if ( start_index < limit ) gDisplayText[row][start_index++] = '0'+minute/10;
   if ( start_index < limit ) gDisplayText[row][start_index++] = '0'+minute%10;
   if ( start_index < limit ) gDisplayText[row][start_index++] = ':';
   if ( start_index < limit ) gDisplayText[row][start_index++] = '0'+second/10;
   if ( start_index < limit ) gDisplayText[row][start_index++] = '0'+second%10;

   return start_index;
}



#if (POT_COUNT == 2)
void update_display()
{
   int i;
#if (LCD_ROWS == 1)
#if (LCD_COLS<16)
#error Display too small for information to be displayed.
#endif
   // Need to squeeze everything on one line
   i = 0;
   gDisplayText[0][i++] = 'A';
   gDisplayText[0][i++] = ' ';
   i = seconds_to_time_str( gTimeSinceBrew[0], 0, i, LCD_COLS );
   gDisplayText[0][i++] = ' ';
   gDisplayText[0][i++] = 'B';
   gDisplayText[0][i++] = ' ';
   i = seconds_to_time_str( gTimeSinceBrew[1], 1, i, LCD_COLS );
   for ( ; i <= LCD_COLS; ++i )
      gDisplayText[0][i] = 0;
#else
#if (LCD_COLS<13)
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
   i = seconds_to_time_str( gTimeSinceBrew[0], 0, i, LCD_COLS );
   for ( ; i <= LCD_COLS; ++i )
      gDisplayText[0][i] = 0;
   i = 0;
   gDisplayText[1][i++] = 'P';
   gDisplayText[1][i++] = 'o';
   gDisplayText[1][i++] = 't';
   gDisplayText[1][i++] = ' ';
   gDisplayText[1][i++] = 'B';
   gDisplayText[1][i++] = ' ';
   i = seconds_to_time_str( gTimeSinceBrew[1], 1, i, LCD_COLS );
   for ( ; i <= LCD_COLS; ++i )
      gDisplayText[1][i] = 0;
#endif
   display_lines(false);

   for ( i = 0; i < POT_COUNT; ++i )
   {
      if ( gTimeSinceBrew[i] < 0 )
         set_color( i, 0, 0, 0 );
      else if ( gTimeSinceBrew[i] > BREW_TIME_LIMIT )
         set_color( i, 255, 0, 0 );
      else
      {
         unsigned long val = 255 * (BREW_TIME_LIMIT - gTimeSinceBrew[i]) / BREW_TIME_LIMIT;
         if ( val > 255 )
            val = 255;
         set_color( i, 255-val, val, 0 );
      }
   }
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
      pinMode( gLEDPin[i].red, OUTPUT );
      pinMode( gLEDPin[i].green, OUTPUT );
      pinMode( gLEDPin[i].blue, OUTPUT );
      set_color( i, 127, 127, 255 );
   }

   // Display an initial message on all rows
   for ( int i = 0; i < LCD_ROWS; ++i )
   {
      scroll_line( GreetingString[i], i );
      for ( int j = 0 ; j < POT_COUNT; ++j )
         set_color( j, 63, 63, 127 );
   }
   delay(500);
   for ( int i = 0 ; i < POT_COUNT; ++i )
      set_color( i, 0, 0, 0 );

   update_display();
}


unsigned long gLastTime = 0;

/// The main loop.  Release the hounds!
void loop()
{
   unsigned long time = millis();
   bool do_update = false;

   if ( time < gLastTime )
   {
      // Deal with rollover
      time = gLastTime - time;
      gLastTime = 0;
   }

   // See if a second's gone by
   if ( time - gLastTime > 1000 )
   {
      gLastTime = time;
      for ( int i = 0; i < POT_COUNT; ++i )
      {
         if ( gTimeSinceBrew[i] >= 0 )
            ++gTimeSinceBrew[i];
      }
      do_update = true;
   }

   // Check for resets
   for ( int i = 0; i < POT_COUNT; ++i )
   {
      if ( digitalRead( gBrewResetPin[i] ) == LOW )
      {
         gTimeSinceBrew[i] = 0;
         do_update = true;
      }
   }

   if ( do_update )
      update_display();

}


