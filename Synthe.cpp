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

///////////////////////////////////////////////////////
// *** SYNTHE ***
// Synthé logiciel (fonctions publiques)
///////////////////////////////////////////////////////

#ifdef WIN32
	#define EXTERNE extern "C" _declspec(dllexport)	//pour Synthe.h
#else // linux ...
	#define EXTERNE extern "C"
	#include <unistd.h>
	#include <pthread.h>
#endif

#define SYNTHE_VERSION "Synthé version 1.1"

#define TAILLE_PARAG 10000	//pour conversion utf-8 - latin1

#include <ctime>
#include <string.h>
#include "SynMain.h"
#include "Synthe.h"
#include "SynVoix.h"

//Variables de Synthe.cpp
HANDLE hThread;
short posLec[NM_INDEX+1];	//il y a une place de plus que d'index
short nbIndex;
Voix** tVoix;	//tableau des voix
Tab* tab;	//les tables de Synthé (objet, ne pas confondre avec tTab, table qui fait partie de cet objet)

//////////////////////////////////////////////////////////////////
// Fonctions publiques pour faire parler Synthé
//////////////////////////////////////////////////////////////////

//Les fonctions de Synthé sont en mode _stdcall pour les appels à partir d'autres langages (Visual Basic, ...)
//Le nombre d'index est calculé avant de créer le thread, pour être bon dès le début (min 1 pendant le son)

//Envoi d'un texte à lire par Synthé
//Le paramètre texte est obligatoire, les autres sont facultatifs, la valeur -1 indique la conservation de la valeur courante
void _stdcall synTexte(
	const char* const texte,	//peut être constitué de plusieurs paragraphes, sans dépasser NM_CAR_TEX caractères.
	short volume,	//0 à 15 par pas de 25 % (par défaut 10)
	short debit,	//0 à 15 par pas de 12 % (par défaut 4)
	short hauteur,	//0 à 15 par pas de 12 % (par défaut 4)
	short phon, 	//1, le texte est phonétique
	short modeLecture,	//0, normal, 1, dit la ponctuation
	short modeCompta,	//0, le séparateur de milliers reste, 1, le séparateur de milliers est enlevé
	short sortieSon,		//sortie sur la carte-son
	short sortieWave,		//sortie sous forme de fichier wave
	char* nomFicWave	//nom éventuel du fichier à construire
	) {

	typeParamThread* lpParamThread=new typeParamThread;
#ifdef WIN32
	DWORD idThread;
#endif
	char* texteAlphaLatin1;	//texte après conversion en latin1
	char* bufferMessage;
	long longTex;	//longueur texte

	demandeStopEtAttendFinThread();	//stop texte en cours
	if (!texte) {
		delete lpParamThread;
		return;
	}
	longTex=strlen((char*)texte);
	if (longTex<1000)
		longTex=1000;
	else if (longTex>NM_CAR_TEX)
		longTex=NM_CAR_TEX;
	bufferMessage=new char[longTex*2+10];	//au pire : "a b c d ..."
	//Conversion utf8-latin1 (message -> texteAlphaLatin1)
	texteAlphaLatin1=new char[longTex+1];
	if (!UTF8Latin1(texte, texteAlphaLatin1))
		strcpy(texteAlphaLatin1, texte);	//si la conversion échoue, c'est qu'on est déjà en latin1, on copie directement le texte
	//Demande la version de Synthé
	if (strncmp(texteAlphaLatin1,"nversionsyn",9)==0)
		strcpy (bufferMessage, SYNTHE_VERSION);
	//Compte les index, les place s'il n'y en a pas
	copieEtAjouteIndexSiPas(texteAlphaLatin1, bufferMessage);	//compte les index
	delete[] texteAlphaLatin1;	//on détruit le texte provisoire
	//Crée le thread
	lpParamThread->texte=bufferMessage;
	lpParamThread->phon=phon;
	lpParamThread->volume=volume;
	lpParamThread->debit=debit;
	lpParamThread->hauteur=hauteur;
	lpParamThread->modeLecture=modeLecture;
	lpParamThread->modeCompta=modeCompta;
	lpParamThread->sortieSon=sortieSon;
	lpParamThread->sortieWave=sortieWave;
	lpParamThread->nomFicWave=nomFicWave;
	synGlobal.setThreadOK(false);
#ifdef WIN32
	hThread=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&fThAlpha, lpParamThread, 0, &idThread);
	SetPriorityClass(hThread, HIGH_PRIORITY_CLASS);
	Sleep(0);	//accélère la création du thread
