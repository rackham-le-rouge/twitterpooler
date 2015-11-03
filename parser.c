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
 * @brief Remove all useless spaces by compressiing sentence on the left. Leave spaces on the right side. Use rtim() after
 * @param p_sString : the sentence to clean
 * @return p_sString : the sentence is modified directlly at its own memory address
 */
void removeUselessSpaces(char* p_sString)
{
    char* l_cCursor;
    char* l_cCursorDetached;

    /* the detached cursor is on the old version of the sentence -exploring-, and the cursor is
     * on the new version. If there is no useless spaces, this two cursors will be equals */
    l_cCursor = p_sString;
    l_cCursorDetached = p_sString;

    while(*l_cCursorDetached != '\0')
    {
        if(*l_cCursorDetached == ' ')
        {
            *l_cCursor = *l_cCursorDetached;
            l_cCursor++;
            l_cCursorDetached++;

            while(*l_cCursorDetached == ' ' && *l_cCursorDetached != '\0')
            {
                l_cCursorDetached++;
            }
        }
        else
        {
            *l_cCursor = *l_cCursorDetached;
            l_cCursor++;
            l_cCursorDetached++;
        }
    }
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
    int l_iWordLevel;   /* Define the word position in the wanted sentence according to our algo */
    char l_cSpecialWordLevel;   /* For word level < 0 we refer to this value to define a special behavior */
    linkedListKeywords* l_structAnchor;
    linkedListKeywords* l_structCurrent;
    linkedListKeywords* l_structPrevious;

    l_iWordLevel = 0;
    l_cSpecialWordLevel = 'A';
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

        /* Fill this section of the linked list */
        l_structCurrent->structNext = NULL;
        l_structCurrent->sKeyword = (char*)malloc((l_iWordLenght + 1) * sizeof(char));
        /* this function have to re-cut the keyword and find word level and special attribute of this keyword */
        l_cBeginingOfTheWord = extractWordLevel(&l_cSpecialWordLevel, &l_iWordLevel, l_cBeginingOfTheWord, &l_iWordLenght);

        memcpy(l_structCurrent->sKeyword, l_cBeginingOfTheWord, l_iWordLenght);
        *(l_structCurrent->sKeyword + l_iWordLenght) = '\0';
        l_structCurrent->cSpecialWordLevel = l_cSpecialWordLevel;
        l_structCurrent->iWordLevel = l_iWordLevel;

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

        LOG_INFO("Parameter [%s] WordLevel %d:%c", l_structCurrent->sKeyword, l_structCurrent->iWordLevel, l_structCurrent->cSpecialWordLevel);
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




/**
 * @brief This function parse the p_sQuote in order to find at least one of the keyword holded in
 * the linked list p_structKeywords. This function get all keywords in the linked list and try to
 * find them (even with an approx spelling)
 * @param p_sQuote : the sentence to analyse
 * @param p_structKeywords : the linked list with all words to find. Have to be created by getKeywords()
 * @return EXIT_SUCCESS if at least one ketword is find the sentence p_sQuote even if it was with an approx spelling. On the other case EXIT_FAILURE
 */
int keyWordsDetection(char* p_sQuote, linkedListKeywords* p_structKeywords)
{
    linkedListKeywords* l_structCurrent;
    int l_iRetCode = EXIT_FAILURE;

    l_structCurrent = p_structKeywords;

    l_structCurrent = p_structKeywords;

    /* Analyse the whole linked list FIXME basic way to do it. Have to be musch smarter, analyse words in context  and links with other words of the list */
    while(l_structCurrent != NULL)
    {
        if(detectKeyWordInAString(l_structCurrent->sKeyword, p_sQuote) == EXIT_SUCCESS)      /* FIXME too basic way to treat results */
        {
            l_iRetCode = EXIT_SUCCESS;
        }
        l_structCurrent = l_structCurrent->structNext;
    }

    return l_iRetCode;
}



/**
 * @brief Function to find a word in the quote. able to find even a word with not exact match.
 * This function have to be improved a lot in order to find all interesting data
 * @param p_sKeyWord : the wanted word to find
 * @param p_sQuote : the string to parse
 * @return EXIT_SUCCESS or EXIT_FAILURE if the word (even with an approx typo) is or isn't found
 */
int detectKeyWordInAString(char* p_sKeyWord, char* p_sQuote)
{
    int l_iLenQuote;
    int l_iLenKeyWord;
    int l_iCursorQuote;
    int l_iCursorKeyWord;
    int l_iCorrelationDegree;
    int l_iRetCode;

    l_iLenQuote = strlen(p_sQuote);
    l_iLenKeyWord = strlen(p_sKeyWord);
    l_iCursorQuote = 0;
    l_iCursorKeyWord = 0;
    l_iCorrelationDegree = 0;
    l_iRetCode = EXIT_FAILURE;

    while(l_iCursorQuote < l_iLenQuote)
    {
        while(l_iCursorKeyWord < l_iLenKeyWord && (l_iCursorQuote + l_iCursorKeyWord) < l_iLenQuote)
        {
            if(p_sQuote[l_iCursorQuote + l_iCursorKeyWord] == p_sKeyWord[l_iCursorKeyWord])
            {
                l_iCorrelationDegree++;
            }
            l_iCursorKeyWord++;
        }

        if(l_iCorrelationDegree > (int)((float)l_iLenKeyWord) * WORD_CORRELATION)
        {
            l_iRetCode = EXIT_SUCCESS;
        }

        l_iCursorKeyWord = 0;
        l_iCorrelationDegree = 0;
        l_iCursorQuote++;
    }
    return l_iRetCode;
}


/** @brief Function to extract the WordLevel -the importance & position- of a keyword in the wanted sentence
 * @param p_cSpecialWordLevel : if it is a special keyword, its wordlevel is a letter - please refer to the doc of the semantic algo
 * @param p_iWordLevel : conventionnal numeric word level, define order of appearance
 * @param p_sKeyWord : the keyword to analyse. Could be "X:keyword" or "Y:keyword" or "234:keyword" or even "keyword" (without any order specified) etc...
 * @return a pointer on the real begining of the keyword
 */
char* extractWordLevel(char* p_cSpecialWordLevel, int* p_iWordLevel, char* p_sKeyWord, int* p_iWordLenght)
{
    int l_iCursor;
    char* l_cBeginingOfTheWord;

    l_cBeginingOfTheWord = p_sKeyWord;
    l_iCursor = 0;

    while(l_iCursor < *p_iWordLenght)
    {
        if(*(p_sKeyWord + l_iCursor) == ':')
        {
            l_cBeginingOfTheWord += (l_iCursor + 1);

            if(*(p_sKeyWord + l_iCursor - 1) >= 'A' && *(p_sKeyWord + l_iCursor - 1) <= 'Z')
            {
                *p_iWordLevel = -1;
                *p_cSpecialWordLevel = *(p_sKeyWord + l_iCursor - 1);
            }
            else
            {
                *p_iWordLevel = atoi(p_sKeyWord);
                *p_cSpecialWordLevel = 'A';
            }
        }

        l_iCursor++;
    }

    *p_iWordLenght = *p_iWordLenght - (l_cBeginingOfTheWord - p_sKeyWord);
    return l_cBeginingOfTheWord;
}
