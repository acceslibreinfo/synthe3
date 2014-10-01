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

#ifndef ___SYNTHE_H__
#define ___SYNTHE_H__

#ifndef EXTERNE
	#ifdef WIN32
		#define EXTERNE extern "C" _declspec(dllimport)
	#else // linux ...
		#define EXTERNE extern "C"
	#endif
#endif

#ifndef WIN32
	#define _stdcall
#endif

//////////////////////////////////////////////////////////////////
// Fonctions publiques pour faire parler Synthé
//////////////////////////////////////////////////////////////////

//Envoi d'un texte à lire par Synthé
//Le paramètre texte est obligatoire, les autres sont facultatifs, la valeur -1 indique la conservation de la valeur courante
EXTERNE void _stdcall synTexte(
	char* texte,	//peut être constitué de plusieurs paragraphes, sans dépasser NM_CAR_TEX caractères.
	short volume=-1,	//0 à 15 par pas de 25 % (par défaut 10) (-1 indique inchangé)
	short debit=-1,	//0 à 15 par pas de 12 % (par défaut 4)
	short hauteur=-1,	//0 à 15 par pas de 12 % (par défaut 4)
	short phon=-1, 	//1, le texte est phonétique
	short modeLec=-1,	//0 à 15 par pas de 6 % (par défaut 6)
	short compta=-1,	//0, le séparateur de milliers reste, 1, le séprateur de milliers est enlevé
	short son=-1,		//sortie sur la carte-son
	short wave=-1,		//1, sortie sous forme de fichier wave
	char* nomWave=NULL	//nom éventuel du fichier à construire
	);

//Retourne la valeur de l'index de lecture (va du nb d'index à 0 en fin de lecture)
EXTERNE short _stdcall synIndex();

//Retourne la position de lecture du texte (indice du caractère en cours de lecture)
EXTERNE short _stdcall synPosLec();

//Stop parole : arrête la lecture (effet immédiat)
//Indispensable quand on arrête un programme utilisant Synthé sous peine de bouclage irréversible du tampon de lecture
EXTERNE void _stdcall synStop();

//Initialisation de Synthe
void initSynthe();	//init section critique, réglages, voix, tables (au démarrage de l'utilisation de Synthé)
void quitteSynthe();	//stoppe parole, détruit tout (nécessaire pour quitter Synthé avant d'arrêter l'application)

#endif
