/**
 * @file        io.c
 * @program     Twitter pooler to grab crap
 * @brief       IO header, for read/write on files
 * @description All functions to read write on the program configuration files
 * @date        2015 bitch
 * @licence     Beerware (rev 42)
 * @author      Jerome GRARD - Not your sister....
 */


#include "config.h"


/** @brief
 * checkIfAFileExist : check existance of a file by trying to opening it
 * @param p_cFileName : file to test
 * @return EEXIST if there is a success or -EEXIST in the other case
 */
int checkIfAFileExist(const char* p_cFileName)
{
    FILE* l_fileFile = NULL;

    if(p_cFileName == NULL)
    {
        return -EEXIST;
    }

    l_fileFile = fopen(p_cFileName, "r");
    if(l_fileFile == NULL)
    {
        return -EEXIST;
    }

    fclose(l_fileFile);

    return EEXIST;
}



/** @brief
 * Check if you can read & write the file provided as a parameter by trying
 * to fopen it with w+ mode
 * @param p_cFileName : name of the file to test
 * @return EXIT_SUCCESS if success or EXIT_FAILURE in the other cases.
 */
int checkReadWriteFile(const char* p_cFileName)
{
    FILE* l_fileFile = NULL;

    if(p_cFileName == NULL)
    {
        return EXIT_FAILURE;
    }

    l_fileFile = fopen(p_cFileName, "w+");
    if(l_fileFile == NULL)
    {
        return EXIT_FAILURE;
    }

    fclose(l_fileFile);

    return EXIT_SUCCESS;
}



/** @brief createDirectory
 * Create a directory
 * @param p_cName : directory' name
 * @return EXIT_SUCCESS if success or EXIT_FAILURE in the other cases.
 */
int createDirectory(const char* p_cName)
{
    LOG_INFO("Going to create directory %s", p_cName);
    return mkdir(p_cName, 0755);
}


/** @brief
 * checkConfigurationFiles is the main function to tests all needed files.
 * that's here we proceed to all calls for checks existance or rights on the needed files.
 * @return If we have all config files and no issues with IO permission returns how many compagnies hold in the configuration file. -EXIT_FAILURE or a negative value if there is any error
 */
int checkConfigurationFiles(void)
{
    FILE* l_fileConfigurationFile = NULL;
    FILE* l_fileEmptyChecksumFile = NULL;
    char l_cCharacter = 0;
    char l_sLine[MAX_CONFIG_LINE_LEN];
    char l_sFileWithCompagnyChecksums[MAX_CONFIG_LINE_LEN];
    unsigned int l_iCursor = 0;
    int l_iCompagnyCounter = 0;

    /* Doesn't care about the ret code, because EEXIST seems to be K.O, this 
     * function always send back -1 instead of a smarter code */
    createDirectory(CHECKSUM_DIRECTORY);

    if(checkIfAFileExist(CONFIGURATION_FILE) != EEXIST)
    {
        LOG_ERROR("configuration file %s missing", CONFIGURATION_FILE);
        return -EXIT_FAILURE;
    }

    l_fileConfigurationFile = fopen(CONFIGURATION_FILE, "r");
    if(l_fileConfigurationFile == NULL)
    {
        return -EXIT_FAILURE;
    }

    while(l_cCharacter != EOF)
    {
        l_cCharacter = fgetc(l_fileConfigurationFile);

        /* End of line - Check is we take a compagny name or a keyword */
        if(l_cCharacter == '\n' || l_iCursor > MAX_CONFIG_LINE_LEN - 1)
        {
            if(l_sLine[l_iCursor - 1] == ':')
            {
                /* Compagny name on the line */

                /* Remove the 'compagny' marker. It was arbitrary decided */
                l_sLine[l_iCursor - 1] = '\0';
                LOG_INFO("Checking compagny %s", l_sLine);
                l_iCompagnyCounter++;

                bzero(l_sFileWithCompagnyChecksums, MAX_CONFIG_LINE_LEN);
                snprintf(l_sFileWithCompagnyChecksums,
                        MAX_CONFIG_LINE_LEN,
                        "%s/%s.md5",
                        CHECKSUM_DIRECTORY,
                        l_sLine);

                if(checkIfAFileExist(l_sFileWithCompagnyChecksums) != EEXIST)
                {
                    /* Compgany have no checksum file associated */
                    LOG_WARNING("Compagny %s have no checksum file. First init. Create one.", l_sLine);
                    l_fileEmptyChecksumFile = fopen(l_sFileWithCompagnyChecksums, "w");
                    if(l_fileEmptyChecksumFile == NULL)
                    {
                        LOG_ERROR("Impossible to create %s", l_sFileWithCompagnyChecksums);
                        fclose(l_fileConfigurationFile);
                        return -EXIT_FAILURE;
                    }

                    /* FIXME create here a true fake line */
                    fprintf(l_fileEmptyChecksumFile, "000\n");

                    fclose(l_fileEmptyChecksumFile);
                }
            }
            else
            {
                /* Key words - We don't care here */
            }
            bzero(l_sLine, strlen(l_sLine));
            l_iCursor = 0;
        }
        else
        {
            l_sLine[l_iCursor++] = l_cCharacter;
        }
    }

    fclose(l_fileConfigurationFile);

    return l_iCompagnyCounter;
}


