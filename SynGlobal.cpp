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

//Classe Global : section critique, index, stop, réglages et paramètres

#include <string.h>
#ifdef WIN32
	#include <afxdisp.h>
#endif

#include "SynGlobal.h"

/////////////////////////////////
// variables et fonctions
/////////////////////////////////

Global synGlobal;	//le seul objet Global

//Index de lecture
short Global::getNbIndexLec() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short nbInd=nbIndexLec;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return nbInd;
}
void Global::setNbIndexLec(short nbInd) {
	if (nbInd<1) nbInd++;	//0 -> 1 pas encore fini, -1 -> 0 fini
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	nbIndexLec=nbInd;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}
short Global::getNbIndexEcr() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short nbInd=nbIndexEcr;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return nbInd;
}
void Global::setNbIndexEcr(short nbInd) {
	if (nbInd<1) nbInd++;	//min 1 pour ignorer le dernier index (repoussé à la fin car il reste un peu de signal derrière)
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	nbIndexEcr=nbInd;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

short Global::getNbIndexMax() {	//Romain pour index sous linux
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short nbInd=nbIndexMax;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return nbInd;
}
void Global::setNbIndexMax(short nbInd) {	//Romain pour index sous linux
	if (nbInd<1) nbInd++;
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	nbIndexMax=nbInd;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

//Stop
bool Global::getDemandeStop() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	bool demStop=demandeStop;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return demStop;
}
void Global::setDemandeStop(bool demStop) {
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	demandeStop=demStop;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

//Contrôle le passage des paramètres vers les threads (pour debug...)
bool Global::getThreadOK() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	bool thOK=threadOK;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return thOK;
}

void Global::setThreadOK(bool thOK) {
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	threadOK=thOK;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

//Réglages
short Global::getVolume() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short vol=volume;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return vol;
}
void Global::setVolume(short vol) {
	if (vol==-1) return;
	if (vol<REGLAGE_VOLUME_MIN) vol=REGLAGE_VOLUME_MIN;
	else if (vol>REGLAGE_VOLUME_MAX) vol=REGLAGE_VOLUME_MAX;
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	volume=vol;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

short Global::getDebit() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short deb=debit;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return deb;
}
void Global::setDebit(short deb) {
	if (deb==-1) return;
	if (deb<REGLAGE_DEBIT_MIN-2) deb=REGLAGE_DEBIT_MIN-2;
	else if (deb>REGLAGE_DEBIT_MAX) deb=REGLAGE_DEBIT_MAX;
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	debit=deb;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

short Global::getHauteur() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short hau=hauteur;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return hau;
}
void Global::setHauteur(short hau) {
	if (hau==-1) return;
	if (hau<REGLAGE_HAUTEUR_MIN) hau=REGLAGE_HAUTEUR_MIN;
	else if (hau>REGLAGE_HAUTEUR_MAX) hau=REGLAGE_HAUTEUR_MAX;
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	hauteur=hau;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

//Paramètres de lecture
short Global::getPhon() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short phon=phonetique;	//pas bool car -1 possible pour ignorer
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return phon;
}
void Global::setPhon(short phon) {
	if (phon==-1) return;
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	phonetique=phon;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

short Global::getModeLecture() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short modeLec=modeLecture;	//pas bool car -1 possible pour ignorer
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return modeLec;
}
void Global::setModeLecture(short modeLec) {
	if (modeLec==-1) return;
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	modeLecture=modeLec;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

short Global::getModeCompta() {
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	short modeComp=modeCompta;	//pas bool car -1 possible pour ignorer
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return modeComp;
}
void Global::setModeCompta(short modeCompt) {
	if (modeCompt==-1) return;
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	modeCompta=modeCompt;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

//Paramètres de son
short Global::getSortieSon() {	//tj ds le thread de l'instance : pas de section critque
	return sortieSon;
}
void Global::setSortieSon(short sortSon) {	//thread de l'instance arrêté : pas de section critique
	if (sortSon==-1) return;
	sortieSon=sortSon;
}

short Global::getSortieWave() {	//tj ds le thread de l'instance : pas de section critque
	return sortieWave;
}
void Global::setSortieWave(short sortWave) {	//thread de l'instance arrêté : pas de section critique
	if (sortWave==-1) return;
	sortieWave=sortWave;
}

char* Global::getNomFichierWave() {	//tj ds le thread de l'instance : pas de section critque
	return nomFichierWave;
}
void Global::setNomFichierWave(char* nomFicWave) {	//thread de l'instance arrêté : pas de section critique
	if (nomFichierWave)
		delete[] nomFichierWave;
	if (nomFicWave==NULL)
		return;
	nomFichierWave=new char[strlen((char*)nomFicWave)+1];
	strcpy(nomFichierWave, nomFicWave);
}

//Pour index sous Linux
int Global::getktime0us() {	//le set est dans le même thread -> pas de section critique
	return ktime0us;
}
void Global::setktime0us(int a) {	//le get est dans le même thread -> pas de section critique
	ktime0us=a;
}
int Global::getktime0s() {	//le set est dans le même thread -> pas de section critique
	return ktime0s;
}
void Global::setktime0s(int a) {	//le get est dans le même thread -> pas de section critique
	ktime0s=a;
}
void Global::initTNEch(short a) {	//avant le message de début -> pas de section critique
	if (a>0 && a<=NM_INDEX) {	//il y a une place de plus que d'index
		if (tNEch!=NULL) {
			delete tNEch;
			tNEch=NULL;
		}
		tNEch=new int[a];
	}
}
void Global::destTNEch(void) {	//après la fin -> pas de section critique
	delete[] tNEch;
	tNEch=NULL;
}
int Global::getTNEch(short a) {
	int b;
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	if (a>=0 && a<=NM_INDEX)	//il y a une place de plus que d'index
		b=tNEch[a];
	else
		b=-1;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return b;
}
void Global::setTNEch(short a, int b) {
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	if (a>=0 && a<=NM_INDEX)	//il y a une place de plus que d'index
		tNEch[a]=b;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}
int Global::getCtEch(void) {
	int a;
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	a=ctEch;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
	return a;
}
void Global::setCtEch(int a) {
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	ctEch=a;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}
//Incrémente le compteur d'échantillons pour gérer le tableau d'index (position de lecture sous Linux)
void Global::incrCtEch(void) {
	entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
	ctEch+=1;
	quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
}

//Fonctions globales
//Section critique
#ifdef WIN32
CRITICAL_SECTION gVarBuffer;
CRITICAL_SECTION gVarBuffer2;
CRITICAL_SECTION gVarBuffer3;
#else
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
#endif
void entreSectionCritiqueGlobal() {
#ifndef NOCRITICAL
#ifdef WIN32
	EnterCriticalSection(&gVarBuffer);
#else
	pthread_mutex_lock(&mutex);
#endif
#endif
}
void quitteSectionCritiqueGlobal() {
#ifndef NOCRITICAL
#ifdef WIN32
	LeaveCriticalSection(&gVarBuffer);
#else
	pthread_mutex_unlock(&mutex);
#endif
#endif
}

void entreSectionCritiqueGlobal2() {
#ifndef NOCRITICAL
#ifdef WIN32
	EnterCriticalSection(&gVarBuffer2);
#else
	pthread_mutex_lock(&mutex2);
#endif
#endif
}
void quitteSectionCritiqueGlobal2() {
#ifndef NOCRITICAL
#ifdef WIN32
	LeaveCriticalSection(&gVarBuffer2);
#else
	pthread_mutex_unlock(&mutex2);
#endif
#endif
}
 
void entreSectionCritiqueGlobal3() {
#ifndef NOCRITICAL
#ifdef WIN32
	EnterCriticalSection(&gVarBuffer3);
#else
	pthread_mutex_lock(&mutex3);
#endif
#endif
}
void quitteSectionCritiqueGlobal3() {
#ifndef NOCRITICAL
#ifdef WIN32
	LeaveCriticalSection(&gVarBuffer3);
#else
	pthread_mutex_unlock(&mutex3);
#endif
#endif
}
  
void initVariablesSectionCritiqueGlobal() {
#ifndef NOCRITICAL
#ifdef WIN32
	InitializeCriticalSection(&gVarBuffer);
	InitializeCriticalSection(&gVarBuffer2);
	InitializeCriticalSection(&gVarBuffer3);
#else
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex2, NULL);
	pthread_mutex_init(&mutex3, NULL);
#endif
#endif
}

void detruitVariablesSectionCritiqueGlobal() {
#ifndef NOCRITICAL
#ifdef WIN32
	DeleteCriticalSection(&gVarBuffer);
	DeleteCriticalSection(&gVarBuffer2);
	DeleteCriticalSection(&gVarBuffer3);
#else
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mutex2);
	pthread_mutex_destroy(&mutex3);
#endif
#endif
}

//xxx faire tourner
//xxx faire un main comme Kali
//xxx faire tourner
