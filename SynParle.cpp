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

///////////////////////////////////
// Lit le texte phonétique
///////////////////////////////////

//Procédure de fabrication de la parole à partir du texte phonétique, sans prosodie

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SynSon.h"
#include "SynMain.h"
#include "SynCalcul.h"
#include "SynParle.h"
#include "SynVoix.h"

////////////////////////////////////////////
// Constantes de synParle.cpp
////////////////////////////////////////////
#define FREQ_ECH 10000
#define FREQ_COUPURE 400	//fréquence de coupure basse
#define PI 3.14159
#define TYPE_ECH 16
#define MUL_AMP 8	//pour max moitié pleine échelle
#define MS_UNSPEC 0x7ffff000 // magic length pour wave vers stdout

////////////////////////////
//Variables globales
////////////////////////////

extern classSon* synSon;
long longWave;	//longueur du wave
FILE* ficWave;
bool ficWaveFerme;
long echE_Prec, echS_Prec;	//échantillons précédents entrée et sortie du filtre

////////////////////////////////////////////
// Fonctions de la classe Parle
////////////////////////////////////////////

//Fonction principale : prononce le texte phonétique reçu
void Parle::traiteTextePhonetique(char* chainePhon) {

	echE_Prec=echS_Prec=32768;	//init car utilisés dès le 1er échantillon
	sortieWave=synGlobal.getSortieWave();
	sortieSon=synGlobal.getSortieSon();
	mulHauteur=(float)tab->tabHau(synGlobal.getHauteur()-HAUTEUR_REF)/1000;
	mulDebit=(float)tab->tabVit(synGlobal.getDebit()-DEBIT_REF)/1000;
	mulVolume=(float)tab->tabVol(synGlobal.getVolume()-VOLUME_REF)/1000;
	texPhon=chainePhon;
	if (!strcmp(texPhon, "IXIXIZTEESI_RI"))
		strcpy(texPhon, "XDNIZXB[IZ\\IZITS[IZ\\[HR[BVCTMKG]J]SI[VDS_");
	sonDestruction();	//redondance par sécurité
	if (synGlobal.getSortieSon()) {	//sortie sur la carte-son
		entreSectionCritiqueGlobal2();	// E E E E E E E E E E E E E
#ifdef WIN32
		synSon = new classSon(FREQ_ECH, MONO, TYPE_ECH);
#else
		synSon = new classSon(FREQ_ECH, STEREO, TYPE_ECH);
#endif
		quitteSectionCritiqueGlobal2();	// Q Q Q Q Q Q Q Q Q Q Q Q
		if (!synSon->ouvertOK())
			return;
		synSon->finirSon(false);	//=son en cours
	}
	//Init signal de parole
	iLecPhon=0;	//1er caractère du texte phonétique
	iLecPhonCroit=iLecPhon;
	phonDCroit=VR;	//le texte commence par une virgule (va passer gauche)
	catDCroit=tab->categ(phonDCroit);
	nSousDiphCroit=2;	//avant le 1er sous-diphone (prochain=0)
	nSegIni.DSCroit=tab->finTim(phonDCroit);	//segment de VR
	nSeg.DSCroit=nSegIni.DSCroit;
	marq.DSCroit=true;	//début de sous-diphone activé
	marq.FSCroit=false;	//pas de fin de sous-diphone
	//Le prochain sous-diphone sera en fait le 1er
	if (!nouveauSousDiph(iLecPhonCroit, phonGCroit, phonDCroit, catGCroit, catDCroit, nSousDiphCroit,
		nSegIni.DSCroit, nSeg.DSCroit, ptAmp.DSCroit, amp.DSCroit))	//fournit ts les DS
		return;
	ptAmp.DSCroit++;	//prépare la prochaine période
	//Init Décroit sur Croit
	initDecroitSurCroit();
	amp.DSDecroit=amp.DSCroit;
	amp.FSDecroit=amp.FSCroit=0;	//pas de fin de sous-diphone
	ptSeg.DSCroit=tVoix[0]->getPtSeg(nSeg.DSCroit);	//segment de VR
	perio.DSDecroit=perio.DSCroit=(unsigned char)*ptSeg.DSCroit++;
	ptSeg.DSDecroit=ptSeg.DSCroit;
	perioResult=(unsigned char)(perio.DSCroit/mulHauteur);
	xEcrEchelleDeLec=perioResult/2*mulDebit;	//simule l'écriture (envoi des éch) à l'échelle de la lecture
	allonge= -xEcrEchelleDeLec;	//pour l'allongement des voyelles

#ifndef WIN32
	//Gestion de l'index sous linux (juste avant le début d'écriture pour une meilleure précision)
	struct timeval tv;
	gettimeofday(&tv, NULL);
	synGlobal.setktime0s(tv.tv_sec);	//temps initial qui servira de repère
	synGlobal.setktime0us(tv.tv_usec);	//µs de la s pour la précision
	short i;
	for (i=0; i<synGlobal.getNbIndexMax()+1; i++)	//il y a 1 place de plus que d'inde
		synGlobal.setTNEch(i, 0);	//tout le tableau à 0
	synGlobal.setCtEch(0);	//ainsi que le compteur d'échantillons
#endif
	
	//Traite période par période : corps du programme
	while (traiteUnePeriode());

	if (synGlobal.getSortieSon()) synSon->sonExit();
}

