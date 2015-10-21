/** 
 * @file        network.c
 * @rogram      Twitter aspirator
 * @brief       Network stuff, in order to connect to the net and retreive pages
 * @description In order to download pages
 * @date        2015 bitch
 * @licence     Beerware (rev 42)
 * @author      Jerome GRARD - Not your sister....
 */

#include "config.h"


/**
 * @brief writeMemoryCallback is the called function, when the request is executed. It aim to reseize the allocated memory for
 * for the buffer. Usinf a realloc.
 * @param p_contents : retrieved data from the internet
 * @param p_size : size of the retreieved data in (blocks ?)
 * @param nmemb : size of one (block ?)
 * @param p_data : the structure used to carry the data
 * @return real size of the retrieved data (and thus of the extended memory location)
 */
size_t writeMemoryCallback(void* p_contents, size_t p_size, size_t nmemb, void* p_data)
{
    size_t l_sizeRealsize = p_size * nmemb;
    struct MemoryStruct* l_structMemory = (struct MemoryStruct*)p_data;

    l_structMemory->memory = realloc(l_structMemory->memory, l_structMemory->size + l_sizeRealsize + 1);
    if(l_structMemory->memory == NULL)
    {
        /* out of memory! */ 
        LOG_ERROR("Not enough memory (realloc returned NULL)%s", ".");
        return EXIT_FAILURE;
    }

    memcpy(&(l_structMemory->memory[l_structMemory->size]), p_contents, l_sizeRealsize);
    l_structMemory->size += l_sizeRealsize;
    l_structMemory->memory[l_structMemory->size] = 0;

    return l_sizeRealsize;
}

/**
 * @brief Retreive html for the given url, and store it in the struture
 * @param p_cUrlToGet : the url to download
 * @param p_structMemory : the memory location of the struture to fill with the data
 * @return EXIT_SUCCESS for a a success or EXIT_FAILURE in the other cases
 */
int retrieveAnUrl(const char* p_cUrlToGet, struct MemoryStruct* p_structMemory)
{
    CURL* l_curlHandler = NULL;
    CURLcode l_curlResponseCode = 0;

    /* Initialisation with useless values */
    p_structMemory->memory = malloc(1);
    p_structMemory->size = 0; 

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */ 
    l_curlHandler = curl_easy_init();

    /* specify URL to get */ 
    curl_easy_setopt(l_curlHandler, CURLOPT_URL, p_cUrlToGet);

    /* send all data to this function  */ 
    curl_easy_setopt(l_curlHandler, CURLOPT_WRITEFUNCTION, writeMemoryCallback);

    /* we pass our struct to the callback function */ 
    curl_easy_setopt(l_curlHandler, CURLOPT_WRITEDATA, (void *)p_structMemory);

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */ 
    curl_easy_setopt(l_curlHandler, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux i686; rv:31.0) Gecko/20100101 Firefox/31.0 Iceweasel/31.6.0");    //libcurl-agent/1.0");

    /* get it! */ 
    l_curlResponseCode = curl_easy_perform(l_curlHandler);

    /* check for errors, on the other cases, the page is stored in the memory */ 
    if(l_curlResponseCode != CURLE_OK)
    {
        LOG_WARNING("Function curl_easy_perform() failed: %s", curl_easy_strerror(l_curlResponseCode));
        return EXIT_FAILURE;
    }

    /* cleanup curl stuff */ 
    curl_easy_cleanup(l_curlHandler);
    curl_global_cleanup();

    return EXIT_SUCCESS;
}


/**
 *@brief The pooling thread. This thread receive a page to pool, and loop on this action. Retrieve the page, analyse it
 *  find new results, send them to the main thread.
 *@param p_structInitData : a parameter of type structPagePoolingInitData to provide all informations and memory address pointers needed by this thread
 *@return with the function pthread_exit an exit code
 */
