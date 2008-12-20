#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "picoc.h"

/* all platform-dependent code is in this file */

void Fail(const char *Message, ...)
{
    va_list Args;
    
    va_start(Args, Message);
    vStrPrintf(Message, Args);
    exit(1);
}

void ProgramFail(struct LexState *Lexer, const char *Message, ...)
{
    va_list Args;

    StrPrintf("%S:%d: ", Lexer->FileName, Lexer->Line);   
    va_start(Args, Message);
    vStrPrintf(Message, Args);
    StrPrintf("\n");
    exit(1);
}

/* read a file into memory. this is the only function using malloc().
 * do it differently for embedded devices without malloc */
char *ReadFile(const Str *FileName)
{
    struct stat FileInfo;
    char *Text;
    FILE *InFile;
    char CFileName[PATH_MAX];
    
    StrToC(CFileName, PATH_MAX, FileName);
    
    if (stat(CFileName, &FileInfo))
        Fail("can't read file %s\n", CFileName);
    
    Text = malloc(FileInfo.st_size);
    
    InFile = fopen(CFileName, "r");
    if (InFile == NULL)
        Fail("can't read file %s\n", CFileName);
    
    if (fread(Text, 1, FileInfo.st_size, InFile) != FileInfo.st_size)
        Fail("can't read file %s\n", CFileName);

    fclose(InFile);
    
    return Text;    
}

/* read and scan a file for definitions */
void ScanFile(const Str *FileName)
{
    char *Source;
    Str SourceStr;
    
    Source = ReadFile(FileName);
    StrFromC(&SourceStr, Source);
    Parse(FileName, &SourceStr, TRUE);
}

int main(int argc, char **argv)
{
    Str FileName;
    Str StartFunc;
    
    if (argc < 2)
        Fail("Format: picoc <program.c> <args>...\n");
    
    ParseInit();
    
    StrFromC(&FileName, argv[1]);
    ScanFile(&FileName);
    
    return 0;
}