//Fabrique une période du signal vocal
bool Parle::traiteUnePeriode() {
	short ech;			//échantillon final à envoyer à la carte son
	short echDecroit;	//échantillon de la période Decroit
	short echCroit;	//échantillon de la période Croit
	short echT;
	long echE, echS;	//échantillon entrée et sortie du filtre
	char ech8;

	//Cherche la période Croit suivante (traitement du débit)
	while (true) {
		// Si on trouve la bonne valeur on s'arrête
		if (xEcrEchelleDeLec<0) break;
		// Sinon on passe à la suivante (xEcrEchelleDeLec diminue de chaque période lue)
		if (!perioSuiv(marq.DSCroit, marq.FSCroit, iLecPhonCroit, phonGCroit, phonDCroit, catGCroit, catDCroit,
			nSousDiphCroit, nSegIni.DSCroit, nSegIni.FSCroit, nSeg.DSCroit, nSeg.FSCroit, ptSeg.DSCroit, ptSeg.FSCroit,
			perio.DSCroit, perio.FSCroit, ptAmp.DSCroit, ptAmp.FSCroit,
			amp.DSCroit, amp.FSCroit, true))
			break;	//fin du texte : on garde la dernière période valide
	}
	//Les 4 segments (.DSDecroit, .DSCroit, .FSDecroit, .FSCroit) sont prêts
	//Calcule chaque échantillon de la nouvelle période
	for (iEch=0; iEch<perioResult; iEch++) {
//		for (short i=0;i<50;i++) {	//simule une machine plus lente (10 rien 20 un peu, 30 beaucoup, 50 un peu bouclage)
		echDecroit=calculeEchPerioDecroit(iEch, ptSeg.DSDecroit, perio.DSDecroit, amp.DSDecroit)
			+calculeEchPerioDecroit(iEch, ptSeg.FSDecroit, perio.FSDecroit, amp.FSDecroit);
		echCroit=calculeEchPerioCroit(iEch, ptSeg.DSCroit, perio.DSCroit, amp.DSCroit)
			+calculeEchPerioCroit(iEch, ptSeg.FSCroit, perio.FSCroit, amp.FSCroit);
//		}
		echT=echDecroit+echCroit;
		echE=(long)(echT*mulVolume*MUL_AMP+32768.5);	//positif, arrondi sans distorsion (echE au lieu de echS)
		//Attention, le zéro est à 32768
		echS=echS_Prec+echE-echE_Prec-(echS_Prec-32768)*2*PI*FREQ_COUPURE/FREQ_ECH;	//formule du filtre des basses
		echS_Prec=echS;	//pour le prochain échantillon
		echE_Prec=echE;	//idem
		if (TYPE_ECH==8) {	//8 bits (pour une raison inconnue, le son est de très mauvaise qualité en 8 bits)
			echS=echS/256-128;
			if (echS>127) ech8=127;
			else if (echS<-128) ech8=-128;
			else ech8=(char)echS ^ 128;	//inversion du bit de fort poids
		} else {	//16 bits
			echS=echS-32768;
			if (echS>32766) ech=32766;
			else if (echS<-32766) ech=-32766;
			else ech=(short)echS;
		}
		if (sortieWave) {
			if (TYPE_ECH==8)
				fwrite(&ech8, 1, 1, ficWave);
			else
				fwrite(&ech, 2, 1, ficWave);
			longWave++;
		}
		if (sortieSon) {
			//Transfère l'échantillon vers le fichier ou le buffer de sortie de synSon
			if (TYPE_ECH==8) {
				if (!synSon->transfert((void*)&ech8)) return false;	//faux si stop
			}
			else {
				if (!synSon->transfert((void*)&ech)) return false;	//faux si stop
			}
		}
	}
	//Mises à jour pour préparer la période suivante
	ampAnc=amp;	//on repartira de l'amplitude actuelle (4 .valeurs)
	//Avance le pointeur simulant l'écriture à l'échelle de la lecture
	if (allonge>0)
		allonge-=perioResult*mulDebit;	//cas du ':'
	else
		xEcrEchelleDeLec+=perioResult*mulDebit;	//période écrite
	//Prépare pério décroit
	if (iLecPhonCroit==iLecPhon) {	//Croit = courant : on s'y réfère avant de calculer Decroit (cas général)
		initDecroitSurCroit();	//Initialise Décroit sur Croit
		//Décroit est maintenant la période suivante
		if (!perioSuiv(marq.DSDecroit, marq.FSDecroit, iLecPhonDecroit, phonGDecroit, phonDDecroit,
			catGDecroit, catDDecroit, nSousDiphDecroit, nSegIni.DSDecroit, nSegIni.FSDecroit,
			nSeg.DSDecroit, nSeg.FSDecroit, ptSeg.DSDecroit, ptSeg.FSDecroit, perio.DSDecroit, perio.FSDecroit,
			ptAmp.DSDecroit, ptAmp.FSDecroit, amp.DSDecroit, amp.FSDecroit, false))
			return false;	//fin du message phonétique
	}	//sinon le décroit est à jour
	return true;
}

