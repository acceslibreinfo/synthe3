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

//Transcription alphabétique-phonétique

#include <stdlib.h>
#include <string.h>
#include "SynMain.h"
#include "SynTrans.h"
#include "SynVoix.h"

//////////////////////////////////////////////////////
//Méthodes de la classe Transcription
//////////////////////////////////////////////////////

//Phonème alpha -> phonème interne
void Transcription::phonemePhoneme(char* texte, char* texPhon) {
	char c;

	texteLec=texte;
	texteEcr=texPhon;
	iLec=0; iEcr=0;
	while (true) {
		if (!carSuiv()) break;
		c=tab->tPhon(tab->tWin(carac));
		if (c==NULP) continue;
		ecrit(c+65);	//évite 0 et rend visibles les caractères
	}
	texPhon[iEcr]=0;
}

//minuscules -> majuscules et traitement carac répété n fois
void Transcription::minMajNFois(char* texteAlphaLec, char* texteAlphaEcr) {
	short nbFois;
	char chaine[5];	//pour nbFois
	short iLecCherche;

	texteLec=texteAlphaLec;
	texteEcr=texteAlphaEcr;
	iLec=0; iEcr=0;
	while (true) {
		carac=(unsigned char)texteLec[iLec++];
		if ((char)carac==MARQ_MARQ) {
			if (texteLec[iLec]==MARQ_INDEX) {
				if (iEcr<NM_CAR_TEX_2) {
					texteEcr[iEcr++]=MARQ_MARQ;
					texteEcr[iEcr++]=MARQ_INDEX;
				}
				iLec++;
				continue;
			}
		}
		if (carac==0) break;
		if ((carac<48 && carac >32) || carac>57) {	//pas les chiffres ni esp ni tab
			if (texteLec[iLec]==carac) {
				if (texteLec[iLec+1]==carac) {
					nbFois=3;	//déjà trois fois
					for (iLecCherche=iLec+2; texteLec[iLecCherche]==carac; iLecCherche++)
						nbFois++;
					if (iEcr<NM_CAR_TEX_8) {
						sprintf(chaine, "%d", nbFois);
						texteEcr[iEcr]=0;	//pour concaténer
						strcat(texteEcr, chaine);
						iEcr+=strlen(chaine);
						texteEcr[iEcr++]=tab->FOIS;
						texteEcr[iEcr++]=(char)tab->tWin(carac);
						texteEcr[iEcr++]=' ';
						iLec=iLecCherche;
						continue;
					}
				}
			}
		}
		carac=tab->tWin(carac);
		ecrit(carac);
	}
	texteEcr[iEcr]=0;
}

//Caractère alpha -> phonème (code interne)
void Transcription::graphemePhoneme(char* texte, char* texPhon) {
	char cArbre;
	unsigned char caracAnc;
	unsigned char categAnc;
	char* ptArbreAnc;
	short iLecAnc;
	short iEcrAnc;
	bool sigle=false;	//pas de sigle au départ
	short bavard=synGlobal.getModeLecture();
	short abrev=1-synGlobal.getModeLecture();

	texteLec=texte;
	texteEcr=texPhon;
	ctBava=0;	//pas de bavard provisoire
	liaison=0;	//normal
	iLec=0; iEcr=0;
	ptArbre=tab->getPtTabDeb(' ');	//comme si on était derrière un espace
	if (!carSuiv()) {
		texPhon[iEcr]=0;
		return;
	}
	while (true) {	//traite caractère par caractère
		while (true) {	//cherche la règle qui s'applique
			cArbre=*ptArbre++;
			if ((unsigned char)cArbre==carac) {
				iLecAnc=iLec; iEcrAnc=iEcr;
				ptArbreAnc=ptArbre;
				caracAnc=carac; categAnc=categ;	//garde anciennes valeurs pour dico non
				carSuiv();
				break;	//caractère trouvé
			}
			if (cArbre<=TTT && cArbre>=categ) break;	//catégorie trouvée
			if (cArbre==BVRD && bavard) break;
			if (cArbre==NABR && !abrev) break;
			while (true) {	//règle non validée, passer à la suivante
				cArbre=*ptArbre++;
				if (cArbre>=PG) break;
			}
			ptArbre+=2;
		}
		//Règle validée
		while (true) {	//traite la règle validée et prépare l'adresse suivante
			while (true) {	//lit les phonèmes et les écrit
				cArbre=*ptArbre++;
				if (cArbre>=PG) break;
				ecritPhon(cArbre);
			}
			ptArbre=tab->getPtArbre(ptArbre);	//prépare adresse suivante
			if (cArbre==RB) {	//arbre : branche
				break;
			} else if (cArbre==PG) {	//arbre : racine
				if (*ptArbre>0) liaison=*ptArbre;
				ptArbre++;
				if (carac==0) {	//fin de l'énoncé
					liaison=-1;
					break;
				}
				if (categ==CHIF)
					traiteNombre();
				else {
					ptArbre=tab->getPtArbre(ptArbre, carac);	//racine
					if (ptArbre==tab->getPtTabDeb(carac)) {	//début de mot : cherche si sigle
						if (categ==CNS) {
							iLecAnc=iLec; iEcrAnc=iEcr;
							while (true) {
								carSuiv();
								if (categ!=CNS) {
									if (categ==VOY || categ==VOYM) sigle=false;
									else sigle=true;
									break;
								}
							}
							iLec=iLecAnc; iEcr=iEcrAnc;
						}
					}
					carSuiv();
					if (sigle) {
						if (categ==LET) categ=PONC;
						else sigle=false;
					}
				}
				break;
			} else if (cArbre==DC) {	//dictionnaire
				while (*ptArbre!=0) {	//vérifie si la chaine correspond
					if ((unsigned char)*ptArbre!=carac) break;
					ptArbre++;
					carSuiv();
				}
				if (*ptArbre==0) {
					ptArbre++;
					sigle=false;
					continue;	//le mot est dans le dico
				} else {	//dico non, restaure les anciennes valeurs
					carac=caracAnc; categ=categAnc;
					ptArbre=ptArbreAnc+3;	//début règle suivante
					iLec=iLecAnc; iEcr=iEcrAnc;
					break;
				}
			}
		}
		if (liaison<0) break;
	}
	texPhon[iEcr]=0;
}

