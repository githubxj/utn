/**
*
 * iniFile.c : iniFile read/write implementation
 *
  * Modify : 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "iniFile.h"

#define pfree free
#define pmalloc malloc

static int strtokcasecmp(const char *s1, const char *s2, const char *token)
{
  int i;

  for(i=0; 0!=s1[i]; i++)
  {
    if(0 == index(token, s1[i]))
      break;
  }
  s1 += i;

  for(i=0; 0!= s2[i]; i++)
  {
    if(0 == index(token, s2[i]))
      break;
  }
  s2 += i;

  int l1,l2;
  for(i=strlen(s1)-1; i>=0; i--)
    if(0 == index(token, s1[i]))
      break;
  l1 = i+1;

  for(i=strlen(s2)-1; i>=0; i--)
    if(0 == index(token, s2[i]))
      break;
   l2=i+1;

  i=l1>l2?l2:l1;
  int r = strncasecmp(s1, s2, i);
  if(0 == r)
  {
    if(l1>l2)
      return 1;
    else if(l1<l2)
      return -1;
    else
      return 0;
  }
  else
  {
    return r;
  }
}

typedef struct _string
{
  int size;
  struct _string *next;
  char *content;
} linestring;

static linestring *newstr()
{
  linestring *str = (linestring *)pmalloc(sizeof(linestring));
  if(0 == str)
    return 0;

  str->content = (char *)pmalloc(32);
  if(0 == str->content)
  {
    pfree(str);
    return 0;
  }
  memset(str->content, 0 , 32);
  str->size = 32;
  str->next = 0;
  return str;
}

static void delstr(linestring *str)
{
  if(0 == str)
    return;

  if(0 != str->content)
    pfree(str->content);
  pfree(str);
}

static char *strval(linestring *str)
{
  if(0 == str)
    return 0;
  return str->content;
}

static int strapps(linestring *str, const char *buf, int num)
{
  if(0 == str)
    return -1;
  if(-1 > num)
    return -1;
  if(0 == num)
    return -1;
  if(-1 == num)
    num = strlen(buf);
  //if(num > strlen(buf))
  //  num = strlen(buf);

  int len = 0;
  int size = 0;

  if(0 != str->size)
    len = strlen(str->content);

  size = (len+num)/32*32+32;

  if(str->size < size)
  {
    char *content = (char *)pmalloc(size);
    if(0 == content)
    {
      return 0;
    }

    memset(content, 0, size);

    if(0 != str->size)
    {
      strcpy(content, str->content);
      pfree(str->content);
    }

    str->content = content;
    str->size = size;
  }

  strncpy(str->content+len, buf, num);
  
  return num;
}

static int streval(linestring *str, const char *val)
{
  if(0 == str)
    return 0;

  memset(str->content, 0, str->size);
  return strapps(str, val, -1);
}

struct _IniFile
{
  linestring *filename;
  linestring *lhead;
  int lcnt;
  //int lsize;
};

static struct _IniFile *newini()
{
  struct _IniFile *pIni = (struct _IniFile *)pmalloc(sizeof(struct _IniFile));
  if(0 == pIni)
    return 0;
  pIni->filename = 0;
  pIni->lhead = 0;
  pIni->lcnt  = 0;
  //pIni->lsize = 0;
  return pIni;
}

static void delini(struct _IniFile *pIni)
{
  if(0 == pIni)
    return;
  if(pIni->filename)
    delstr(pIni->filename);
  linestring *ln, *lt= pIni->lhead;
  while(lt)
  {
    ln = lt->next;
    delstr(lt);
    lt = ln;
  }
  pfree(pIni);
}

static linestring *newline(struct _IniFile *pIni, int idx)
{
  if(0 == pIni)
    return 0;
  if(-1 > idx)
    return 0;
  if(-1==idx || idx>pIni->lcnt)
    idx = pIni->lcnt;

  linestring *s = newstr();
  if(0 == s)
    return 0;

  int i = 0;
  linestring *lp = 0, *lt = pIni->lhead;
  while(lt)
  {
    if(i==idx)
    {
      break;
    }
    lp = lt; //if here be excuted, lp will never be null
    lt = lt->next;
    i++;
  }
  if(lp)
    lp->next = s;
  else
    pIni->lhead = s;
  s->next = lt;

  pIni->lcnt ++;
  return s;
}

static linestring *addline(struct _IniFile *pIni, const char *buf, int idx)
{
  linestring *lt;
  if(0 == (lt=newline(pIni, idx)))
    return 0;
  streval(lt, buf);
  return lt;
}

static int dellines(struct _IniFile *pIni, int idx, int lcnt)
{
  if(0 == pIni)
    return 0;
  if(-1>idx || 0>=lcnt)
    return 0;
  if(-1==idx || idx>=pIni->lcnt)
    idx = pIni->lcnt-1;
  //if(idx+lcnt > pIni->lcnt)
  //  lcnt = pIni->lcnt-idx;

  int i = 0, j = 0;
  linestring *ln = 0, *lp = 0, *lt = pIni->lhead;
  while(lt)
  {
    if(i>=idx)
    {
      if(j<lcnt)
      {
        ln = lt->next;
        delstr(lt);
        lt = ln;
      }
      else
      {
        break;
      }
      j++;
    }
    else
    {
      lp = lt;
      lt = lt->next;
    }
    i++;
  }

  if(0 == lp)
    pIni->lhead = ln;
  else
    lp->next = ln;

  pIni->lcnt -= j;
  return lcnt;
}

//static void delline(struct _IniFile *pIni, int idx)
//{
//  dellines(pIni, idx, 1);
//}

IniFilePtr iniFileOpen(const char *filename)
{
  IniFilePtr pIni = newini();
  if(0 == pIni)
  {
    return 0;
  }

  if(0 == filename)
    return pIni;

  FILE *f = fopen(filename, "a+");
  if(0 == f)
  {
    delini(pIni);
    return 0;
  }

  fseek(f, 0, SEEK_SET);

  if(0 == pIni->filename)
    pIni->filename = newstr();
  streval(pIni->filename, filename);

  char buf[256];
  int done = 0;
  linestring *lstr = 0;
  do{
    memset(buf, 0, 256);
    int rs = fread(buf, 1, 255, f);
    if(rs == 0)
      break;
    if(rs < 255)
      done = 1;

    char *tb, *pbuf = buf;
    while(pbuf[0] != '\0')
    {
      tb = index(pbuf, '\n');
      if(tb)//means has '\n', set end of line
      {
        tb[0] = '\0';
        if(tb != pbuf && tb[-1] == '\r')
          tb[-1] = '\0';
      }

      if(lstr)//pre-line not ended
        strapps(lstr, pbuf, -1);
      else
        lstr = addline(pIni, pbuf, -1);

      if(tb)//means has '\n', end of line
      {
        lstr = 0;
        pbuf = tb+1;
      }
      else
        break;
    }
  }while(!done);

  fclose(f);
  return pIni;
}

void iniFileFree(IniFilePtr pIni)
{
  delini(pIni);
}

int  iniFileSave(IniFilePtr pIni)
{
  int len;
  if(0 == pIni->filename)
    return -1;
  if(0 == (len=strlen(strval(pIni->filename))))
    return -1;

  char *file = (char *)pmalloc(len+4);
  if(0 == file)
    return -1;
  sprintf(file, "%s.b", strval(pIni->filename));

  if(0 == iniFileSaveTo(pIni, file))
  {
    unlink(strval(pIni->filename));
    link(file, strval(pIni->filename));
    unlink(file);
    pfree(file);
    return 0;
  }
  else
  {
    pfree(file);
    return -1;
  }
}

int iniFileSaveTo(IniFilePtr pIni, const char *filename)
{
  if(0 == pIni)
    return -1;

  if(0 == strlen(filename))
    return -1;

  FILE *f = fopen(filename, "w+");
  if(0 == f)
    return -1;

  linestring *lt = pIni->lhead;
  while(lt)
  {
    const char *val = strval(lt);
    if(val)
      fprintf(f,"%s\n",val);
    lt = lt->next;
  }

  fclose(f);
  return 0;
}

int iniHasSection(IniFilePtr pIni, const char *section)
{
  int bFindSection = 0;

  const char *line;

  linestring *lt = pIni->lhead;
  for(; lt != 0; lt = lt->next)
  {
    line = strval(lt);
    if(0 == line) // empty string
      continue;

    if( line[0] == ';' )  //Comment
      continue;

    if( line[0] != '[' )
      continue;

    //Section Line
    if(0 == index(line, ']')) //no end
      continue;

    if( 0 == strtokcasecmp(line, section, " []") )
    {
      bFindSection = 1;
      break;
    }
  }

  return bFindSection;
}

const char *iniGetStr(IniFilePtr pIni, const char *section, \
                        const char *keyname)
{
  int bFindSection = 0;
  char *result = 0;

  const char *line;

  linestring *lt = pIni->lhead;
  for(; lt != 0; lt = lt->next)
  {
    line = strval(lt);
    if(0 == line) // empty string
    {
      continue;
    }


    if( line[0] == ';' )  //Comment
    {
      continue;
    }

    //Section Line
    if( line[0] == '[' )
    {
      if(0 == index(line, ']'))
      {
        continue;
      }

      if( bFindSection )
      {
        break;
      }

      if( 0 == strtokcasecmp(line, section, " []") )
      {
        bFindSection = 1;
      }

      continue;
    }

    if( !bFindSection )
    {
      continue;
    }

    //Key line
    if(0 == index(line, '='))
      continue;

    int len = index(line,'=') - line + 1;
    char *key = (char *)pmalloc(len);
    memset(key, 0, len);
    strncpy(key, line, len-1);

    if(0 == strtokcasecmp(key, keyname, " "))
    {
      result = index(line, '=') + 1;
	  while(*result == ' ') result++; /*skipp the leading space*/
      pfree(key);
      break;
    }
    pfree(key);
  }//end of while

  return result;
}

