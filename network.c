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



void* threadPagePooling(void* p_structInitData)
{
    structPagePoolingInitData* l_structInitData = (structPagePoolingInitData*)p_structInitData;
    /*LOG_INFO("Cpy name %s", l_structInitData->sName);*/
    return NULL;
}



void networkLoop(int p_iHowManyCompagnies)
{
    struct MemoryStruct l_structMemory;
    structPagePoolingInitData* l_structPagePoolingInitInformation;
    int l_iReturnedValue;
    int l_iThreadNumber;
    char l_sCompagnyName[MAX_CONFIG_LINE_LEN];
    pthread_t* l_structPagePoolingThreadID;

    UNUSED(l_structMemory);
    l_structPagePoolingInitInformation = (structPagePoolingInitData*)malloc(p_iHowManyCompagnies * sizeof(structPagePoolingInitData));
    l_structPagePoolingThreadID = (pthread_t*)malloc(p_iHowManyCompagnies * sizeof(pthread_t));

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
}



