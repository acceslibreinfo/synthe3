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

#ifndef __SYN_MAIN_H__
#define __SYN_MAIN_H__

//Déclarations communes aux fichiers de Synthé

#include "SynGlobal.h"
#ifdef WIN32
	#include <windows.h>	//nécessaire pour Synthé
#else
	#define LPVOID void*
	#define DWORD int
	#define HANDLE pthread_t
#endif
 
//Constantes
//Constantes pour SynSon
#define MONO 1
#define STEREO 2
//Taille max texte
#define NM_CAR_TEX 10000
#define NM_CAR_TEX_1 NM_CAR_TEX-1
#define NM_CAR_TEX_2 NM_CAR_TEX-2
#define NM_CAR_TEX_8 NM_CAR_TEX-8
//Constantes pour conversion alpha-phonèmes
#define BVRD 1	//teste le mode bavard
#define NABR 2	//teste le mode non abréviations
//Catégories (attention : ds l'arbre la catégorie est testée par < ou =)
#define VOYM 3	//voyelle "mouillée"
#define VOY 4	//voyelle
#define LET 5	//lettre
#define CNS 5	//lettre
#define PONC 6	//ponctuation
#define SYMB 7	//symbole
#define CHIF 8	//chiffre
#define TTT 8	//tout
//Terminateurs pour arbre (alpha-phonèmes)
#define PG 80	//programme (retour au cas général)
#define RB 81	//arbre
#define DC 82	//dictionnaire
//Catégories phonétiques
#define VOYP 0	//voyelles
#define GLIP 1	//glissantes
#define CNSP 2	//consonnes
//Phonèmes
#define OU 0
#define WW 0
#define OO 1
#define UU 2
#define II 3
#define AA 4
#define AN 5
#define ON 6
#define EU 7
#define EE 8
#define UN 9
#define YY 10
#define FF 11
#define SS 12
#define HH 13
#define VV 14
#define ZZ 15
#define JJ 16
#define PP 17
#define TT 18
#define KK 19
#define BB 20
#define DD 21
#define GG 22
#define MM 23
#define NN 24
#define LL 25
#define RR 26
#define VR 27	//virgule phon
#define PL 28	//allongement
#define INDEX '_'
#define NULP -2	//nul phon

//Tableaux nécessaire pour pouvoir retrouver les valeurs définies à l'extérieur du thread
extern HANDLE hThread;
extern short posLec[NM_INDEX+1];	//il y a une place de plus que d'index
extern short nbIndex;

//Structures
struct typeParamThread {
	char* texte;	//peut être constitué de plusieurs paragraphes, sans dépasser NM_CAR_TEX caractères.
	short phon; 	//1, le texte est phonétique
	short volume;	//0 à 15 par pas de 25 % (par défaut 10)
	short debit;	//0 à 15 par pas de 12 % (par défaut 4)
	short hauteur;	//0 à 15 par pas de 12 % (par défaut 4)
	short modeLecture;	//0, normal, 1, dit la ponctuation
	short modeCompta;	//0, le séparateur de milliers reste, 1, le séprateur de milliers est enlevé
	short sortieSon;	//sortie sur la carte-son
	short sortieWave;		//sortie sous forme de fichier wave
	char* nomFicWave;	//nom éventuel du fichier à construire
};

//Variables pour la conversion UTF-8 latin1
extern char latinC2[256];
extern char latinC3[256];
extern char latinC5[256];
extern char latinC6[256];
extern char latinCB[256];

//Fonctions externes de SynTex.cpp
extern void synTex(void* lpParam);

//Fonctions externes de SynParle.cpp
extern void initWave(bool init);
extern void sonDestruction();

//Fonction externe de Synthe.cpp
extern bool UTF8Latin1(char* chLec, char* chEcr);

//Fonctions privées de Synthe.cpp
void fThAlpha(void*);
void copieEtAjouteIndexSiPas (char* chaineLec, char* chaineEcr);
bool caracValide(char carac);
void demandeStopEtAttendFinThread();

#endif