void* threadPagePooling (void* p_structInitData)
{
    structPagePoolingInitData* l_structInitData = (structPagePoolingInitData*)p_structInitData;
    MD5_CTX l_structMD5Context;
    FILE* l_fileChecksum;
    struct MemoryStruct l_structMemory;
    char* l_sUrl;
    char* l_sCursor;
    char* l_sQuote;
    int* l_iReturnValue;
    int l_iIterator;
    char l_sMD5Hash[33];
    unsigned char l_iMD5Output[16];     /* Declared as an array of bytes */

    LOG_INFO("Thread for %s started.", l_structInitData->sName);

    l_iReturnValue = (int*)malloc(sizeof(int));
    l_sUrl = (char*)malloc(MAX_CONFIG_LINE_LEN * sizeof(char));
    l_sCursor = NULL;
    l_sQuote = NULL;
    l_fileChecksum = NULL;

    MD5_Init(&l_structMD5Context);
    updateAndReadChecksumFile(l_structInitData->sName, NULL, INIT, &l_fileChecksum);

    if( l_iReturnValue == NULL ||
        l_sUrl == NULL)
    {
        LOG_ERROR("Not enought memory avilable. Abort. errno %d", errno);
        pthread_exit(l_iReturnValue);
    }

    /* URL creation for this thread - This thread is only going to pool this URL */
    snprintf(l_sUrl, MAX_CONFIG_LINE_LEN, "%s/%s", URL_PREFIX, l_structInitData->sName);
    if(retrieveAnUrl(l_sUrl, &l_structMemory) == EXIT_SUCCESS)
    {
        LOG_INFO("Page %s retrieved. Bytes %d", l_sUrl, l_structMemory.size);

        while(strstr(l_structMemory.memory, TOKEN_DELIMITER_FOR_DATA_START) != NULL)
        {
            /*****************
            *
            * Quote retrieving
            *
            *****************/
            /* Find token and remove starting point */
            l_sQuote = strstr(l_structMemory.memory, TOKEN_DELIMITER_FOR_DATA_START);
            memset(l_sQuote, ' ', strlen(TOKEN_DELIMITER_FOR_DATA_START));

            /* Prepare the line, starting and end of it */
            l_sQuote = strstr(l_sQuote, ">") + 1;
            l_sCursor = strstr(l_sQuote, TOKEN_DELIMITER_FOR_DATA_END);
            memset(l_sCursor, '\0', strlen(TOKEN_DELIMITER_FOR_DATA_END));

            /* Here, line is in l_sQuote, until the \0 character. And end of the line is at l_sCursor */
            LOG_INFO("Token [%s]", l_sQuote);

            /*****************
            *
            * MD5 of the quote
            *
            *****************/
            MD5_Update(&l_structMD5Context, l_sQuote, strlen(l_sQuote));
            MD5_Final(l_iMD5Output, &l_structMD5Context);

            for(l_iIterator = 0; l_iIterator < 16; ++l_iIterator)
            {
                sprintf(&l_sMD5Hash[l_iIterator * 2], "%02x", (unsigned int)l_iMD5Output[l_iIterator]);
            }

            /*****************
            *
            *  MD5Quote save
            *
            *****************/
            if(updateAndReadChecksumFile(l_structInitData->sName, l_sMD5Hash, CHECK_EXIST, &l_fileChecksum) != 1)
            {
                updateAndReadChecksumFile(l_structInitData->sName, l_sMD5Hash, UPDATE, &l_fileChecksum);
            }

 
            /*****************
            *
            *  Quote erase
            *
            *****************/
            /* Remove end of line marker */
            memset(l_sCursor, ' ', strlen(TOKEN_DELIMITER_FOR_DATA_END));
        }
    }
    else
    {
        LOG_INFO("Page %s NOT retrieved. Network error.", l_sUrl);
    }

    updateAndReadChecksumFile(l_structInitData->sName, NULL, CLOSE, &l_fileChecksum);
    *l_iReturnValue = 314;              /* Test value */


    free(l_sUrl);

    pthread_exit(l_iReturnValue);
}


