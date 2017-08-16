/**
 * Copyright @ Covond Ltd. co.
 *
 * iniFile.h : iniFile read/write interface
 *
 * Author : hfxu@covond.com
 * Create : 2006-2-24
 * Modify : 
 */

#ifndef __INI_FILE__
#define __INI_FILE__

typedef struct _IniFile *IniFilePtr;
//typedef void *IniFilePtr;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

IniFilePtr iniFileOpen(const char *filename);
void iniFileFree(IniFilePtr pIni);
int  iniFileSave(IniFilePtr pIni);
int  iniFileSaveTo(IniFilePtr pIni, const char *filename);

int iniHasSection(IniFilePtr pIni, const char *section);
const char * iniGetStr(IniFilePtr pIni, const char *section, \
                        const char *keyname);
int  iniGetInt(IniFilePtr pIni, const char *section, \
                const char *keyname);

void iniSetStr(IniFilePtr pIni, const char *section, \
                const char *keyname, const char *keyvalue);
void iniSetInt(IniFilePtr pIni, const char *section, \
                const char *keyname, int keyvalue);

int  iniDelSec(IniFilePtr pIni, const char *section);
char *iniGetStrByKey(IniFilePtr pIni, const char *keyname);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INI_FILE__ */