//Initialise tous les Décroit sur les Croit
void Parle::initDecroitSurCroit() {
	iLecPhonDecroit=iLecPhonCroit;
	phonDDecroit=phonDCroit;
	phonGDecroit=phonGCroit;
	catDDecroit=catDCroit;
	catGDecroit=catGCroit;
	nSousDiphDecroit=nSousDiphCroit;
	marq.DSDecroit=marq.DSCroit;
	marq.FSDecroit=marq.FSCroit;
	ptAmp.DSDecroit=ptAmp.DSCroit;
	ptAmp.FSDecroit=ptAmp.FSCroit;
	nSegIni.DSDecroit=nSegIni.DSCroit;
	nSegIni.FSDecroit=nSegIni.FSCroit;
	nSeg.DSDecroit=nSeg.DSCroit;
	nSeg.FSDecroit=nSeg.FSCroit;
}

//Prépare les variables de la période suivante
bool Parle::perioSuiv(bool& marqDS, bool& marqFS, long& iLecPhonX, char& phonG, char& phonD, char& catG, char& catD,
		char& nSousDiph, unsigned char& nSegIniDS, unsigned char& nSegIniFS,
		unsigned char& nSegDS, unsigned char& nSegFS, char*& ptSegDS, char*& ptSegFS,
		unsigned char& perioDS, unsigned char& perioFS,
		unsigned char*& ptAmpDS, unsigned char*& ptAmpFS,
		unsigned char& ampDS, unsigned char& ampFS, bool mCroit) {
	if (!marqDS && !marqFS)
		return false;
	ampDS=ampFS=0;	//par défaut si pas marq
	if (marqDS) {
		while (marqDS) {
			ampDS=*ptAmpDS;
			if ((ampDS & 128)==128) {	//début du sous-phonème suivant
				ampDS-=128;
				//DS devient FS
				marqFS=marqDS;
				ptAmpFS=ptAmpDS;
				ampFS=ampDS;
				nSegIniFS=nSegIniDS;
				nSegFS=nSegDS;
				ptSegFS=ptSegDS;
				perioFS=perioDS;
				//Nouveau DS
				ampDS=0;	//par defaut si pas marq
				marqDS=nouveauSousDiph(iLecPhonX, phonG, phonD, catG, catD, nSousDiph,
							nSegIniDS, nSegDS, ptAmpDS, ampDS);	//fournit ts les DS
			} else break;
		}
		if ((ampDS & 32)==32) {	//phonème avec bruit (fshvzjr), segment suivant
			ampDS-=32;
			nSegDS++;
		} else
			nSegDS=nSegIniDS;
		ptSegDS=tVoix[0]->getPtSeg(nSegDS);
		perioDS=(unsigned char)*ptSegDS++;
	}
	if (marqFS) {
		ampFS=*ptAmpFS & 127;
		if ((ampFS & 32)==32) {	//phonème avec bruit (fshvzjr), segment suivant
			ampFS-=32;
			nSegFS++;
		} else
			nSegFS=nSegIniFS;
		if ((ampFS & 64)==64) {	//dernière période du sous-phonème
			ampFS-=64;
			marqFS=false;	//pour le prochain tour
		}
		ptSegFS=tVoix[0]->getPtSeg(nSegFS);
		perioFS=(unsigned char)*ptSegFS++;
	}
	if (marqDS) ptAmpDS++;
	if (marqFS) ptAmpFS++;
	perioBase=((float)perio.DSDecroit*amp.DSDecroit+(float)perio.FSDecroit*amp.FSDecroit
		+(float)perio.DSCroit*amp.DSCroit+(float)perio.FSCroit*amp.FSCroit)
		/(amp.DSDecroit+amp.FSDecroit+amp.DSCroit+amp.FSCroit);
	perioResult=(unsigned char)(perioBase/mulHauteur+.5);
	if (mCroit)
		xEcrEchelleDeLec-=perioResult*mulHauteur;	//période lue mais pas forcément écrite
	return true;
}

