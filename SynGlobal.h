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

#ifndef _SYN_GLOBAL_H_
#define _SYN_GLOBAL_H_

//Classe des globales (constantes, section critique, index, stop, réglages et paramètres)
//L'objet synGlobal représente l'état de Synthé à tout moment

#include <iostream>
#include <fstream>
/*#ifdef LINUX
	#include "port_linux.h"
#endif	
*/
using namespace std;

//Constantes
#define REGLAGE_VOLUME_MIN 0
#define REGLAGE_VOLUME_MAX 15
#define REGLAGE_VOLUME_INC 1
#define REGLAGE_VOLUME_DEFAUT 10
#define REGLAGE_DEBIT_MIN 0
#define REGLAGE_DEBIT_MAX 15
#define REGLAGE_DEBIT_INC 1
#define REGLAGE_DEBIT_DEFAUT 5
#define REGLAGE_HAUTEUR_MIN 0
#define REGLAGE_HAUTEUR_MAX 15
#define REGLAGE_HAUTEUR_INC 1
#define REGLAGE_HAUTEUR_DEFAUT 6
//Index
#define NM_INDEX 2500	//si " a b c ..."
#define MARQ_MARQ ((char) 248) /* LATIN SMALL LETTER O WITH STROKE */
#define MARQ_INDEX ((char) 237) /* LATIN SMALL LETTER I WITH ACUTE */

//Classe Global : section critique, index, stop, réglages et paramètres
class Global {
private:
	short nbIndexLec;
	short nbIndexEcr;
	bool demandeStop;
	bool threadOK;
	short volume;
	short debit;
	short hauteur;
	short phonetique;
	short modeLecture;
	short modeCompta;
	short sortieSon;
	short sortieWave;
	char* nomFichierWave;

	//Pour l'index sous linux
	short nbIndexMax;
	int ktime0us;
	int ktime0s;
	int* tNEch;	//repère le nb d'échantillons fabriqués depuis le début pour chaque index rencontré
	int ctEch;

public:
	//Index et stop
	short getNbIndexLec();
	void setNbIndexLec(short nbInd);
	short getNbIndexEcr();
	void setNbIndexEcr(short nbInd);
	bool getDemandeStop();
	void setDemandeStop(bool demStop);
	bool getThreadOK();
	void setThreadOK(bool thOK);

	//Pour index sous linux
	short getNbIndexMax();
	void setNbIndexMax(short nbInd);
	int getktime0us();
	void setktime0us(int);
	int getktime0s();
	void setktime0s(int);
	void initTNEch(short);
	void destTNEch(void);
	int getTNEch(short);
	void setTNEch(short, int);
	int getCtEch(void);
	void setCtEch(int);
	void incrCtEch(void);

	//Réglages
	short getVolume();
	void setVolume(short vol);
	short getDebit();
	void setDebit(short deb);
	short getHauteur();
	void setHauteur(short hau);

	//Paramètres de lecture
	short getPhon();
	void setPhon(short phon);
	short getModeLecture();
	void setModeLecture(short modeLec);
	short getModeCompta();
	void setModeCompta(short modeComp);

	//Paramètres de son
	void setSortieSon(short sortSon);
	short getSortieSon();
	void setSortieWave(short sortWave);
	short getSortieWave();
	void setNomFichierWave(char* nomFicWave);
	char* getNomFichierWave();
};

extern Global synGlobal;	//le seul objet Global

//Section critique
extern void entreSectionCritiqueGlobal();
extern void quitteSectionCritiqueGlobal();
extern void entreSectionCritiqueGlobal2();
extern void quitteSectionCritiqueGlobal2();
extern void entreSectionCritiqueGlobal3();
extern void quitteSectionCritiqueGlobal3();
extern void initVariablesSectionCritiqueGlobal();
extern void detruitVariablesSectionCritiqueGlobal();

#endif
