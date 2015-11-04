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
void rtrim(char* p_sString);
linkedListKeywords* getKeywords(char* p_sKeywords);
void cleanKeywords(linkedListKeywords* p_structKeywords);
int detectKeyWordInAString(char* p_sKeyWord, char* p_sQuote);
int keyWordsDetection(char* p_sQuote, linkedListKeywords* p_structKeywords);
char* extractWordLevel(char* p_cSpecialWordLevel, int* p_iWordLevel, char* p_sKeyWord, int* p_iWordLenght);
void removeUselessSpaces(char* p_sString);
void toLowerCase(char* p_sString);
void removeUnwantedCharacters(char* p_sString);

#endif
