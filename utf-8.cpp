/*
 * Synthé 3 - A speech synthetizer software for french
 *
 * Copyright (C) 1985-2015 by Michel MOREL <michel.morel@unicaen.fr>.
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

//Convertit UTF-8 en latin1 pour les traitements internes de Synthé 3

#include <string.h>
#include "SynMain.h"

//Globales au module
char latinC2[256];	//tables selon le premier caractère C2, C3...
char latinC3[256];
char latinC5[256];
char latinC6[256];
char latinCB[256];

//Convertit chaîne UTF-8 en latin1
bool UTF8Latin1(const char *chaineLec, char* chaineEcr) {
	short iLec=0;
	short iEcr=0;
	unsigned char carac;
	unsigned char carac1;
	unsigned char carac2;
	long bilan=0;	//sert à déterminer si le texte en entrée est en UTF-8 ou latin1
	//Pour chaque caractère : bilan inchangé si <0x80, bilan-1 si <0xC0, +10 si UTF-8 valide, -10 si pas UTF-8
	//Si le caractère est un UTF-8 valide mais non pertinent (langue étrangère ou non traité) il est ignoré, avec bilan+10

	//Lire un caractère à la fois dans la chaîne,
	while (chaineLec[iLec] !=0 && iEcr<NM_CAR_TEX-3) {
		carac1=chaineLec[iLec++];
		//Caractères sur 3 octets
		if (carac1>223) {	//E0 et plus
			switch(carac1) {
			case 0xE2:
				carac2=chaineLec[iLec++];
				if (carac2>127) {
					switch(carac2) {
					case 0x80:
						carac=chaineLec[iLec++];
						if (carac>127) {
							switch (carac) {
							case 0x8C:	//ZERO WIDTH NON-JOINER
								bilan+=10;
								break;
							case 0x8D:	//ZERO WIDTH JOINER
								bilan+=10;
								break;
							case 0x8E:	//LEFT-TO-RIGHT MARK
								bilan+=10;
								break;
							case 0x8F:	//RIGHT-TO-LEFT MARK
								bilan+=10;
								break;
							case 0x90:
								chaineEcr[iEcr++]='-';	//HYPHEN
								bilan+=10;
								break;
							case 0x91:
								chaineEcr[iEcr++]=(char)173;	//NON-BREAKING HYPHEN
								bilan+=10;
								break;
							case 0x92:
								chaineEcr[iEcr++]=(char)150;	//FIGURE DASH
								bilan+=10;
								break;
							case 0x93:
								chaineEcr[iEcr++]=(char)150;	//EN DASH
								bilan+=10;
								break;
							case 0x94:
								chaineEcr[iEcr++]=(char)151;	//EM DASH
								bilan+=10;
								break;
							case 0x98:
								chaineEcr[iEcr++]=(char)145;	//LEFT SINGLE QUOTATION MARK
								bilan+=10;
								break;
							case 0x99:
								chaineEcr[iEcr++]=(char)146;	//RIGHT SINGLE QUOTATION MARK
								bilan+=10;
								break;
							case 0x9A:
								chaineEcr[iEcr++]=(char)130;	//GUILLEMET-VIRGULE INFÉRIEUR
								bilan+=10;
								break;
							case 0x9C:
								chaineEcr[iEcr++]=(char)147;	//LEFT DOUBLE QUOTATION MARK
								bilan+=10;
								break;
							case 0x9D:
								chaineEcr[iEcr++]=(char)148;	//RIGHT DOUBLE QUOTATION MARK
								bilan+=10;
								break;
							case 0x9E:
								chaineEcr[iEcr++]=(char)132;	//DOUBLE LOW-9 QUOTATION MARK
								bilan+=10;
								break;
							case 0xA0:
								chaineEcr[iEcr++]=(char)134;	//CROSS
								bilan+=10;
								break;
							case 0xA1:
								chaineEcr[iEcr++]=(char)135;	//DOUBLE CROSS
								bilan+=10;
								break;
							case 0xA2:
								chaineEcr[iEcr++]=(char)149;	//BULLET
								bilan+=10;
								break;
							case 0xA6:
								chaineEcr[iEcr++]=(char)133;	//HORIZONTAL ELLIPSIS
								bilan+=10;
								break;
							case 0xAA:	//LEFT-TO-RIGHT EMBEDDING
								bilan+=10;
								break;
							case 0xAB:	//RIGHT-TO-LEFT EMBEDDING
								bilan+=10;
								break;
							case 0xB0:
								chaineEcr[iEcr++]=(char)137;	//POUR MILLE
								bilan+=10;
								break;
							case 0xB9:
								chaineEcr[iEcr++]=(char)139;	//LEFT ANGLE BRACKET
								bilan+=10;
								break;
							case 0xBA:
								chaineEcr[iEcr++]=(char)155;	//RIGHT ANGLE BRACKET
								bilan+=10;
								break;
							default:
								bilan--;
							}
						} else
							bilan--;
						break;
					case 0x82:
						carac=chaineLec[iLec++];
						if (carac>127) {
							switch (carac) {
							case 0xAC:
								chaineEcr[iEcr++]=(char)128;	//EURO
								bilan+=10;
								break;
							default:
								bilan--;
							}
						} else
							bilan--;
						break;
					case 0x84:
						carac=chaineLec[iLec++];
						if (carac>127) {
							switch (carac) {
							case 0xA2:
								chaineEcr[iEcr++]=(char)153;	//TRADE MARK SIGN
								bilan+=10;
								break;
							default:
								bilan--;
							}
						} else
							bilan--;
						break;
					default:
						bilan--;
					}
				} else {
					bilan-=10;
				}
				break;
			//Autres cas de caractères sur 3 octets : RAS en français
			default:
				bilan--;
			}
		} else if (carac1>191) {	//caractères sur 2 octets (C0 et plus)
			switch(carac1) {
			case 0xC2:
				carac2=chaineLec[iLec++];
				if (carac2>127) {
					carac=latinC2[carac2];	//voir tableau latinC2 dans Synthe.cpp
					if (carac==0) {
						bilan--;	//UTF-8 non défini
					} else {
						chaineEcr[iEcr++]=carac;
						bilan+=10;	//UTF-8 valide
					}
				} else {
					bilan-=10;	//pas UTF-8
				}
				break;
			case 0xC3:
				carac2=chaineLec[iLec++];
				if (carac2>127) {
					carac=latinC3[carac2];	//voir tableau latinC3 dans Synthe.cpp
					if (carac==0) {
						bilan--;	//UTF-8 non défini
					} else {
						chaineEcr[iEcr++]=carac;
						bilan+=10;	//UTF-8 valide
					}
				} else {
					bilan-=10;	//pas UTF-8
				}
				break;
			case 0xC5:
				carac2=chaineLec[iLec++];
				if (carac2>127) {
					carac=latinC5[carac2];	//voir tableau latinC5 dans Synthe.cpp
					if (carac==0) {
						bilan--;	//UTF-8 non défini
					} else {
						chaineEcr[iEcr++]=carac;
						bilan+=10;	//UTF-8 valide
					}
				} else {
					bilan-=10;	//pas UTF-8
				}
				break;
			case 0xC6:
				carac2=chaineLec[iLec++];
				if (carac2>127) {
					carac=latinC6[carac2];	//voir tableau latinC6 dans Synthe.cpp
					if (carac==0) {
						bilan--;	//UTF-8 non défini
					} else {
						chaineEcr[iEcr++]=carac;
						bilan+=10;	//UTF-8 valide
					}
				} else {
					bilan-=10;	//pas UTF-8
				}
				break;
			case 0xCB:
				carac2=chaineLec[iLec++];
				if (carac2>127) {
					carac=latinCB[carac2];	//voir tableau latinCB dans Synthe.cpp
					if (carac==0) {
						bilan--;	//UTF-8 non défini
					} else {
						chaineEcr[iEcr++]=carac;
						bilan+=10;	//UTF-8 valide
					}
				} else {
					bilan-=10;	//pas UTF-8
				}
				break;
			default:
				bilan--;
			}
		} else {
			chaineEcr[iEcr++]=carac1;
			if (carac1>127)	//carac latin1 ou erreur UTF-8
				bilan--;
		}
	}
	chaineEcr[iEcr]=0;
	return (bilan>0);
}