//Prépare les variables du sous-diphone décroit suivant
bool Parle::nouveauSousDiph(long& iLecPhonX, char& phonG, char& phonD, char& catG, char& catD, char& nSousDiph,
		unsigned char& nSegIniDS, unsigned char& nSegDS, unsigned char*& ptAmpDS,
		unsigned char& ampDS) {
	short nAmp;

	if (nSousDiph==0) {
		nSousDiph++;
		ptAmpDS=tab->getPtAmp(tab->traAmp(catG, catD));
		ampDS=*ptAmpDS;
		nSegIniDS=tab->traTim(phonG, phonD);
	} else if (nSousDiph==1) {
		nSousDiph++;
		ptAmpDS=tab->getPtAmp(tab->debAmp(catG, catD));
		ampDS=*ptAmpDS;
		nSegIniDS=tab->debTim(phonD);
	} else {
		nSousDiph=0;
		phonG=phonD; catG=catD;
		phonD=phonSuiv(iLecPhonX, phonG);
		if (phonD==-1) return false;
		catD=tab->categ(phonD);
		nAmp=tab->finAmp(catG, catD);
		if (nAmp>100) {
			nAmp-=100;
			nSousDiph++;
		}
		ptAmpDS=tab->getPtAmp(nAmp);
		ampDS=*ptAmpDS;
		if (phonG==YY && phonD>YY && phonD<VR)
			nSegIniDS=tab->traTim(II,EU);
		else
			nSegIniDS=tab->finTim(phonG);
	}
	nSegDS=nSegIniDS;
	return true;
}

//Retourne le n° du phonème suivant du texte phonétique
char Parle::phonSuiv(long& iLecPhonX, char phonG) {
	
	if (iLecPhonX<iLecPhon) {	//phonème déjà trouvé pour Croit ou Décroit
		iLecPhonX=iLecPhon;
		return phonMem;	//gardé en mémoire
	}	//phonème suivant
	while (true) {
		phonMem=texPhon[iLecPhon];
		if (phonMem==0) {
			phonMem=-1;
			break;	//fin du texte phonétique
		}
		phonMem-=65;
		iLecPhon++;
		if (phonMem<YY && phonMem==phonG)
			phonMem=PL;
		if (phonMem==PL)
			allonge+=250;	//+25 ms à débit normal (~25 %)
		else if (phonMem+65==INDEX) {
			if (synGlobal.getSortieSon()) {
				synGlobal.setTNEch(synGlobal.getNbIndexEcr(), synGlobal.getCtEch());	//pour index sous linux
				synGlobal.setNbIndexEcr(synGlobal.getNbIndexEcr()-1);	//décrémente le nb d'index restant à lire (à écrire dans tampon)
			}
		}
		else break;	//rien de particulier donc phonème
	}
	iLecPhonX=iLecPhon;
	if (phonMem==-1 && phonG!=VR) phonMem=VR;
	return phonMem;
}

//Calcule un échantillon de la période décroissante.
//x abscisse de l'échantillon dans la période
short Parle::calculeEchPerioDecroit(short x, char* ptSegDecroit, unsigned char perioDecroit,
		unsigned char ampDecroit) {

	if (ampDecroit==0) return 0;
	short xCadre=x+perioDecroit/4;  //cadrage de la fenêtre sur la partie de forte amplitude
	if (xCadre>=perioDecroit) xCadre-=perioDecroit;
	if (perioResult<=perioDecroit) {	// pério résultante <= pério decroit
		return (short)(fenDecroit((float)x/perioResult)*ptSegDecroit[xCadre]*ampDecroit);
	}
	else {	// pério résultante > pério decroit
		if (x<perioDecroit)
			return (short)(fenDecroit((float)x/perioDecroit)*ptSegDecroit[xCadre]*ampDecroit);
		else
			return 0;	//reste nul derrière la fenêtre décroit
	}
}

