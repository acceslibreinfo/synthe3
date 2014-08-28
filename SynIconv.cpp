///////////////////////////////////
// Convertit la chaîne UTF-8 en ISO8859-15
///////////////////////////////////

#include <iostream>
#include <fstream>
#include <cstring>
#include <iconv.h>

char* synIconv(char *texte) {
{
  char *dst;
  size_t srclen = strlen(texte);
  size_t dstlen = strlen(texte);

  char *pIn = texte;
  char *pOut = dst;

        iconv_t conv = iconv_open("WCHAR_T", "ISO8859-15");
        iconv(conv, &pIn, &srclen, &pOut, &dstlen);
        iconv_close(conv);
}
