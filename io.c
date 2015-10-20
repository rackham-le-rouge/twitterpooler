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
    return mkdir(p_cName, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXUSR);
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

                    /* Create here a true fake line */
                    fprintf(l_fileEmptyChecksumFile, "00000000000000000000000000000000\n");

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

    l_iReturnedValue = (mkfifo(PIPE_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXUSR) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

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
                /* We found keywords, clean the line and start another */
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




/**
 * @brief This function manage all the I/O of a thread to its assigned checksum file.
 * A checksum file (.md5) hold all md5 of all already known messages, if a md5 isn't here it is
 * because the message a new one. With this function, you can open the file, check if a md5
 * is in, append a new md5 at the end and close the file. You have to open the file once, at
 * the thread's starts and close once too at the thread's closing.
 * @param p_sName : name of the page pooled by the thread
 * @param p_sMD5Hash : string with the md5sum in it (already converted in letters. You don't have to put a value when p_enumAction equals INIT or CLOSE
 * @param p_enumAction : the wanted action, INIT to init the file descriptor etc.. Cf enum checksumFileAction
 * @return 0 in all cases, and if p_enumAction equals CHECK_EXIST this function returns 1 if the p_sMD5Hash is already in the file and 0 if this p_sMD5Hash is unknown
 */
int updateAndReadChecksumFile(char* p_sName, char* p_sMD5Hash, enum checksumFileAction p_enumAction)
{
    static FILE* l_fileChecksum = NULL;
    static char l_sFileName[MAX_CONFIG_LINE_LEN];
    char l_sReadLine[34];       /* 33 + 1 EOL */
    int l_iRetCode;

    l_iRetCode = 0;
    bzero(l_sReadLine, 34);

    /* If we want to use a un-initialized file */
    if(p_enumAction != INIT && l_fileChecksum == NULL)
    {
        LOG_WARNING("Try to do action %d but no INIT have be done before...", p_enumAction);
        return 0;
    }


    /* Classical actions, described in the enum structure */
    switch(p_enumAction)
    {
        /* To call once at the first usage of this function */
        case INIT:
            snprintf(l_sFileName, MAX_CONFIG_LINE_LEN, "%s/%s.md5", CHECKSUM_DIRECTORY, p_sName);
            l_fileChecksum = fopen(l_sFileName, "a+");
            if(l_fileChecksum == NULL)
            {
                LOG_ERROR("File %s impossible to open. No actions on it.", l_sFileName);
            }
            else
            {
                LOG_INFO("File %s correctlly opened.", l_sFileName);
            }
            break;

        /* you have to check if the p_sMD5Hash is already present in the file, this action
         * just append p_sMD5Hash to the end of the file */
        case UPDATE:
            LOG_INFO("Add [%s]", p_sMD5Hash);
            fseek (l_fileChecksum, 0, SEEK_END);
            fprintf(l_fileChecksum, "%s\n", p_sMD5Hash);
            break;

        /* Check if p_sMD5Hash is already in the file, read all the file. This function may
         * be greedy, if there is too many disk access we have to fix this FIXME */
        case CHECK_EXIST:
            fseek (l_fileChecksum, 0, SEEK_SET);
            do
            {
                fgets(l_sReadLine, MAX_CONFIG_LINE_LEN, l_fileChecksum);
                if(strstr(l_sReadLine, p_sMD5Hash) != NULL)
                {
                    return 1;
                }
            }while(!feof(l_fileChecksum));
            break;

        /* Call when the thread is going to close */
        case CLOSE:
            l_iRetCode = fclose(l_fileChecksum);
            if(l_iRetCode != 0)
            {
                LOG_ERROR("fclose failed. File still open for %s, errno is %d", l_sFileName, errno);
            }
            break;

        default:
            LOG_WARNING("This action doen't exist : %d", p_enumAction);
            break;
    }

    return 0;
}
