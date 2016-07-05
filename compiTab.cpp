#include <fstream>
#include <stdlib.h>
#include "stdafx.h"

//Constantes
const int M_SYMB=5000;

//Structure Symbole
struct Symbole {
	char* nom;
	short valeur;
	short nbDef;
	short nbRef;
} tSymb[M_SYMB];	//table des symboles

//Variable globale
short nbSymb;

//Compile ficEntree en fournissant ficSortie
void compiTab(char* nomFicEntree, char* nomFicSortie, int nPasse);
//Fabrique le mot suivant sur la ligne
void motSuiv(char* lin, char* mot, short& i);
//Définit ou référence un symbole à partir du mot
void motSymb(char* mot, short& nSymb);
//Teste si le mot est un nombre et le calcule
bool nombre(char* mot, short num, short& n);

//Programme principal
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {
	short iSymb;

//	if (lpCmdLine!=NULL && *lpCmdLine!=0) compiTab (lpCmdLine);
//	else 
	nbSymb=0;
	compiTab ("synthe.txt", "bilan.txt", 1);
	compiTab ("synthe.txt", "synthe.tab", 2);
	for (iSymb=0; iSymb<nbSymb; iSymb++)
		delete[] tSymb[iSymb].nom;
	return 0;
}

//Compile le fichier tables
void compiTab(char* nomFicEntree, char* nomFicSortie, int nPasse) {
	char lin[200];
	char mot[100];
	short iCarLin;
	short iCarMot;
	short nSymb;
	short iSymb;
	short constante;
	short adresse;
	short nOct;	//1 (DB) ou 2 (DW)
	short num;	//numération 10 ou 16
	short n;
	char texNbSymb[6];

	constante=0;
	adresse=0;
	num=10;
	std::ifstream ficEntree(nomFicEntree, std::ios::binary);
	std::ofstream ficSortie(nomFicSortie, std::ios::binary);
	while (!ficEntree.eof()) {
		ficEntree.getline(lin, 200);
		for (iCarLin=0; (unsigned)iCarLin<strlen(lin)-1; ) {
			motSuiv(lin, mot, iCarLin);	//extrait le mot suivant de la ligne
			if (*mot==0 || *mot==';')
				break;
			else if (*mot=='#') {
				if (mot[1]=='H')
					num=16;	//numérotation hexa
				else
					num=10;	//numérotation décimale
			} else if (!strcmp(mot, "DB") || !strcmp(mot, "DW")) {	//ligne commençant par le format (DB ou DW)
				if (mot[1]=='B')
					nOct=1;	//variable byte (1 octet)
				else
					nOct=2;	//variable word (2 octets)
				while (true) {	//lit les mots l'un après l'autre
					motSuiv(lin, mot, iCarLin);	//le mot suivant
					if (*mot==0 || *mot==';')	//terminé en fin de ligne ou au début d'un commentaire
						break;
					if (nombre(mot, num, n)) {	//soit c'est un nombre
						if (nPasse==2)
							ficSortie.write((char*) &n, nOct);	//écrit le nombre (byte ou word)
						if (adresse<32000)
							adresse+=nOct;
					} else if (*mot==39) {	//soit c'est une chaîne de caractères ('Copyright')
						for (iCarMot=1; (unsigned)iCarMot<strlen(mot)-1; iCarMot++) {
							n=mot[iCarMot];
							if (nPasse==2)
								ficSortie.write((char*) &n, nOct);	//écrit chaque carac de la chaine
							if (adresse<32000)
								adresse+=nOct;
						}
					} else {	//soit c'est un symbole
						motSymb(mot, nSymb);	//définit ou référence un symbole à partir du mot
						tSymb[nSymb].nbRef++;	//référence le symbole
						n=tSymb[nSymb].valeur;
						if (nPasse==2)
							ficSortie.write((char*) &n, nOct);	//écrit la valeur du symbole (byte ou word)
						if (adresse<32000)
							adresse+=nOct;
					}
				}
				if (*mot==0 || *mot==';')
					break;
			} else if (mot[strlen(mot)-1]==':') {	//si c'est une étiquette
				mot[strlen(mot)-1]=0;
				if (*mot==0)
					break;	//':'
				motSymb(mot, nSymb);
				tSymb[nSymb].nbDef++;	//l'étiquette définit un nouveau symbole (truc:)
				tSymb[nSymb].valeur=adresse;	//et sa valeur
			} else {	//sinon, c'est un symbole
				motSymb(mot, nSymb);
				tSymb[nSymb].nbDef++;	//un nouveau symbole est défini par une égalité (truc =)
				motSuiv(lin, mot, iCarLin);
				if (*mot==0 || *mot==';')	//fin de ligne ou début de commentaire
					break;
				if (*mot=='=') {
					motSuiv(lin, mot, iCarLin);
					if (*mot==0 || *mot==';')	//fin de ligne ou début de commentaire : égalité sans 2e terme
						tSymb[nSymb].valeur=++constante;	//on incrémente la valeur implicite de la constante
					else {
						nombre(mot, num, constante);
						tSymb[nSymb].valeur=constante;	//on attribue la valeur de la constante
					}
				}
				break;
			}
		}
	}
	if (nPasse==1) {
		//1ère passe : on fait le bilan
		ficSortie.write("nbSymb = ", 9);
		sprintf(texNbSymb, "%d", nbSymb);
		ficSortie.write(texNbSymb, strlen(texNbSymb));	//nb de symboles
		ficSortie.write("\n", 1);
		for (iSymb=0; iSymb<nbSymb; iSymb++) {
			if (tSymb[iSymb].nbDef==0) {	//symbole non défini (= erreur)
				ficSortie.write("non défini : ", 13);
				ficSortie.write(tSymb[iSymb].nom, strlen(tSymb[iSymb].nom));
				ficSortie.write("\n", 1);
			} else if (tSymb[iSymb].nbDef>1) {	//symbole défini plusieurs fois (= erreur probable)
				ficSortie.write("multi défini : ", 15);
				ficSortie.write(tSymb[iSymb].nom, strlen(tSymb[iSymb].nom));
				ficSortie.write("\n", 1);
			} else if (tSymb[iSymb].nbRef==0) {	//symbole non référencé (= warning)
				ficSortie.write("non référencé : ", 16);
				ficSortie.write(tSymb[iSymb].nom, strlen(tSymb[iSymb].nom));
				ficSortie.write("\n", 1);
			}
		}
	}
	ficEntree.close();
	ficSortie.close ();
}