//Traite les nombres, y compris en mode compta
void Transcription::traiteNombre() {
	unsigned short int nbC=0;	//nombre de chiffres dans le dernier groupe de 3 chiffres (1 à 3)
	unsigned short int nbGr3=0;	//nombre de groupes de 3 chiffres (1 à 3)
	unsigned short int iN=0;	//indice chiffre du nombre
	unsigned short int groupe=0;	//compte les chiffres dans un groupe continu
	bool petitGroupeInterdit=false;	//groupe <3 chiffres interdit (sauf au début)
	short iLecPrec;
	short iEcrPrec;
	short iLecAnc;
	short iEcrAnc;
	unsigned short int nbCAnc;
	unsigned short int nbGr3Anc;
	char c;	//chiffre
	char c1;	//chiffre
	char c2;	//chiffre

	while (true) {	//Fabrique un nombre de 12 chiffres max, en enlevant les séparateurs en mode compta
		if (categ!=CHIF && !synGlobal.getModeCompta()) {
			iLec=iLecPrec;
			iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
			break;	//et arrête le nombre
		}
		if (nbC==3) {
			if (nbGr3==3) {	//dépasse la taille max du nombre
				if (petitGroupeInterdit) {	//il y a déjà eu au moins un séparateur (mode compta)
					nbGr3=nbGr3Anc;
					nbC=nbCAnc;
					iLec=iLecAnc;
					iEcr=iEcrAnc;	//reprend les valeurs anciennes
					break;	//et arrête le nombre
				}
				//tronque le nombre à 12 chiffres
				iLec=iLecPrec;
				iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
				break;	//et arrête le nombre
			} else {
				nbGr3++; nbC=1;
			}
		} else nbC++;
		nombre[iN++]=carac;
		groupe++;
		iLecPrec=iLec;
		iEcrPrec=iEcr;	//sauvegarde
		carSuiv();
		if (synGlobal.getModeCompta()) {
			if (carac==' ') {	//séparateur
				if (groupe!=3) {	//pas groupe 3 chiffres
					if (petitGroupeInterdit) {	//c'est interdit
						nbGr3=nbGr3Anc;
						nbC=nbCAnc;
						iLec=iLecAnc;
						iEcr=iEcrAnc;	//reprend les valeurs anciennes
						break;	//et arrête le nombre
					} else if (groupe>3) {	//groupe>3
						iLec=iLecPrec;
						iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
						break;	//et arrête le nombre
					}
				}
				//groupe de 3 chiffres ou 1er groupe < 4 chiffres : sauve l'état et continue
				groupe=0;
				petitGroupeInterdit=true;	//maintenant, groupe<3 chiffres interdit
				nbGr3Anc=nbGr3;
				nbCAnc=nbC;
				iLecAnc=iLecPrec;
				iEcrAnc=iEcrPrec;	//sauve les valeurs actuelles (avant carSuiv précédent)
				carSuiv();
			} else if (categ!=CHIF) {	//fin obligée
				if (groupe==3 || !petitGroupeInterdit) {	//groupe OK
					iLec=iLecPrec;
					iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
					break;	//et arrête le nombre
				} else {	//interdit
					nbGr3=nbGr3Anc;
					nbC=nbCAnc;
					iLec=iLecAnc;
					iEcr=iEcrAnc;	//reprend les valeurs anciennes
					break;	//et arrête le nombre
				}
			}
		}
	}
	//Phonétise le nombre
	iN=0;
	while (nombre[iN]=='0') {	//nombre commençant par zéro
		phonChif(tab->ch01());	//"zéro"
		iN++;
		nbC--;
		if (nbC==0) {
			nbC=3;
			if (nbGr3==0) {
				finNombre(iN);
				return;
			} else {
				nbGr3--;
			}
		}
	}
	//Cas général
	while (true) {
		c=nombre[iN++]; //nbC=1, 2, 3
		switch (nbC) {
		case 3:	//... nnn ...
			nbC--;
			if (c=='0') continue;	//... 0nn ...
			else if (c=='1')	//... 1nn ...
				phonChif(tab->ch100());	//"sâ[t]"
			else {
				phonChif(tab->chListUnit(c));	//"de:", "trwa:", "kat", ...
				phonChif(tab->ch100Z());	//"sâ:[z]"
			}
			break;
		case 2: //...nn ...
			nbC--;
			if (c=='0') continue;	//...0n ...
			else if (c=='1' && nombre[iN]<'7')	//...10 ... à ...16 ...
				break;
			else {
				phonChif(tab->chListDiz(c));	//"diz", "vî:[t]", "trât", "karâ:t", ...
				if (c=='2') {	//...2n ...
					if (nombre[iN]>'0')	//...21 ... à ...29 ...
						ecritPhon(TT);
				} else if ((c=='7' || c=='9') && nombre[iN]>'6')
					phonChif(tab->chListDiz('1'));
			}
			break;
		case 1:	//...n ...
			nbC--; c1=0; c2=0;
			if (iN>1) c1=nombre[iN-2];
			if (iN>2) c2=nombre[iN-3];
			if (c=='1') {
				if (c1>'1' && c1<'8') {	//...21 ... à ...71 ...
					ecritPhon(EE); ecritPhon(PL);	//"é:"
				} else if (c1=='8')	//...81 ...
					ecritPhon(VR);	//","
			}
			if (c<'7' && (c1=='1' || c1=='7' || c1=='9'))	//...10 ... à ...16 ..., ...70 ... à ...76 ..., ...90 ... à ...96 ...
				phonChif(tab->chListUnit(c+10));	//"di", "ô:z", "dùz", ...
			else if (iN==1 || (c1=='0' && c2=='0')) {	//n, n nnn, n nnn nnn, n nnn nnn nnn, ... 00n ...
				if (c=='1') {	//1, 1 nnn, 1 nnn nnn, 1 nnn nnn nnn, ... 001 ...
					if (nbGr3!=1) phonChif(tab->ch1S());	//"î:"
				} else
					phonChif(tab->chListUnit(c));	//"de:", "trwa:", "kat", ...
			} else phonChif(tab->chListUnit(c));	//"î", "de:", "trwa:", "kat", ...
			if (nbGr3==0) {
				finNombre(iN);
				return;
			} else {	//... nnn
				nbGr3--; nbC=3;
				if (c!='0' || c1!='0' || c2!='0')	//sauf ... 000 ...
					phonChif(tab->chListMil(nbGr3));	//"mil", "milyô", "milya::r"
			}
		}
	}
}

