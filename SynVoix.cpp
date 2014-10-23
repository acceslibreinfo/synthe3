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

#include "SynVoix.h"
#include "SynMain.h"

///////////////////////////////////////////
// Méthodes de la classe Voix
///////////////////////////////////////////

Voix::Voix(char nVoix, const char* nomFicVoix) {	//constructeur : charge la voix
	//Charge voix Michel
	ifstream ficVoix(nomFicVoix, ios::binary);
	ficVoix.read((char*)&nbSeg, 2);
	ficVoix.read((char*)&nbEch, 2);
	tAdr=new short[nbSeg];
	tSeg=new char[nbEch];
	ficVoix.read((char*)tAdr, nbSeg*2);
	ficVoix.read(tSeg, nbEch);
	ficVoix.close ();
}

Voix::~Voix() {	//destructeur
	delete[] tAdr;
	delete[] tSeg;
}

//Retourne un pointeur sur le segment
char* Voix::getPtSeg(unsigned char nSeg) {
	return tSeg+tAdr[nSeg];
}

//////////////////////////////////////////
// Méthodes de la classe Tab
//////////////////////////////////////////

Tab::Tab(const char* nomFicTab) {
	//Charge tables
	ifstream ficTab(nomFicTab, ios::binary);
	ficTab.read((char*)&aTWin, 2);
	ficTab.read((char*)&aTCat, 2);
	ficTab.read((char*)&aTPhon1, 2);
	ficTab.read((char*)&aTPhon2, 2);
	ficTab.read((char*)&aTCatP1, 2);
	ficTab.read((char*)&aTCatP2, 2);
	ficTab.read((char*)&aChLst1, 2);
	ficTab.read((char*)&aChLst2, 2);
	ficTab.read((char*)&aChLst3, 2);
	ficTab.read((char*)&aCh01, 2);
	ficTab.read((char*)&aCh1S, 2);
	ficTab.read((char*)&aCh100, 2);
	ficTab.read((char*)&aCh100Z, 2);
	ficTab.read((char*)&aTabVol, 2);
	ficTab.read((char*)&aTabVit, 2);
	ficTab.read((char*)&aTabHau, 2);
	ficTab.read((char*)&aCateg, 2);
	ficTab.read((char*)&aFinAmp, 2);
	ficTab.read((char*)&aFinTim, 2);
	ficTab.read((char*)&aTraAmp, 2);
	ficTab.read((char*)&aTraTim, 2);
	ficTab.read((char*)&aDebAmp, 2);
	ficTab.read((char*)&aDebTim, 2);
	ficTab.read((char*)&aTabAmp, 2);
	ficTab.read((char*)&aAdAmp, 2);
	ficTab.read((char*)&aTabDeb, 2);
	ficTab.read((char*)&a_Huit1, 2);
	ficTab.read((char*)&a_Dix1, 2);
	ficTab.read((char*)&aXChif, 2);
	ficTab.read((char*)&lgTab, 2);
	ficTab.read((char*)&FOIS, 1);
	decal=(short)ficTab.tellg();
	tTab=new char[lgTab];
	ficTab.read(tTab, lgTab);
	ficTab.close ();
	tTab-=decal;	//décalage liste des adresses
}

Tab::~Tab() {
	tTab+=decal;
	delete[] tTab;
}

//codes Windows (127 à 255) -> code commun
unsigned char Tab::tWin(unsigned char carac) {
	if (carac<32) carac=32;
	if (carac>='a' && carac<='z') carac-=32;
	if (carac<123) return carac;
	return ((unsigned char*)tTab+aTWin)[carac-123];
}

//code commun (32 à 189) -> catégorie
char Tab::tCat(unsigned char c) {
	if (c<32 || c>FOIS) return PONC;
	return (tTab+aTCat)[c-32];
}

//code commun (97 à 122)(FOIS-30 à FOIS-8) -> code phonétique
char Tab::tPhon(unsigned char c) {
	if (c==',') return VR;
	if (c==':') return PL;
	if (c<65) return NULP;
	if (c<91) return (tTab+aTPhon1)[c-65];
	if (c<97) return NULP;
	if (c<123) return (tTab+aTPhon1)[c-97];
	if (c==174) return UU;
	if (c<FOIS-30) return NULP;
	if (c<FOIS-7) return (tTab+aTPhon2)[c-FOIS+30];
	return NULP;
}