//Fabrique le mot suivant sur la ligne
void motSuiv(char* lin, char* mot, short& iCarLin) {
	char c;
	short iCarMot=0;
	bool apost;	//attend la 2ème apostrophe

	apost=false;
	while ((unsigned)iCarLin<strlen(lin)-1) {
		c=lin[iCarLin++];
		if ((c==' ' || c==9) && !apost) {
			if (iCarMot==0)
				continue;
			else
				break;
		} else if (c==39) {	// '
			apost=!apost;
		}
		else if (c==',' && !apost)
			break;
		if (c>='a' && c<='z' && !apost)
			c-=32;
		mot[iCarMot++]=c;
	}
	mot[iCarMot]=0;
}

//Définit ou référence un symbole à partir du mot
void motSymb(char* mot, short& nSymb) {
	short iSymb;

	for (iSymb=0; iSymb<nbSymb; iSymb++) {
		if (!strcmp(mot, tSymb[iSymb].nom))
			break;	//Le symbole existe déjà
	}
	if (iSymb==nbSymb) {
		//Nouveau symbole
		if (nbSymb<M_SYMB)
			nbSymb++;
		iSymb=nbSymb-1;
		tSymb[iSymb].nom=new char[strlen(mot)+1];
		strcpy(tSymb[iSymb].nom, mot);
		tSymb[iSymb].nbDef=0;
		tSymb[iSymb].nbRef=0;
		tSymb[iSymb].valeur=0;
	}
	nSymb=iSymb;
}

//Teste si le mot est un nombre et le calcule
bool nombre(char* mot, short num, short& n) {
	short i;
	short d;

	if (mot[0]<'0' || mot[0]>'9')
		return false;
	n=0;
	for (i=0; (unsigned)i<strlen(mot); i++) {
		d=mot[i];
		if (d>'9')
			d-=7;	//A à F
		n=n*num+d-'0';
	}
	return true;
}
