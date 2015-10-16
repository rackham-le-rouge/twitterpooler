/** 
 * @file        network.h
 * @rogram      Twitter aspirator
 * @brief       Network stuff, in order to connect to the net and retreive pages
 * @description In order to download pages
 * @date        2015 bitch
 * @licence     Beerware (rev 42)
 * @author      Jerome GRARD - Not your sister....
 */

#ifndef NETWORK_H
#define NETWORK_H

size_t writeMemoryCallback(void* p_contents, size_t p_size, size_t nmemb, void* p_data);
int retrieveAnUrl(const char* p_cUrlToGet, struct MemoryStruct* p_structMemory);
void* threadPagePooling(void* p_structInitData);
void networkLoop(void);


#endif