/**
 * @brief Ths function initialize the way we have to output usefull informations grabbed by this program
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int initExternalCommunication(void)
{
    int l_iReturnedValue = 0;

    l_iReturnedValue = (mkfifo(PIPE_NAME, S_IRUSR | S_IWUSR) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

    if(l_iReturnedValue == EXIT_FAILURE && errno == EEXIST)
    {
        l_iReturnedValue = EXIT_SUCCESS;
    }

    return l_iReturnedValue;
}


/**
 * @brief Function used to send name of the compagny stored in the configuration file, one
 *  by one, at each call. This system allow us to start a thread for this compagny, give it the
 *  name, and so on. If we reach the end of the file, the function send back a ret code. At the
 *  next call, the configuration file is going to be read from the beginning.
 * @param p_sCompagny : string where the compagny is stored. Use this result only if the function returns EXIT_SUCCESS
 * @return EXIT_FAILURE if there is an opening issue. EOF is we reach the end of the file (no compagny to retrieve on this turn) ; EXIT_SUCCESS is p_sCompagny contains a valid compagny name
 */
int configurationAnalyseLineByLine(char* p_sCompagny)
{
    char l_cCharacter = 0;
    char l_sLine[MAX_CONFIG_LINE_LEN];
    unsigned int l_iCursor = 0;
    static FILE* l_fileConfigurationFile = NULL;


    /* First init of the function */
    if(l_fileConfigurationFile == NULL)
    {
        l_fileConfigurationFile = fopen(CONFIGURATION_FILE, "r");

        /* Real fail */
        if(l_fileConfigurationFile == NULL)
        {
            return EXIT_FAILURE;
        }
    }

    while(l_cCharacter != EOF)
    {
        l_cCharacter = fgetc(l_fileConfigurationFile);

        /* End of line - Check is we take a compagny name or a keyword */
        if(l_cCharacter == '\n' || l_iCursor > MAX_CONFIG_LINE_LEN - 1)
        {
            if(l_sLine[l_iCursor - 1] == ':')
            {
                /* Compagny name on the line */

                /* Remove the 'compagny' marker. It was arbitrary decided */
                l_sLine[l_iCursor - 1] = '\0';
                strcpy(p_sCompagny, l_sLine);
                /* End of this call - but another is coming that's why we don't close the stream  */
                return EXIT_SUCCESS;
            }
            else
            {
                /* We founf keywords, clean the line and start another */
                bzero(l_sLine, strlen(l_sLine));
                l_iCursor = 0;
            }
        }
        else
        {
            l_sLine[l_iCursor++] = l_cCharacter;
        }
    }

    fclose(l_fileConfigurationFile);

    return EOF;
}
