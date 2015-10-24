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
    char* l_sFinal;
    int l_iCursorI;
    int l_iCursorF;
    int l_iCursorC;
    int l_iLen;

    l_iLen = strlen(p_sString);
    l_sFinal = (char*)malloc(l_iLen * sizeof(char));
    if(l_sFinal == NULL)
    {
        return;
    }
    bzero(l_sFinal, l_iLen);
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
    free(l_sFinal);
}


/**
 * @brief Remove all data between <> because it was just technical things in our case
 * @param p_sString : the string to analyse and give back. This string is filled with space
 * on the right side
 */
void removeHTMLContent(char* p_sString)
{
    char* l_sFinal;
    int l_iCursorI;
    int l_iCursorF;
    int l_iCursorC;
    int l_iLen;
    int l_iLevel;

    l_iLen = strlen(p_sString);
    l_sFinal = (char*)malloc(l_iLen * sizeof(char));
    if(l_sFinal == NULL)
    {
        return;
    }
    bzero(l_sFinal, l_iLen);
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
    free(l_sFinal);
}

/**
 * @brief Function to remove all spaces on the right
 * @param p_sString : string to trim
 */
void rtrim(char* p_sString)
{
    char* l_cCursor;

    l_cCursor = p_sString + strlen(p_sString) - 1;
    while(*l_cCursor-- == ' ');

    *(l_cCursor + 2) = '\0';
}



/**
 * @brief This function analyse keywords string gived as an arg, parse it, create the linked
 * list and fill it with keywords. One keyword per structure. This function returns the
 * the anchor, in the anchor there is the first keyword and a pointer to the next structure.
 * The last struture is the one with the 'next' member pointing on NULL.
 * @param p_sKeywords : pkeyword strings retrieved from the configuration file. Something like "keyword1;keyword2;keyword3;\n"
 * @return A pointer on the first structure, this structure holds the first keyword. Or NULL if there was no keywords in the p_sParameters strings.
 */
linkedListKeywords* getKeywords(char* p_sKeywords)
{
    char* l_cCursor;
    char* l_cBeginingOfTheWord;
    int l_iWordLenght;
    linkedListKeywords* l_structAnchor;
    linkedListKeywords* l_structCurrent;
    linkedListKeywords* l_structPrevious;

    l_iWordLenght = 0;
    l_cCursor = p_sKeywords;
    l_cBeginingOfTheWord = p_sKeywords;
    l_structAnchor = NULL;
    l_structCurrent = NULL;
    l_structPrevious = NULL;

    while(*l_cCursor != '\0')
    {
        /* compute the lenght of the word in order to malloc just the right size */
        while(*l_cCursor != ';')
        {
            l_iWordLenght++;
            l_cCursor++;
        }

        if(l_structPrevious != NULL)
        {
            /* We are on a turn != 0 */
            l_structPrevious->structNext = (linkedListKeywords*)malloc(sizeof(linkedListKeywords));
            l_structCurrent = l_structPrevious->structNext;
        }
        else
        {
            /* We are on the first turn */
            l_structCurrent = (linkedListKeywords*)malloc(sizeof(linkedListKeywords));
        }
        l_structCurrent->structNext = NULL;
        l_structCurrent->sKeyword = (char*)malloc((l_iWordLenght + 1) * sizeof(char));
        memcpy(l_structCurrent->sKeyword, l_cBeginingOfTheWord, l_iWordLenght);
        *(l_structCurrent->sKeyword + l_iWordLenght) = '\0';

        if(l_structAnchor == NULL)
        {
            /* We are on the first turn, the anchor */
            l_structAnchor = l_structCurrent;
        }
        l_structPrevious = l_structCurrent;

        /* Handle the end of the string, reset flags, prepare the next detection */
        l_cBeginingOfTheWord += (l_iWordLenght + 1);
        l_iWordLenght = 0;
        l_cCursor++;

        LOG_INFO("Parameter [%s]", l_structCurrent->sKeyword);
    }
    return l_structAnchor;
}



/**
 * @brief This function free() all content of the linked list of the parameters. Provide the
 * anchor and this function recursivelly clean all content until the end of the list.
 * @param p_structKeywords : the anchor of the linked list
 */
void cleanKeywords(linkedListKeywords* p_structKeywords)
{
    unsigned int l_iCounter;
    linkedListKeywords* l_structCurrent;
    linkedListKeywords* l_structNext;

    l_iCounter = 0;
    l_structCurrent = p_structKeywords;
    while(l_structCurrent != NULL)
    {
        l_structNext = l_structCurrent->structNext;
        free(l_structCurrent->sKeyword);
        free(l_structCurrent);
        l_structCurrent = l_structNext;
        l_iCounter++;
    }
    LOG_INFO("Free %d level in the linked list.", l_iCounter);
}

