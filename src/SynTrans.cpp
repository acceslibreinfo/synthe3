//Transcription alphab�tique-phon�tique

#include <stdlib.h>
#include <string.h>
#include "SynMain.h"
#include "SynTrans.h"
#include "SynVoix.h"

//////////////////////////////////////////////////////
//M�thodes de la classe Transcription
//////////////////////////////////////////////////////

//Phon�me alpha -> phon�me interne
void Transcription::phonemePhoneme(char* texte, char* texPhon) {
	char c;

	texteLec=texte;
	texteEcr=texPhon;
	iLec=0; iEcr=0;
	while (true) {
		if (!carSuiv()) break;
		c=tab->tPhon(tab->tWin(carac));
		if (c==NULP) continue;
		ecrit(c+65);	//�vite 0 et rend visibles les caract�res
	}
	texPhon[iEcr]=0;
}

//minuscules -> majuscules et traitement carac r�p�t� n fois
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
		if (carac<48 && carac >32 || carac>57) {	//pas les chiffres ni esp ni tab
			if (texteLec[iLec]==carac) {
				if (texteLec[iLec+1]==carac) {
					nbFois=3;	//d�j� trois fois
					for (iLecCherche=iLec+2; texteLec[iLecCherche]==carac; iLecCherche++)
						nbFois++;
					if (iEcr<NM_CAR_TEX_8) {
					  // Fonction itoa() non d�finie en C++ sauf par certains compilateurs
						#ifdef WIN32
						itoa(nbFois, chaine,10);
						#else
						sprintf(chaine, "%d", nbFois);
						#endif
						texteEcr[iEcr]=0;	//pour concat�ner
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

//Caract�re alpha -> phon�me (code interne)
void Transcription::graphemePhoneme(char* texte, char* texPhon) {
	char cArbre;
	unsigned char caracAnc;
	unsigned char categAnc;
	char* ptArbreAnc;
	short iLecAnc;
	short iEcrAnc;
	bool sigle=false;	//pas de sigle au d�part
	short bavard=synGlobal.getModeLecture();
	short abrev=1-synGlobal.getModeLecture();

	texteLec=texte;
	texteEcr=texPhon;
	ctBava=0;	//pas de bavard provisoire
	liaison=0;	//normal
	iLec=0; iEcr=0;
	ptArbre=tab->getPtTabDeb(' ');	//comme si on �tait derri�re un espace
	if (!carSuiv()) {
		texPhon[iEcr]=0;
		return;
	}
	while (true) {	//traite caract�re par caract�re
		while (true) {	//cherche la r�gle qui s'applique
			cArbre=*ptArbre++;
			if ((unsigned char)cArbre==carac) {
				iLecAnc=iLec; iEcrAnc=iEcr;
				ptArbreAnc=ptArbre;
				caracAnc=carac; categAnc=categ;	//garde anciennes valeurs pour dico non
				carSuiv();
				break;	//caract�re trouv�
			}
			if (cArbre<=TTT && cArbre>=categ) break;	//cat�gorie trouv�e
			if (cArbre==BVRD && bavard) break;
			if (cArbre==NABR && !abrev) break;
			while (true) {	//r�gle non valid�e, passer � la suivante
				cArbre=*ptArbre++;
				if (cArbre>=PG) break;
			}
			ptArbre+=2;
		}
		//R�gle valid�e
		while (true) {	//traite la r�gle valid�e et pr�pare l'adresse suivante
			while (true) {	//lit les phon�mes et les �crit
				cArbre=*ptArbre++;
				if (cArbre>=PG) break;
				ecritPhon(cArbre);
			}
			ptArbre=tab->getPtArbre(ptArbre);	//pr�pare adresse suivante
			if (cArbre==RB) {	//arbre : branche
				break;
			} else if (cArbre==PG) {	//arbre : racine
				if (*ptArbre>0) liaison=*ptArbre;
				ptArbre++;
				if (carac==0) {	//fin de l'�nonc�
					liaison=-1;
					break;
				}
				if (categ==CHIF)
					traiteNombre();
				else {
					ptArbre=tab->getPtArbre(ptArbre, carac);	//racine
					if (ptArbre==tab->getPtTabDeb(carac)) {	//d�but de mot : cherche si sigle
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
				while (*ptArbre!=0) {	//v�rifie si la chaine correspond
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
					ptArbre=ptArbreAnc+3;	//d�but r�gle suivante
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
	char nbC=0;	//nombre de chiffres dans le dernier groupe de 3 chiffres (1 � 3)
	char nbGr3=0;	//nombre de groupes de 3 chiffres (1 � 3)
	char iN=0;	//indice chiffre du nombre
	char groupe=0;	//compte les chiffres dans un groupe continu
	bool petitGroupeInterdit=false;	//groupe <3 chiffres interdit (sauf au d�but)
	short iLecPrec;
	short iEcrPrec;
	short iLecAnc;
	short iEcrAnc;
	char nbCAnc;
	char nbGr3Anc;
	char c;	//chiffre
	char c1;	//chiffre
	char c2;	//chiffre

	while (true) {	//Fabrique un nombre de 12 chiffres max, en enlevant les s�parateurs en mode compta
		if (categ!=CHIF && !synGlobal.getModeCompta()) {
			iLec=iLecPrec;
			iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
			break;	//et arr�te le nombre
		}
		if (nbC==3) {
			if (nbGr3==3) {	//d�passe la taille max du nombre
				if (petitGroupeInterdit) {	//il y a d�j� eu au moins un s�parateur (mode compta)
					nbGr3=nbGr3Anc;
					nbC=nbCAnc;
					iLec=iLecAnc;
					iEcr=iEcrAnc;	//reprend les valeurs anciennes
					break;	//et arr�te le nombre
				}
				//tronque le nombre � 12 chiffres
				iLec=iLecPrec;
				iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
				break;	//et arr�te le nombre
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
			if (carac==' ') {	//s�parateur
				if (groupe!=3) {	//pas groupe 3 chiffres
					if (petitGroupeInterdit) {	//c'est interdit
						nbGr3=nbGr3Anc;
						nbC=nbCAnc;
						iLec=iLecAnc;
						iEcr=iEcrAnc;	//reprend les valeurs anciennes
						break;	//et arr�te le nombre
					} else if (groupe>3) {	//groupe>3
						iLec=iLecPrec;
						iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
						break;	//et arr�te le nombre
					}
				}
				//groupe de 3 chiffres ou 1er groupe < 4 chiffres : sauve l'�tat et continue
				groupe=0;
				petitGroupeInterdit=true;	//maintenant, groupe<3 chiffres interdit
				nbGr3Anc=nbGr3;
				nbCAnc=nbC;
				iLecAnc=iLecPrec;
				iEcrAnc=iEcrPrec;	//sauve les valeurs actuelles (avant carSuiv pr�c�dent)
				carSuiv();
			} else if (categ!=CHIF) {	//fin oblig�e
				if (groupe==3 || !petitGroupeInterdit) {	//groupe OK
					iLec=iLecPrec;
					iEcr=iEcrPrec;	//reprend les valeurs avant carSuiv
					break;	//et arr�te le nombre
				} else {	//interdit
					nbGr3=nbGr3Anc;
					nbC=nbCAnc;
					iLec=iLecAnc;
					iEcr=iEcrAnc;	//reprend les valeurs anciennes
					break;	//et arr�te le nombre
				}
			}
		}
	}
	//Phon�tise le nombre
	iN=0;
	while (nombre[iN]=='0') {	//nombre commen�ant par z�ro
		phonChif(tab->ch01());	//"z�ro"
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
	//Cas g�n�ral
	while (true) {
		c=nombre[iN++]; //nbC=1, 2, 3
		switch (nbC) {
		case 3:	//... nnn ...
			nbC--;
			if (c=='0') continue;	//... 0nn ...
			else if (c=='1')	//... 1nn ...
				phonChif(tab->ch100());	//"s�[t]"
			else {
				phonChif(tab->chListUnit(c));	//"de:", "trwa:", "kat", ...
				phonChif(tab->ch100Z());	//"s�:[z]"
			}
			break;
		case 2: //...nn ...
			nbC--;
			if (c=='0') continue;	//...0n ...
			else if (c=='1' && nombre[iN]<'7')	//...10 ... � ...16 ...
				break;
			else {
				phonChif(tab->chListDiz(c));	//"diz", "v�:[t]", "tr�t", "kar�:t", ...
				if (c=='2') {	//...2n ...
					if (nombre[iN]>'0')	//...21 ... � ...29 ...
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
				if (c1>'1' && c1<'8') {	//...21 ... � ...71 ...
					ecritPhon(EE); ecritPhon(PL);	//"�:"
				} else if (c1=='8')	//...81 ...
					ecritPhon(VR);	//","
			}
			if (c<'7' && (c1=='1' || c1=='7' || c1=='9'))	//...10 ... � ...16 ..., ...70 ... � ...76 ..., ...90 ... � ...96 ...
				phonChif(tab->chListUnit(c+10));	//"di", "�:z", "d�z", ...
			else if (iN==1 || c1=='0' && c2=='0') {	//n, n nnn, n nnn nnn, n nnn nnn nnn, ... 00n ...
				if (c=='1') {	//1, 1 nnn, 1 nnn nnn, 1 nnn nnn nnn, ... 001 ...
					if (nbGr3!=1) phonChif(tab->ch1S());	//"�:"
				} else
					phonChif(tab->chListUnit(c));	//"de:", "trwa:", "kat", ...
			} else phonChif(tab->chListUnit(c));	//"�", "de:", "trwa:", "kat", ...
			if (nbGr3==0) {
				finNombre(iN);
				return;
			} else {	//... nnn
				nbGr3--; nbC=3;
				if (c!='0' || c1!='0' || c2!='0')	//sauf ... 000 ...
					phonChif(tab->chListMil(nbGr3));	//"mil", "mily�", "milya::r"
			}
		}
	}
}

//Fin du nombre
void Transcription::finNombre(char& iN) {
	char c;
	char c1;

	c=nombre[--iN];
	if (c=='8') ptArbre=tab->_Huit1();
	else {
		c1=0;
		if (iN>0) c1=nombre[iN-1];
		if ((c1=='0' || c1>'1' && c1<'7' || c1=='8' || c1==0) && c=='5') ecritPhon(KK);
		if ((c1=='1' || c1=='7' || c1=='9') && c=='0' || c1!='1' && c1!='7' && c1!='9' && c=='6') ptArbre=tab->_Dix1();
		else ptArbre=tab->XChif();
	}
	carSuiv();
}

//Phon�tise le chiffre
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

//Lit le caract�re suivant
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

//Ecrit le caract�re de la chaine cible avec la liaison �ventuelle
void Transcription::ecritPhon(char c) {
	if (liaison>0) {
		if (liaison<10 && c>9 || liaison>9 && c<10) {	//liaison possible
			ecrit(liaison+65);	//�vite 0 et rend visibles les caract�res
		}
	}
	ecrit(c+65);	//�vite 0 et rend visibles les caract�res
	liaison=0;
}

//Ecrit le caract�re de la chaine cible en contr�lant la taille
void Transcription::ecrit(char c) {
	if (iEcr<NM_CAR_TEX_1) texteEcr[iEcr++]=c;
}