#else
	pthread_create(&hThread, NULL, (void*(*)(void*))fThAlpha, lpParamThread);
	sleep(0);	//accélère la création du thread
#endif
}

//Retourne la valeur de l'index de lecture (va du nb d'index à 0 en fin de lecture)
short _stdcall synIndex() {
	return synGlobal.getNbIndexLec();
}

//Retourne la position de lecture du texte
short _stdcall synPosLec() {
	short n=synGlobal.getNbIndexLec();
	if (n<1) return -1;
	return posLec[nbIndex-n];
}

//Stop parole : arrête la lecture (effet immédiat)
//Indispensable quand on arrête un programme utilisant Synthé avec DirectX
//	sous peine de bouclage irréversible du tampon de lecture
void _stdcall synStop() {
	demandeStopEtAttendFinThread();
}

/////////////////////////////
//	Fonctions privées
/////////////////////////////

//Démarrage du thread : synthèse à partir du texte
void fThAlpha(void* lpParam) {
	synTex(lpParam);
}

//Si thread en cours : demande stop puis attend la fin du thread
void demandeStopEtAttendFinThread() {
#ifdef WIN32
	if (WaitForSingleObject(hThread,0)==WAIT_TIMEOUT) {
		synGlobal.setDemandeStop(true);	//demande le stop
		WaitForSingleObject(hThread,INFINITE); //et attend la fin du Thread
		synGlobal.setDemandeStop(false);	//OK, on peut envoyer un nouveau message
	}
#else
	if (hThread) {
		synGlobal.setDemandeStop(true);	//demande le stop (positionne une globale)
		//pthread_cancel(hThread);
		pthread_join(hThread, NULL);
		synGlobal.setDemandeStop(false);	//OK, on peut envoyer un nouveau message
	}
#endif
	synGlobal.setNbIndexLec(-1);	//-1 -> 0 indique lecture terminée (à 0 -> 1, il reste ce qui suit le dernier index)
	initWave(false);	//termine une éventuelle sortie wave
	sonDestruction();	//détruit l'objet son synSon (donc le buffer circulaire et l'objet carte-son snd_dev)
}

//Indexation automatique : copie la chaine en ajoutant des index si elle n'en comporte pas (et les compte)
void copieEtAjouteIndexSiPas (char* chaineLec, char* chaineEcr) {
	char* lec;
	char* ecr=chaineEcr;
	bool marqValide=false;

	posLec[0]=0;	//position initiale
	nbIndex=1;	//init comptage à 1 pour posLec car 0 représente le 1er mot (mais on décrémente après)
	for (lec=chaineLec; *lec!=0; lec++) {
		if (*lec==MARQ_MARQ) {
			if (lec[1]==MARQ_INDEX) {	//index trouvé
				if (nbIndex<=NM_INDEX)
					posLec[nbIndex++]=lec-chaineLec;	//repère l'index et compte
				lec++;
			}
		}
	}
	if (nbIndex>1) {
		//La chaine comporte des index
		ecr=chaineEcr;
		for (lec=chaineLec; *lec!=0 && lec<chaineLec+NM_CAR_TEX; lec++)	//limite la lecture à NM_CAR_TEX carac
			*ecr++=*lec;	//recopie simplement
		*ecr=0;
		synGlobal.setNbIndexLec(nbIndex-1);	//valeur pour Synthé avant mise à jour par lecture tampon
		synGlobal.setNbIndexMax(nbIndex-1);	//pour index sous linux
		return;
	}
	//Pas d'index : on les place
	ecr=chaineEcr;
	for (lec=chaineLec; *lec!=0 && lec<chaineLec+NM_CAR_TEX; lec++) {	//limite la lecture à NM_CAR_TEX carac
		if (*lec==0) {
			break;	//si finit par marqueur
		}
		if (!caracValide((char)*lec)) {
			if (marqValide) {	//place l'index derrière le car valide et le repère
				if (nbIndex<NM_INDEX) {
					posLec[nbIndex++]=lec-chaineLec;
					*ecr++=MARQ_MARQ; *ecr++=MARQ_INDEX;
				}
			}
			marqValide=false;
		} else
			 marqValide=true;	//prêt à placer le prochain index
		*ecr++=*lec;
	}
	*ecr++=MARQ_MARQ; *ecr++=MARQ_INDEX;*ecr=0;	//finit par un index
	posLec[nbIndex]=lec-chaineLec;	//position du dernier index (inutile car il sera ignoré et la vraie fin de lecture renverra -1)
	synGlobal.setNbIndexEcr(nbIndex);	//à écrire dans le tampon circulaire (min 1 pendant le son)
	synGlobal.setNbIndexLec(nbIndex);	//à retourner avant mise à jour par lecture tampon (min 1 pendant le son)
	synGlobal.setNbIndexMax(nbIndex);	//Romain pour index sous linux
}