//Calcule un échantillon de la période croissante.
//x abscisse de l'échantillon dans la période
short Parle::calculeEchPerioCroit(short x, char* ptSegCroit, unsigned char perioCroit,
		unsigned char ampCroit) {

	if (ampCroit==0) return 0;
	short xCadre=x+perioCroit-perioResult+perioCroit/4;  //cadrage de la fenêtre sur la partie de forte amplitude
	if (xCadre>=perioCroit) xCadre-=perioCroit;
	if (perioResult<=perioCroit) {	// pério résultante <= pério croit
		return (short)(fenCroit((float)x/perioResult)*ptSegCroit[xCadre]*ampCroit);
	}
	else {	//pério résultante > pério croit
		if (x>=perioResult-perioCroit)
			return (short)(fenCroit((float)(x+perioCroit-perioResult)/perioCroit)*ptSegCroit[xCadre]*ampCroit);
		else
			return 0;	//nul avant la fenêtre croit
	}
}

//////////////////////////////
// Fonctions globales
//////////////////////////////

//Initialise (init) ou termine (!init) la production d'un fichier wave
void initWave(bool init) {
	if (!synGlobal.getSortieWave()) return;
	if (init) {	//pas d'autre test car les éch suivent
		longWave=0;
		short entier;
		long entierLong;
		// Ouvre un fichier wave sinon envoie vers stdout
		if (synGlobal.getNomFichierWave() != NULL)
			ficWave=fopen((char*)synGlobal.getNomFichierWave(), "wb");
		else {
			ficWave=stdout;
		}
		fwrite("RIFF", 1, 4, ficWave);
		if (synGlobal.getNomFichierWave() != NULL)
			entierLong=longWave+36;	//taille totale du fichier restant
		else
			entierLong=4+(8+16)+(8+MS_UNSPEC);
		fwrite(&entierLong, 4, 1, ficWave);
		fwrite("WAVEfmt ", 1, 8, ficWave);
		entierLong=16;	//taille des paramètres jusqu'à "data"
		fwrite(&entierLong, 4, 1, ficWave);
		entier=1;	//identifie si PCM, ULAW etc
		fwrite(&entier, 2, 1,ficWave);
		entier=1;	// Nombre de canaux
		fwrite(&entier, 2, 1,ficWave);
		entierLong=FREQ_ECH;	//fréq éch
		fwrite(&entierLong, 4,1,ficWave);
		entierLong=FREQ_ECH*TYPE_ECH/8;	//fréq octets
		fwrite(&entierLong, 4,1,ficWave);
		entier=TYPE_ECH/8;	//nb octets par éch
		fwrite(&entier,2, 1,ficWave);
		entier=TYPE_ECH;	//8 ou 16 bits
		fwrite(&entier, 2, 1,ficWave);
		fwrite("data", 1, 4, ficWave);
		if (synGlobal.getNomFichierWave() != NULL)
			entierLong=longWave;
		else
			entierLong=MS_UNSPEC;
		fwrite(&entierLong, 4, 1, ficWave);
		ficWaveFerme=false;
	} else if (!ficWaveFerme) {	//termine
		if (synGlobal.getNomFichierWave() != NULL) {
			longWave *=TYPE_ECH/8;	// nb octets
			fseek(ficWave, 40, 0);
			fwrite(&longWave, 4, 1, ficWave);
			longWave+=36;
			fseek(ficWave, 4, 0);
			fwrite(&longWave, 4, 1, ficWave);
			fclose(ficWave);
		} else
			fflush(ficWave);
		ficWaveFerme=true;
	}
}

//Détruit synSon après avoir parlé
void sonDestruction() {
	if (synSon) {
#ifdef WIN32
		entreSectionCritiqueGlobal3();	// E E E E E E E E E E E E E
		delete synSon;	//détruit l'objet son (donc le buffer circulaire et l'objet carte-son snd_dev)
		quitteSectionCritiqueGlobal3();	// Q Q Q Q Q Q Q Q Q Q Q Q Q
#else
		delete synSon;	//détruit l'objet son (donc le buffer circulaire et l'objet carte-son snd_dev)
#endif
		synSon=NULL;	//pour redondance de sonDestruction
	}
}