//Fin du nombre
void Transcription::finNombre(short unsigned int& iN) {
	char c;
	char c1;

	c=nombre[--iN];
	if (c=='8') ptArbre=tab->_Huit1();
	else {
		c1=0;
		if (iN>0) c1=nombre[iN-1];
		if ((c1=='0' || (c1>'1' && c1<'7') || c1=='8' || c1==0) && c=='5') ecritPhon(KK);
		if (((c1=='1' || c1=='7' || c1=='9') && c=='0') || (c1!='1' && c1!='7' && c1!='9' && c=='6')) ptArbre=tab->_Dix1();
		else ptArbre=tab->XChif();
	}
	carSuiv();
}

//Phonétise le chiffre
void Transcription::phonChif(char* ptChif) {
	char c;

	if (*ptChif==80) return;	//...0
	while (true) {
		c=*ptChif++;
		if (c==80) break;
		ecritPhon(c);
	}
	liaison=*ptChif;
}

//Lit le caractère suivant
unsigned char Transcription::carSuiv() {
	while (true) {
		carac=(unsigned char)texteLec[iLec++];
		if ((char)carac==MARQ_MARQ) {
			if (texteLec[iLec]==MARQ_INDEX) {
				ecrit(INDEX);
				iLec++;
				continue;
			}
		}
		break;
	}
	categ=tab->tCat(carac);
	return carac;
}

//Ecrit le caractère de la chaine cible avec la liaison éventuelle
void Transcription::ecritPhon(char c) {
	if (liaison>0) {
		if ((liaison<10 && c>9) || (liaison>9 && c<10)) {	//liaison possible
			ecrit(liaison+65);	//évite 0 et rend visibles les caractères
		}
	}
	ecrit(c+65);	//évite 0 et rend visibles les caractères
	liaison=0;
}

//Ecrit le caractère de la chaine cible en contrôlant la taille
void Transcription::ecrit(char c) {
	if (iEcr<NM_CAR_TEX_1) texteEcr[iEcr++]=c;
}