//code commun (97 à 122)(FOIS-30 à FOIS-8) -> catégorie phonétique
char Tab::tCatP(unsigned char c) {
	if (c<97) return NULP;
	if (c<123) return (tTab+aTCatP1)[c-97];
	if (c<FOIS-30) return NULP;
	if (c<FOIS-7) return (tTab+aTCatP2)[c-FOIS+30];
	return NULP;
}

//liste des unités jusqu'à 16
char* Tab::chListUnit(char c) {
	return tTab+((short*)(tTab+aChLst1))[c-48];
}

//liste des dizaines
char* Tab::chListDiz(char c) {
	return tTab+((short*)(tTab+aChLst2))[c-49];
}

//mille, million, milliard
char* Tab::chListMil(unsigned short int n) {
	return tTab+((short*)(tTab+aChLst3))[n];
}

//"zéro"
char* Tab::ch01() {
	return tTab+aCh01;
}

//"î:[n]" ("un" seul)
char* Tab::ch1S() {
	return tTab+aCh1S;
}

//"sâ[t]"
char* Tab::ch100() {
	return tTab+aCh100;
}

//"sâ:[z]"
char* Tab::ch100Z() {
	return tTab+aCh100Z;
}

//table des volumes : -10 à 5 (pas 1.26 = 2 tons)
short Tab::tabVol(char n) {
	return ((short*)(tTab+aTabVol))[n+10];
}

//table des vitesses : -3 à 12 (pas 1.1225 = 1 ton)
short Tab::tabVit(char n) {
	return ((short*)(tTab+aTabVit))[n+3];
}

//table des hauteurs : -6 à 9 (pas 1.05946 = 1/2 tons)
short Tab::tabHau(char n) {
	return ((short*)(tTab+aTabHau))[n+6];
}

//catégories de phonèmes
char Tab::categ(unsigned short int phon) {
	return (tTab+aCateg)[phon];
}

//n° de courbe d'amplitude de fin de phonème (début de diphone)
unsigned char Tab::finAmp(char catG, char catD) {
	return (tTab+aFinAmp)[catG*10+catD];
}

//n° de segment de fin de phonème
unsigned char Tab::finTim(unsigned short int phon) {
	return ((unsigned char*)tTab+aFinTim)[phon];
}

//n° de courbe d'amplitude de la transition
unsigned char Tab::traAmp(char catG, char catD) {
	return (tTab+aTraAmp)[catG*10+catD];
}

//n° de segment de la transition
unsigned char Tab::traTim(char phonG, char phonD) {
	return ((unsigned char*)tTab+aTraTim)[phonG*32+phonD];
}

//n° de courbe d'amplitude de début de phonème (début de diphone)
unsigned char Tab::debAmp(char catG, char catD) {
	return (tTab+aDebAmp)[catG*10+catD];
}

//n° de segment de début de phonème
unsigned char Tab::debTim(unsigned short int phon) {
	return ((unsigned char*)tTab+aDebTim)[phon];
}

//n° courbe d'amplitude -> pt courbe d'amplitude
unsigned char* Tab::getPtAmp(short nAmp) {
	short adr=((short*)(tTab+aAdAmp))[nAmp];
	return (unsigned char*)(tTab+aTabAmp+adr);
}

//pt table des adresses pour début de mot (32 à 189)
char* Tab::getPtTabDeb(unsigned char c) {
	short adr=((short*)(tTab+aTabDeb))[c-32];
	return tTab+adr;
}

//Prépare adresse suivante (branche)
char* Tab::getPtArbre(char* ptArbre) {
	short adr=*((short*)ptArbre);
	return tTab+adr;
}

//Prépare adresse suivante selon carac (racine)
char* Tab::getPtArbre(char* ptArbre, unsigned char carac) {
	short adr=*((short*)ptArbre);
	adr=((short*)(tTab+adr))[carac-32];
	return tTab+adr;
}

//adresse transcription
char* Tab::_Huit1() {
	return tTab+a_Huit1;
}

//adresse transcription
char* Tab::_Dix1() {
	return tTab+a_Dix1;
}

//adresse transcription
char* Tab::XChif() {
	return tTab+aXChif;
}

