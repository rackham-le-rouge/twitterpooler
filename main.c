/**
 * @file        main.c
 * @rogram      Twitter aspirator
 * @brief       Main part of the program
 * @description All the useless thing to start a program, like the administrative part used to start a compagny
 * @date        2015 bitch
 * @licence     Beerware (rev 42)
 * @author      Jerome GRARD - Not your sister....
 */


/**
 *   ERRATUM
 *
 *i.e : l_cCurrentCharacter   == l is for local (g : global and p : parameter) // c is for char (i : integer, f : float etc...)
 *                              CurrentCharacter = if you don't get it, turn off your computer. This thing isn't made for you... Go watch TV.
 */


/** To save the current PID to put it at the begin of each line */
int g_iCurrentPID;

#include "config.h"


/** Revision and version number */
char rev[] = "0.1";



/** waitBetweenTwoTurn
  * @brief In order to be like a human browsing the site, we have to add some function in order to add a random behaviour
  */
void waitBetweenTwoTurn(void)
{
    /* Firstly wait between two main page download */
    sleep(rand() % WAIT_TIME_MAX + WAIT_TIME_MIN); 

    /* TODO Make some random click, or other on the page, in order to create a real fool */
}

/** waitBetweenTwoURL
  * @brief In order to be like a human, wait between two page download, less than the time needed between two turn but still some seconds
  */
void waitBetweenTwoURL(void)
{
    /* Firstly wait between two main page download */
    sleep(rand() % WAIT_TIME_URL_MAX + WAIT_TIME_URL_MIN); 
}

/** getWindowsSize
  * @brief Get back the size of the screen in order to display a progress bar
  * @param p_iRow : row of the screen
  * @param p_iCol : column of the screen
  */
void getWindowsSize(unsigned int* p_iRow, unsigned int* p_iCol)
{
    struct winsize l_structWinSize;
    ioctl(0, TIOCGWINSZ, &l_structWinSize);

    *p_iRow = l_structWinSize.ws_row;
    *p_iCol = l_structWinSize.ws_col;
}


/** printProgressBar
  * @brief Display informations of the progression of the program's execution
  * @param p_iMax,p_iMax : max value to reach
  * @param p_iCurrentValue : current value between 0 and the max
  */
void printProgressBar(unsigned int p_iMax, unsigned int p_iCurrentValue, char* p_cCurrentToken, unsigned int p_iAllRecords)
{
   char* l_cLineBuffer;
	unsigned int l_iIterateur = 0;
	unsigned int l_iPercent = 0;
	unsigned int l_iColumn = 0;
	char l_cCaracterBody = PROGRESS_BAR_BODY_CHARACTER;
	char l_cCaracterHead = PROGRESS_BAR_HEAD_CHARACTER;
	unsigned int l_iScreenLenght = -1;
   unsigned int l_iProgressBarSize = 0;
   unsigned int l_iOffset = 0;

   getWindowsSize(&l_iColumn, &l_iScreenLenght);
   if(l_iScreenLenght < SCREEN_SIZE_COLUMN_MIN)
   {
      LOG_ERROR("Screen too small... Lenght : %d", l_iScreenLenght);
      return;
   }

   l_cLineBuffer = (char*)malloc((l_iScreenLenght + 1)*sizeof(char));
   memset(l_cLineBuffer, ' ', l_iScreenLenght + 1);

   /* in order to estimate the lenght */
   snprintf(l_cLineBuffer, l_iScreenLenght, "[%d/%d][%u]", p_iCurrentValue, p_iMax, p_iAllRecords);

   /*                                     -end and start of the bar      size and the two hooks       number and hooks */
   l_iProgressBarSize = l_iScreenLenght   - 1 - 1 -                   strlen(p_cCurrentToken) - 2 -  strlen(l_cLineBuffer); 

   /* create the line */
   snprintf(l_cLineBuffer, l_iScreenLenght, "[%d/%d][%u][%s][", p_iCurrentValue, p_iMax, p_iAllRecords, p_cCurrentToken);
	l_iPercent = (int)(((float)p_iCurrentValue /(float)p_iMax)*(float)l_iProgressBarSize);

	/* If the bar is taller than the screen */
	l_iPercent = (l_iPercent > l_iScreenLenght) ? l_iScreenLenght : l_iPercent;

	/* If lenght == 0 then we need to add 1 in order to avoid infinite loop */
	l_iPercent = (l_iPercent == 0) ? 1 : l_iPercent;

   l_iOffset = strlen(l_cLineBuffer);
	/* draw body */
	for(l_iIterateur = 0; l_iIterateur < l_iPercent - 1; l_iIterateur++)
	{
      l_cLineBuffer[l_iIterateur + l_iOffset] = l_cCaracterBody;
	}
   l_iIterateur += l_iOffset;

 	/* draw the head */
   l_cLineBuffer[l_iIterateur++] = l_cCaracterHead;
    for( ; l_iIterateur < l_iScreenLenght - 1 ; l_iIterateur++)
   {
      l_cLineBuffer[l_iIterateur] = ' ';
   }

   l_cLineBuffer[l_iIterateur++] = ']';
   l_cLineBuffer[l_iIterateur] = '\0';

   fprintf(stdout, "%s\r", l_cLineBuffer);
   fflush(stdout);

   free(l_cLineBuffer);
}



/** Main
  * @brief Main function of the program, this is the starting point
  * @param argc : number of parameters gived to the program
  * @param argv : 2D array to store all the parameters gived to the program
  */
int main(int argc, char** argv)
{
    UNUSED(argc);
    UNUSED(argv);

    srand(time(NULL));

    LOG_PRINT("\n"
"      _/_/_/   _/_/    _/_/_/    _/     _/   _/_/_     _/_/_/    _/_/   \n"
"       _/   _/   _/   _/    _/  _/_/   _/  _/    _/  _/    _/  _/   _/  \n"
"      _/   _/    _/  _/    _/  _/ _/  _/  _/    _/  _/    _/  _/    _/  \n"
"     _/   _/    _/  _/_/_/    _/  _/ _/  _/_/_/_/  _/    _/  _/    _/   \n"
"    _/   _/    _/  _/ _/     _/   _/_/  _/    _/  _/    _/  _/    _/    \n"
"   _/   _/    _/  _/   _/   _/    _//  _/    _/  _/    _/  _/    _/     \n"
"  _/     _/_/    _/     _/ _/     _/  _/    _/  _/_/_/      _/_/        \n"
"  \n"
"                                 Rev %s\n", rev);


    networkLoop();

    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;	

}