//Détermine s'il s'agit d'un caractère ou d'un séparateur
bool caracValide(char carac) {
	if (carac==' ' || carac==10 || carac==13) return false;	//on pourrait affiner avec qques carac ou un tableau
	return true;
}

////////////////////////////////////
// Initialisation de Synthé
////////////////////////////////////

//Au lancement
void initSynthe() {
	short i;

	//Initialise les tables latin1 à 0
	for (i=0; i<256; i++) {
		latinC2[i]=0;
		latinC3[i]=0;
		latinC5[i]=0;
		latinC6[i]=0;
		latinCB[i]=0;
	}
	//Remplit les tables latin1 avec les valeurs connues
	latinC2[160]=(char)160;	//NO-BREAK SPACE
	latinC2[161]=(char)161;	//INVERTED EXCLAMATION MARK
	latinC2[162]=(char)162;	//CENT SIGN
	latinC2[163]=(char)163;	//POUND SIGN
	latinC2[164]=(char)164;	//CURRENCY SIGN
	latinC2[165]=(char)165;	//YEN SIGN
	latinC2[166]=(char)166;	//BROKEN BAR
	latinC2[167]=(char)167;	//SECTION SIGN
	latinC2[168]=(char)168;	 //DIAERESIS
	latinC2[169]=(char)169;	//COPYRIGHT SIGN
	latinC2[170]=(char)170;	//FEMININE ORDINAL INDICATOR
	latinC2[171]=(char)171;	//LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	latinC2[172]=(char)172;	//NOT SIGN
	latinC2[173]=(char)173;	//SOFT HYPHEN
	latinC2[174]=(char)174;	//REGISTERED SIGN
	latinC2[175]=(char)175;	//MACRON
	latinC2[176]=(char)176;	//DEGREE SIGN
	latinC2[177]=(char)177;	//PLUS-MINUS SIGN
	latinC2[178]=(char)178;	//SUPERSCRIPT TWO
	latinC2[179]=(char)179;	//LATIN SMALL LETTER O WITH ACUTE
	latinC2[180]=(char)180;	//ACUTE ACCENT
	latinC2[181]=(char)181;	//MICRO SIGN
	latinC2[182]=(char)182;	//PILCROW SIGN
	latinC2[183]=(char)183;	//MIDDLE DOT
	latinC2[184]=(char)184;	//CEDILLA
	latinC2[185]=(char)185;	//SUPERSCRIPT ONE
	latinC2[186]=(char)186;	//MASCULINE ORDINAL INDICATOR
	latinC2[187]=(char)187;	//RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	latinC2[188]=(char)188;	//VULGAR FRACTION ONE QUARTER
	latinC2[189]=(char)189;	//VULGAR FRACTION ONE HALF
	latinC2[190]=(char)190;	//VULGAR FRACTION THREE QUARTERS
	latinC2[191]=(char)191;	//INVERTED QUESTION MARK

	latinC3[128]=(char)192;	//CAPITAL A WITH GRAVE
	latinC3[129]=(char)193;	//CAPITAL A WITH ACUTE
	latinC3[130]=(char)194;	//CAPITAL A WITH CIRCUMFLEX
	latinC3[131]=(char)195;	//CAPITAL A WITH TILDE
	latinC3[132]=(char)196;	//CAPITAL A WITH UMLAUT
	latinC3[133]=(char)197;	//CAPITAL A WITH RING ABOVE
	latinC3[134]=(char)198;	//CAPITAL AE
	latinC3[135]=(char)199;	//CAPITAL C WITH CEDILLA
	latinC3[136]=(char)200;	//CAPITAL E WITH GRAVE
	latinC3[137]=(char)201;	//CAPITAL E WITH ACUTE
	latinC3[138]=(char)202;	//CAPITAL E WITH CIRCUMFLEX
	latinC3[139]=(char)203;	//CAPITAL E WITH UMLAUT
	latinC3[140]=(char)204;	//CAPITAL I WITH GRAVE
	latinC3[141]=(char)205;	//CAPITAL I WITH ACUTE
	latinC3[142]=(char)206;	//CAPITAL I WITH CIRCUMFLEX
	latinC3[143]=(char)207;	//CAPITAL I WITH UMLAUT
	latinC3[144]=(char)208;	//CAPITAL ETH
	latinC3[145]=(char)209;	//CAPITAL N WITH TILDE
	latinC3[146]=(char)210;	//CAPITAL O WITH GRAVE
	latinC3[147]=(char)211;	//CAPITAL O WITH ACUTE
	latinC3[148]=(char)212;	//CAPITAL O WITH CIRCUMFLEX
	latinC3[149]=(char)213;	//CAPITAL O WITH TILDE
	latinC3[150]=(char)214;	//CAPITAL O WITH UMLAUT
	latinC3[151]=(char)215;	//MULTIPLICATION SIGN
	latinC3[152]=(char)216;	//LATIN CAPITAL LETTER O WITH STROKE
	latinC3[153]=(char)217;	//LATIN CAPITAL LETTER U WITH GRAVE
	latinC3[154]=(char)218;	//LATIN CAPITAL LETTER U WITH ACUTE
	latinC3[155]=(char)219;	//LATIN CAPITAL LETTER U WITH CIRCUMFLEX
	latinC3[156]=(char)220;	//LATIN CAPITAL LETTER U WITH UMLAUT
	latinC3[157]=(char)221;	//LATIN CAPITAL LETTER Y WITH ACUTE
	latinC3[158]=(char)222;	//LATIN CAPITAL LETTER THORN
	latinC3[159]=(char)223;	//LATIN SMALL LETTER SHARP S
	latinC3[160]=(char)224;	//LATIN SMALL LETTER A WITH GRAVE
	latinC3[161]=(char)225;	//LATIN SMALL LETTER A WITH ACUTE
	latinC3[162]=(char)226;	//LATIN SMALL LETTER A WITH CIRCUMFLEX
	latinC3[163]=(char)227;	//LATIN SMALL LETTER A WITH TILDE
	latinC3[164]=(char)228;	//LATIN SMALL LETTER A WITH DIAERESIS
	latinC3[165]=(char)229;	//LATIN SMALL LETTER A WITH RING ABOVE
	latinC3[166]=(char)230;	//LATIN SMALL LETTER AE
	latinC3[167]=(char)231;	//LATIN SMALL LETTER C WITH CEDILLA
	latinC3[168]=(char)232;	//LATIN SMALL LETTER E WITH GRAVE
	latinC3[169]=(char)233;	//LATIN SMALL LETTER E WITH ACUTE
	latinC3[170]=(char)234;	//LATIN SMALL LETTER E WITH CIRCUMFLEX
	latinC3[171]=(char)235;	//LATIN SMALL LETTER E WITH UMLAUT
	latinC3[172]=(char)236;	//LATIN SMALL LETTER I WITH GRAVE
	latinC3[173]=(char)237;	//LATIN SMALL LETTER I WITH ACUTE
	latinC3[174]=(char)238;	//LATIN SMALL LETTER I WITH CIRCUMFLEX
	latinC3[175]=(char)239;	//LATIN SMALL LETTER I WITH UMLAUT
	latinC3[176]=(char)240;	//LATIN SMALL LETTER ETH
	latinC3[177]=(char)241;	//LATIN SMALL LETTER N WITH TILDE
	latinC3[178]=(char)242;	//LATIN SMALL LETTER O WITH GRAVE
	latinC3[179]=(char)243;	//LATIN SMALL LETTER O WITH ACUTE
	latinC3[180]=(char)244;	//LATIN SMALL LETTER O WITH CIRCUMFLEX
	latinC3[181]=(char)245;	//LATIN SMALL LETTER O WITH DIAERESIS
	latinC3[182]=(char)246;	//LATIN SMALL LETTER O WITH UMLAUT
	latinC3[183]=(char)247;	//DIVISION SIGN
	latinC3[184]=(char)248;	//LATIN SMALL LETTER O WITH STROKE
	latinC3[185]=(char)249;	//LATIN SMALL LETTER U WITH GRAVE
	latinC3[186]=(char)250;	//LATIN SMALL LETTER U WITH ACUTE
	latinC3[187]=(char)251;	//LATIN SMALL LETTER U WITH CIRCUMFLEX
	latinC3[188]=(char)252;	//LATIN SMALL LETTER U WITH DIAERESIS
	latinC3[189]=(char)253;	//LATIN SMALL LETTER Y WITH ACUTE
	latinC3[190]=(char)254;	//LATIN SMALL LETTER THORN
	latinC3[191]=(char)255;	//LATIN SMALL LETTER Y WITH UMLAUT

	latinC5[146]=(char)140;	//LATIN CAPITAL LETTER OE
	latinC5[147]=(char)156;	//LATIN SMALL LETTER OE
	latinC5[160]=(char)138;	//LATIN CAPITAL LETTER S CARON
	latinC5[161]=(char)154;	//LATIN SMALL LETTER S CARON
	latinC5[184]=(char)159;	//LATIN CAPITAL LETTER Y WITH DIAERESIS
	latinC5[189]=(char)141;	//LATIN CAPITAL LETTER Z CARON
	latinC5[190]=(char)158;	//LATIN SMALL LETTER Z CARON

	latinC6[146]=(char)131;	//LATIN SMALL LETTER F HOOK

	latinCB[134]=(char)136;	//CIRCUMFLEX
	latinCB[156]=(char)152;	//SMALL TILDE

	//Autres initialisations
	initVariablesSectionCritiqueGlobal();
	synGlobal.initTNEch(NM_INDEX);	//pour index sous Linux
	synGlobal.setNbIndexLec(0);
	synGlobal.setDemandeStop(false);
	synGlobal.setVolume(10);
	synGlobal.setDebit(4);
	synGlobal.setHauteur(6);
	synGlobal.setPhon(0);
	synGlobal.setModeLecture(0);
	synGlobal.setModeCompta(1);
	synGlobal.setSortieSon(1);
	synGlobal.setSortieWave(0);
	tVoix=new Voix*[1];	//une seule voix (prévu pour plusieurs)
	tVoix[0]=new Voix(0, "Michel.seg");	//construit la voix 0 (la seule)
	tab=new Tab("Synthe.tab");	//construit les tables
	synTexte("synthé prêt");
}

//Pour quitter
void quitteSynthe() {
	demandeStopEtAttendFinThread();	//stoppe le message en cours, index à -1, détruit l'objet son synSon, donc le buffer circulaire et l'objet carte-son snd-dev
	synGlobal.destTNEch();	//index sous Linux
	synGlobal.setNomFichierWave(NULL);
	detruitVariablesSectionCritiqueGlobal();
	delete tab;
	delete tVoix[0];
	delete[] tVoix;
}

#ifdef WIN32
//Initialisation chargement/déchargement la dll (pour Linux, il faut appeler initSynthe et quitteSynthe
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, void* lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		initSynthe();	//met en place les tables pour la transcription utf8-latin1, initialise les réglages et construit les tables
		break;
	case DLL_PROCESS_DETACH:
		quitteSynthe();	//arrête le son, détruit les tables et la voix
		break;
	}
	return true;
}
#endif