char *iniGetStrByKey(IniFilePtr pIni, const char *keyname)
{
  char *result = 0;

  const char *line;

  linestring *lt = pIni->lhead;
  for(; lt != 0; lt = lt->next)
  {
    line = strval(lt);
    if(0 == line) // empty string
    {
      continue;
    }


    if( line[0] == ';' )  //Comment
    {
      continue;
    }

    //Section Line
    if( line[0] == '[' )
    {
        continue;
    }

    //Key line
    if(0 == index(line, '='))
      continue;

    int len = index(line,'=') - line + 1;
    char *key = (char *)pmalloc(len);
    memset(key, 0, len);
    strncpy(key, line, len-1);

    if(0 == strtokcasecmp(key, keyname, " "))
    {
      result = index(line, '=') + 1;
	  while(*result == ' ') result++; /*skipp the leading space*/
      pfree(key);
      break;
    }
    pfree(key);
  }//end of while

  return result;
}

int iniGetInt(IniFilePtr pIni, const char *section, \
                const char *keyname)
{
  const char *val = iniGetStr(pIni, section, keyname);
  if(0 == val)
    return 0;
  return strtol(val, NULL, 0);
}

void iniSetStr(IniFilePtr pIni, const char *section, \
                const char *keyname, const char *keyvalue)
{
  int bFindSection = 0;
  int bFindKey = 0;

  const char *line;

  linestring *lt = pIni->lhead;
  int l = 0;
  for(; lt != 0; lt = lt->next, l++)
  {
    line = strval(lt);
    if(0 == line) // empty string
    {
      continue;
    }

    if( line[0] == ';' )  //Comment
    {
      continue;
    }

    //Section Line
    if((section != NULL) &&( line[0] == '[' ))
    {
      if(0 == index(line, ']'))
      {
        continue;
      }

      if( bFindSection )//not find keyname
      {
        break;
      }

      if( 0 == strtokcasecmp(line, section, " []") )
      {
        bFindSection = 1;
      }

      continue;
    }

    if((section != NULL) &&( !bFindSection ))
    {
      continue;
    }

    //Key line
    if(0 == index(line, '='))
      continue;

    int len = index(line,'=') - line + 1;
    char *key = (char *)pmalloc(len);
    memset(key, 0, len);
    strncpy(key, line, len-1);

    if(0 == strtokcasecmp(key, keyname, " "))//find section and key
    {
      pfree(key);
      bFindKey = 1;
      streval(lt, keyname);
      strapps(lt, "=", -1);
      strapps(lt, keyvalue, -1);
      //strapps(lt, "\n", 1);
      break;
    }

    pfree(key);
  }//end of while

  //not find section (so not key)
  if((section !=NULL) && (!bFindSection))
  {
    lt = newline(pIni, -1);
    if(0 == lt)
      return;
    strapps(lt, "[", 1);
    strapps(lt, section, -1);
    strapps(lt, "]", 1);
    //strapps(lt, "\n", 1);
    l++;
  }

  if(!bFindKey)
  {
    if(0 == (lt=newline(pIni, l)))
    {
      return;
    }
    strapps(lt, keyname, -1);
    strapps(lt, "=", 1);
    strapps(lt, keyvalue, -1);
    //strapps(lt, "\n", 1);
  }
}

void iniSetInt(IniFilePtr pIni, const char *section, \
                const char *keyname, int keyvalue)
{
  char buf[16];
  snprintf(buf,16,"%d",keyvalue);
  iniSetStr(pIni, section, keyname, buf);
}

int iniDelSec(IniFilePtr pIni, const char *section)
{
  int bFindSection = 0;
  int lineBeg = -1;
  const char *line;

  int l=0;
  linestring *lt = pIni->lhead;
  for(; lt != 0; lt = lt->next, l++)
  {
    line = strval(lt);
    if(0 == line) // empty string
    {
      continue;
    }

    if( line[0] == ';' )  //Comment
    {
      continue;
    }

    //Section Line
    if( line[0] == '[' )
    {
      if(0 == index(line, ']'))
      {
        continue;
      }

      if(bFindSection)
      {
        break;
      }

      if( 0 == strtokcasecmp(line, section, " []") )
      {
        lineBeg = l;
        bFindSection = 1;
      }

      continue;
    }
  }

  if(-1 != lineBeg)
  {
    dellines(pIni, lineBeg, l-lineBeg);
    return l - lineBeg;
  }

  return 0; 
}

