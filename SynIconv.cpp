/*
 * Synthé 3 - A speech synthetizer software for french
 *
 * Copyright (C) 1985-2014 by Michel MOREL <michel.morel@unicaen.fr>.
 *
 * Synthé 3 comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU Lesser General Public License, as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version. Please see the file LICENSE-LGPL for details.
 *
 * Web Page: https://github.com/acceslibreinfo/synthe3
 *
 * This software is maintained by Shérab <Sebastien.Hinderer@ens-lyon.org>.
 */

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
