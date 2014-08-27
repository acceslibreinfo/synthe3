#ifndef SYN_VOIX_H_
#define SYN_VOIX_H_

/////////////////////
// Classe Voix
/////////////////////

class Voix {
private:
	char* tSeg;	//tableau des segments concaténé
	short* tAdr;	//tableau des adresses des segments
	short nbSeg;	//nb de segments
	short nbEch;	//nb d'échantillons en tout
public:
	Voix(char nVoix, char* nomFicVoix);
	~Voix();
	char* getPtSeg(unsigned char nSeg);	// demande pointeur sur segment
};

extern Voix** tVoix;	//tableau des voix

///////////////////////
// Classe tables
///////////////////////

class Tab {
private:
	char* tTab;	//tableau des tables
	short decal;	//décalage liste des adresses des tables
	short aTWin;	//codes Windows (127 à 255) -> code commun
	short aTCat;	//code commun (32 à 217) -> catégorie
	short aTPhon1;	//code commun (97 à 122) -> code phonétique
	short aTPhon2;	//code commun (186 à 208) -> code phonétique
	short aTCatP1;	//code commun (97 à 122) -> catégorie phonétique
	short aTCatP2;	//code commun (186 à 208) -> catégorie phonétique
	short aChLst1;	//liste des unités
	short aChLst2;	//liste des centaines
	short aChLst3;	//liste des 10 à 16
	short aChLst4;	//mille, million, milliard
	short aCh01;	//"zéro"
	short aCh1S;	//"î:[n]" ("un" seul)
	short aCh100;	//"sâ[t]"
	short aCh100Z;	//"sâ:[z]"
	short aTabVol;	//table des volumes : -10 à 5 (pas 1.26 = 2 tons)
	short aTabVit;	//table des vitesses : -3 à 12 (pas 1.1225 = 1 ton)
	short aTabHau;	//table des hauteurs : -6 à 9 (pas 1.05946 = 1/2 tons)
	short aCateg;	//catégories de phonèmes
	short aFinAmp;	//n° de courbe d'amplitude de fin de phonème (début de diphone)
	short aFinTim;	//n° de segment de fin de phonème
	short aTraAmp;	//n° de courbe d'amplitude de la transition
	short aTraTim;	//n° de segment de la transition
	short aDebAmp;	//n° de courbe d'amplitude de début de phonème (début de diphone)
	short aDebTim;	//n° de segment de début de phonème
	short aTabAmp;	//adresse courbe d'amplitude -> n° courbe d'amplitude
	short aAdAmp;	//n° courbe d'amplitude -> adresse courbe d'amplitude
	short aTabDeb;	//table des adresses pour début de mot (32 à 217)
	short a_Huit1;	//adresse transcription
	short a_Dix1;	//adresse transcription
	short aXChif;	//adresse transcription
	short lgTab;	//taille totale des tables

public:
	Tab(char* nomFicTab);
	~Tab();
	unsigned char tWin(unsigned char carac);	//codes Windows (127 à 255) -> code commun
	char tCat(unsigned char c);	//code commun (32 à 217) -> catégorie
	char tPhon(unsigned char c);	//code commun (97 à 122)(186 à 208) -> code phonétique
	char tCatP(unsigned char c);	//code commun (97 à 122)(186 à 208) -> catégorie phonétique
	char* chListUnit(char c);	//liste des unités jusqu'à 16
	char* chListDiz(char c);	//liste des dizaines
	char* chListMil(char n);	//mille, million, milliard
	char* ch01();	//"zéro"
	char* ch1S();	//"î:[n]" ("un" seul)
	char* ch100();	//"sâ[t]"
	char* ch100Z();	//"sâ:[z]"
	short tabVol(char n);	//table des volumes : -10 à 5 (pas 1.26 = 2 tons)
	short tabVit(char n);	//table des vitesses : -3 à 12 (pas 1.1225 = 1 ton)
	short tabHau(char n);	//table des hauteurs : -6 à 9 (pas 1.05946 = 1/2 tons)
	char categ(char phon);	//catégories de phonèmes
	unsigned char finAmp(char catG, char catD);	//n° de courbe d'amplitude de fin de phonème (début de diphone)
	unsigned char finTim(char phon);	//n° de segment de fin de phonème
	unsigned char traAmp(char catG, char catD);	//n° de courbe d'amplitude de la transition
	unsigned char traTim(char phonG, char phonD);	//n° de segment de la transition
	unsigned char debAmp(char catG, char catD);	//n° de courbe d'amplitude de début de phonème (début de diphone)
	unsigned char debTim(char phon);	//n° de segment de début de phonème
	unsigned char* getPtAmp(short nAmp);	//n° courbe d'amplitude -> pt courbe d'amplitude
	char* getPtTabDeb(unsigned char c);	//pt table des adresses pour début de mot (32 à 189)
	char* getPtArbre(char* ptArbre);	//prépare adresse suivante
	char* getPtArbre(char* ptArbre, unsigned char carac);	//prépare adresse suivante
	short tabDeb(unsigned char c);	//table des adresses pour début de mot (32 à 217)
	char* _Huit1();	//pt transcription
	char* _Dix1();	//pt transcription
	char* XChif();	//pt transcription
	unsigned char FOIS;	//symbole pour "fois"
};

extern Tab* tab;	//les tables de Synthé

#endif
