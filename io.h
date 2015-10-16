/**
 * @file        io.h
 * @program     Twitter pooler to grab crap
 * @brief       IO header, for read/write on files
 * @description All functions to read write on the program configuration files
 * @date        2015 bitch
 * @licence     Beerware (rev 42)
 * @author      Jerome GRARD - Not your sister....
 */




#ifndef _IO_H_
#define _IO_H_


int checkIfAFileExist(const char* p_cFileName);
int checkReadWriteFile(const char* p_cFileName);
int createDirectory(const char* p_cName);
unsigned int checkConfigurationFiles(void);
int initExternalCommunication(void);







#endif