/**
 * @brief This function is the main loop for the network function. This function starts all threads for all pages to follow, and wait
 *  for the end of their execution. This function call a parse function to find the name of the page to follow, and gives all parameters
 *  to the pooling thread.
 * @param p_iHowManyCompagnies : number of pages to follow, so, number of threads to start
 * @return nothing yet
 */
void networkLoop(int p_iHowManyCompagnies)
{
    structPagePoolingInitData* l_structPagePoolingInitInformation;
    int l_iReturnedValue;
    int l_iThreadNumber;
    int l_iIterator;
    int* l_iReturnedThreadValue;
    char l_sCompagnyName[MAX_CONFIG_LINE_LEN];
    pthread_t* l_structPagePoolingThreadID;

    l_structPagePoolingInitInformation = (structPagePoolingInitData*)malloc(p_iHowManyCompagnies * sizeof(structPagePoolingInitData));
    l_structPagePoolingThreadID = (pthread_t*)malloc(p_iHowManyCompagnies * sizeof(pthread_t));
    l_iReturnedThreadValue = NULL;

    if(l_structPagePoolingInitInformation == NULL)
    {
        LOG_ERROR("Not enouth memory available. errno %d", errno);
        exit(ENOMEM);
    }
    if(l_structPagePoolingThreadID == NULL)
    {
        LOG_ERROR("Not enouth memory available, errno %d", errno);
        exit(ENOMEM);
    }

    l_iThreadNumber = 0;
    LOG_INFO("Going to generate %d threads", p_iHowManyCompagnies);



    /* Analyse the configuration file, find page's name, and start one thread per page */
    do
    {
        bzero(l_sCompagnyName, MAX_CONFIG_LINE_LEN);
        l_iReturnedValue = configurationAnalyseLineByLine(l_sCompagnyName);

        if(l_iReturnedValue == EXIT_SUCCESS)
        {
            /* We have a valid name */
            strcpy((l_structPagePoolingInitInformation + l_iThreadNumber)->sName, l_sCompagnyName);
            LOG_INFO("Start thread for %s", (l_structPagePoolingInitInformation + l_iThreadNumber)->sName);
            if(pthread_create(  l_structPagePoolingThreadID + l_iThreadNumber,
                                NULL,
                                threadPagePooling,
                                (void*)(l_structPagePoolingInitInformation + l_iThreadNumber)) < 0)
            {
                LOG_ERROR("Creating thread number %d for compagny %s failed.", l_iThreadNumber, l_sCompagnyName);
            }
            else
            {
                /* Creation succeed */
                l_iThreadNumber++;
            }
        }
        else if(l_iReturnedValue == EXIT_FAILURE)
        {
            LOG_ERROR("Thread creation killed. Compagny extraction failed. errno %d", errno);
            break;
        }
    }while(l_iReturnedValue != EOF);





    /* Wait for the end of all pooling threads */
    LOG_INFO("Thread starting is OK. %d threads pushed", p_iHowManyCompagnies);
    for(l_iIterator = 0; l_iIterator < p_iHowManyCompagnies; l_iIterator++)
    {
        if(*(l_structPagePoolingThreadID + l_iIterator) != 0)
        {
            if(pthread_join(*(l_structPagePoolingThreadID + l_iIterator), (void*)&l_iReturnedThreadValue) != 0)
            {
                LOG_ERROR("Error on pthread_joined, errno %d", errno);
            }
            else
            {
                LOG_INFO("Returned value for %d is %d", l_iIterator, *l_iReturnedThreadValue);
                *(l_structPagePoolingThreadID + l_iIterator) = 0;

                /* release memory declared in the thread */
                free(l_iReturnedThreadValue);
            }
        }
    }
}



