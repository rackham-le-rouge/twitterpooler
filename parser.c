/** 
 * @file        parser.c
 * @rogram      Twitter aspirator
 * @brief       Functions able to extract usefull informations from the downloaded html page
 * @description In order to parse downloaded data of the file
 * @date        2015 bitch
 * @licence     Beerware (rev 42)
 * @author      Jerome GRARD - Not your sister....
 */

#include "config.h"



/**
 * @brief This function remove all &#XXX html code from the provided string and replace them
 * by the correct character. Modify the provided string
 * @param p_sString : string to analyse / modify
 */
void convertHTML2ASCII(char* p_sString)
{
    char l_sFinal[MAX_CONFIG_LINE_LEN];
    int l_iCursorI;
    int l_iCursorF;
    int l_iCursorC;
    int l_iLen;

    bzero(l_sFinal, MAX_CONFIG_LINE_LEN);       /* FIXME */
    l_iLen = strlen(p_sString);
    l_iCursorI = 0;
    l_iCursorF = 0;
    l_iCursorC = 0;

    while(l_iCursorI < l_iLen)
    {
        if(p_sString[l_iCursorI] == '&' && (l_iCursorI + 1) < l_iLen)
        {
            if(p_sString[l_iCursorI + 1] == '#')
            {
                l_iCursorI += 2;
                l_sFinal[l_iCursorF] = (char)atoi(p_sString + l_iCursorI);

                while((p_sString[l_iCursorI] >= '0' && p_sString[l_iCursorI] <= '9') || 
                      p_sString[l_iCursorI] == ';')
                {
                    l_iCursorI++;
                }
            }
            else
            {
                l_sFinal[l_iCursorF] = p_sString[l_iCursorI];
                l_iCursorI++;
            }
        }
        else
        {
            l_sFinal[l_iCursorF] = p_sString[l_iCursorI];
            l_iCursorI++;
        }
        l_iCursorF++;
    }

    l_iCursorC = l_iCursorF;
    while(l_iCursorC < l_iCursorI)
    {
        p_sString[l_iCursorC++] = ' ';
    }

    memcpy(p_sString, l_sFinal, l_iCursorF);
}


/**
 * @brief Remove all data between <> because it was just technical things in our case
 * @param p_sString : the string to analyse and give back. This string is filled with space
 * on the right side
 */
void removeHTMLContent(char* p_sString)
{
    char l_sFinal[MAX_CONFIG_LINE_LEN];
    int l_iCursorI;
    int l_iCursorF;
    int l_iCursorC;
    int l_iLen;
    int l_iLevel;

    bzero(l_sFinal, MAX_CONFIG_LINE_LEN);       /* FIXME adjust to the needed size */
    l_iLen = strlen(p_sString);
    l_iCursorI = 0;
    l_iCursorF = 0;
    l_iCursorC = 0;
    l_iLevel = -1;

    while(l_iCursorI < l_iLen)
    {
        if(p_sString[l_iCursorI] == '<')
        {
            while(l_iLevel > 0 || l_iLevel == -1)
            {
                if(l_iCursorI >= l_iLen)
                {
                    break;
                }
                if(l_iLevel == -1)
                {
                    /* special case : init. if we are here it is because we've already met a < */
                    l_iLevel = 0;
                }
                if(p_sString[l_iCursorI] == '<')
                {
                    l_iLevel++;
                }
                if(p_sString[l_iCursorI] == '>')
                {
                    l_iLevel--;
                }
                l_iCursorI++;
            }
            l_iLevel = -1;
        }
        else
        {
            l_sFinal[l_iCursorF] = p_sString[l_iCursorI];
            l_iCursorI++;
            l_iCursorF++;
        }
    }

    l_iCursorC = l_iCursorF;
    while(l_iCursorC < l_iCursorI)
    {
        p_sString[l_iCursorC++] = ' ';
    }

    memcpy(p_sString, l_sFinal, l_iCursorF);
}
