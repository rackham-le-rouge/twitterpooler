/** 
 * @file        parser.h
 * @rogram      Twitter aspirator
 * @brief       Prototypes of functions able to extract usefull informations from the downloaded html page
 * @description In order to parse downloaded data of the file
 * @date        2015 bitch
 * @licence     Beerware (rev 42)
 * @author      Jerome GRARD - Not your sister....
 */



#ifndef PARSER_H
#define PARSER_H


/*

	Header of the main file

*/

void convertHTML2ASCII(char* p_sString);
void removeHTMLContent(char* p_sString);

#endif
