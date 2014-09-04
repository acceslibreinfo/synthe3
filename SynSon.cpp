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

////////////////////////////////////
// Sortie vers la carte-son
////////////////////////////////////

#include "SynSon.h"
#include "SynGlobal.h"

classSon* synSon;

//Constructeur : initialise les paramètres des échantillons, les variables globables et les objets de gestion du son.
//Prend en paramètre la fréq éch, le nb de voies (MONO ou STEREO) et le type des échantillons (16 ou 8 bits).
classSon::classSon(int frequence, short typeEchVoie) {
	snd_ok = false;	//direct sound pas encore essayé d'ouvrir
	iEchEcr = 0;	//position écriture au début du buffer (en nb de paquets)
	iEchPaquet = 0;	//position dans le paquet
	iEchPosLec=0;	//position de lecture
	iEchPosLecAnc=0;	//copie pour restauration
	iEchPosLecAv=0;	//indice lecture précédent (avant positionLecture)
	marqJoueInit = false;	//n'a pas commencé à jouer
	marqJoue = false;	//ne joue pas
	pauseEnCours = false;	//pas de pause en cours
	// Paramètres du son
	fEchCarte = frequence;	// 44100, 22050 ou 11025
//	fEchCarte=22050;	//permet de forcer la fréquence de la carte et d'adapter la sortie des éch
	nbVoiesSon = 1;	// 1 MONO ou 2 STEREO
	nbBitsParEchVoie = typeEchVoie;	// 8 ou 16 bits
	nbOctetsParEch = nbBitsParEchVoie*nbVoiesSon/8;
	// Paramètres du tampon
	short nbEchUnite=(short)(UNITE_TEMPS*fEchCarte+.5);	//2
	nbEchPaquet = (long)(TEMPS_PAQUET*fEchCarte+.5)/nbEchUnite*nbEchUnite;
	nbEchSecu = (long)(TEMPS_SECU*fEchCarte+.5)/nbEchUnite*nbEchUnite;
	nbPaquetsLpBuffer=(short)((long)(TEMPS_TAMPON*fEchCarte+.5)/nbEchPaquet);
	nbEchLpBuffer = nbPaquetsLpBuffer*nbEchPaquet;
	nbEchLpBuffer2=nbEchLpBuffer/2;
	nbOctetsPaquet = nbEchPaquet*nbOctetsParEch;
	// Crée le paquet
	switch(nbBitsParEchVoie) {
	case 8 : paquet8 = new char[nbEchPaquet*nbVoiesSon]; break;
	case 16 : paquet16 = new short[nbEchPaquet*nbVoiesSon]; break;
	}
	// Crée le tableau d'index
	tIndex = new short[nbPaquetsLpBuffer];
	tIndex[0]=synGlobal.getNbIndexEcr();
	//Divers init
	echTot=0;
	nbTot=0;
	cumul=0;
	open_snd();
	set_snd_params(nbVoiesSon, typeEchVoie, frequence);
}

//Destructeur son
classSon::~classSon() {
	if (!snd_ok) return;
	snd_ok=false;
	switch(nbBitsParEchVoie) {
		case 8 : delete[] paquet8; break;
		case 16 : delete[] paquet16; break;
	}
	delete[] tIndex;
	close_snd();
}

//Son ouvert avec succès (idem pour fermé)
bool classSon::ouvertOK() {
	return snd_ok;
}

//Indique que Synthé doit finir de lire le buffer jusqu'à la limite d'écriture
void classSon::finirSon(bool marqFin) {
	if (marqFin) {
		etatFin=1;	//lit jusqu'à la limite limLit
		limLit=iEchEcr;	//arrondi à la taille du paquet
		limLitPaquet=iEchPaquet;	//position dans le paquet
	} else
		etatFin=0;	//en cours de lecture
}

//Transfère les échantillons dans le buffer secondaire.
//Prend en paramètre l'échantillon à envoyer à la carte son. 2 valeurs en stéréo
//En cas de stop, return false pour stopper tout
//bool classSon::transfert(LPVOID ptEchG, LPVOID ptEchD) {	//localisation
bool classSon::transfert(LPVOID ptEch) {
	synGlobal.incrCtEch();	//incrémente le compteur d'échantillons pour gérer le tableau d'index (position de lecture sous Linux)
	if (iEchPaquet==0)
		indexAEcrire=synGlobal.getNbIndexEcr();	//met à jour l'index à écrire à chaque nouveau paquet
	if (synGlobal.getDemandeStop()) {
		pauseSiJoue();
		return false;	//le stop agit immédiatement (à chaque échantillon)
	}
	if (etatFin>0) {	//cas de la fin normale du message
		positionLecture();	//demande position à la carte son (->iEchPosLec) (à chaque fois pour plus de finesse en fin de son)
		//Si l'UC a été sollicitée, la lecture peut avoir légèrement dépassé la limite, d'où les précautions qui suivent
		if (etatFin==1) {	//1er état de fin : avant bouclage
			if (iEchPosLec<limLit || iEchPosLec<iEchPosLecAv-nbEchLpBuffer2) {
				//Si la lecture est avant la limite ou a bouclé
				etatFin=2;	//indique que la lecture ne va plus boucler
				limLit+=limLitPaquet;	//position exacte de la fin, mais il fallait attendre que la lecture ait avancé (et reparte pour un tour)
					//car le paquet n'est pas encore écrit et la limite réelle est en avance (UC rapide) (sinon la dernière seconde est coupée)
			}
		}	//Sinon : la lecture est derrière la limite, on ne fait rien, on attend le bouclage
		else if (etatFin==2) {	//2e état de fin: après bouclage
			if (iEchPosLec>=limLit || iEchPosLec<iEchPosLecAv-nbEchLpBuffer2) {
				//Si la lecture a rattrapé ou dépassé la limite, c'est fini
				pauseSiJoue();	//stop normal car lecture terminée (sous directSound)
				return false;
			}
		}	//Sinon : la lecture n'a pas atteint la limite, on attend
		iEchPosLecAv=iEchPosLec;   	//repère la dernière position de lecture pour repérer bouclage plus tard
	}
	// Ajoute un éch dans le paquet
	switch (nbBitsParEchVoie) {
	case 8 : 
		paquet8[iEchPaquet*nbVoiesSon] = *((char*)ptEch);
		break;
	case 16 :
		paquet16[iEchPaquet*nbVoiesSon] = *((short*)ptEch);
		break;
	}
	iEchPaquet++;
	// Si un paquet est rempli, on le transfère dans lpBuffer
	if (iEchPaquet == nbEchPaquet) {
		// Contrôle la lecture et l'écriture
		attendSiEcrOuLecRattrape();	//les pointeurs de lecture et d'écriture ne doivent pas se doubler
		// Ecrit l'index dans un tableau circulaire (du nb de paquets)
		tIndex[iEchEcr/nbEchPaquet] = indexAEcrire;
		// Transfère le paquet plein dans lpBuffer
		entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
		switch (nbBitsParEchVoie) {
		case 8 :
#ifdef WIN32
			transferePaquet(lpBuffer, nbOctetsParEch*iEchEcr, paquet8, nbOctetsPaquet);
#else
			transferePaquet(paquet8, nbOctetsPaquet);
#endif
			break;
		case 16 :
#ifdef WIN32
			transferePaquet(lpBuffer, nbOctetsParEch*iEchEcr, paquet16, nbOctetsPaquet);
#else
			transferePaquet(paquet16, nbOctetsPaquet);
#endif
			break;
		}
		quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
		// Met à jour les positions
		iEchEcr += nbEchPaquet;
		if (iEchEcr == nbEchLpBuffer)
			iEchEcr = 0;	//l'écriture boucle
		iEchPaquet = 0;	//démarre un nouveau paquet
	}
	return true;
}

//Attend que la position de lecture ait pris au moins un paquet d'avance sur la position d'écriture.
//Arrête l'écriture si elle rattrape la lecture (UC rapide)
//Arrête la lecture si elle rattrape l'écriture (UC ralentie) (pause sinon disque rayé)
//Utilisé dans transfert
void classSon::attendSiEcrOuLecRattrape() {
	short nbPaquetsEcart=nbEchSecu/nbEchPaquet;	//nb paquets d'écart min quand lec rattrape ecr (nbPaquetsEcart+1 paquets à préparer avant début lec)
	short iPaquetEcr;
	short iPaquetEcrAvant;	//paquet écrit nbPaquetsEcart paquets avant
	short iPaquetLec;

#ifdef WIN32
	initWindow();	//si le handle de la fenêtre courante à changé, l'objet DirectSound est remis à jour
#endif
	//Si la lecture est en cours, attend qu'il y ait la place d'écrire un paquet plein sans déborder pt lecture
	while (true) {
		//Récupère la position de lecture
		positionLecture();	//fournit iEchPocLec (à chaque paquet terminé)
		//Met à jour l'index (au moins une fois par paquet) pour indiquer à Synthé où il en est
#ifdef WIN32
		synGlobal.setNbIndexLec(tIndex[iEchPosLec/nbEchPaquet]);
#else
		//Ou le calcule en fonction du temps pour Linux
		struct timeval tv;
		float n=0;
		gettimeofday(&tv, NULL);
		float a=(tv.tv_sec-synGlobal.getktime0s())*1000000;
		float b=tv.tv_usec-synGlobal.getktime0us();
		n=a+b;	//en µs
		n/=1000000.0;	//en s
		n*=22050.0;	//en échantillons
		short i=0;
		int nbIndexMax=synGlobal.getNbIndexMax();
		for (i=0; i<=nbIndexMax; i++) {
			if (synGlobal.getTNEch(i)>0 && synGlobal.getTNEch(i)<n) {
				break;
			}
		}
		synGlobal.setNbIndexLec(i);
#endif
		//Tant que le paquet à transférer recouvre le paquet en cours de lecture, on attend
		iPaquetLec=(short)(iEchPosLec/nbEchPaquet);
		iPaquetEcr=(short)(iEchEcr/nbEchPaquet);
		if (iPaquetLec!=iPaquetEcr || !marqJoue)	//si les numéros de paquet ne sont pas égaux (ou si on ne joue pas)
			break;	//on peut écrire
		Sleep(0);	//en cas d'attente, on donne la main aux autres threads
	}
	iPaquetEcrAvant=iPaquetEcr-nbPaquetsEcart;	//repère
	if (iPaquetEcrAvant<0)
		iPaquetEcrAvant+=nbPaquetsLpBuffer;	//bouclage
		if ((iPaquetEcrAvant<iPaquetEcr &&	//si les 2 repères dans la même boucle
		iPaquetLec>=iPaquetEcrAvant &&	//et lecture entre les 2 repères
		iPaquetLec<iPaquetEcr) ||	//donc proche de l'écriture -> pause
		(iPaquetEcrAvant>iPaquetEcr &&	//si bouclage entre les 2 repères
		(iPaquetLec>=iPaquetEcrAvant ||	//et lecture derrière le 1er repère
		iPaquetLec<iPaquetEcr)))	//ou devant la lecture (donc proche) -> pause
		pauseSiJoue();	//si la lecture rattrape l'écriture, on la stoppe un moment
	else if (iPaquetLec!=iPaquetEcr)
		joueSiPause();	//sinon, si l'écart est suffisant, on démarre ou on reprend
}

