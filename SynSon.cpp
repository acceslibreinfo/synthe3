/*
 * SynthÃ© 3 - A speech synthetizer software for french
 *
 * Copyright (C) 1985-2014 by Michel MOREL <michel.morel@unicaen.fr>.
 *
 * SynthÃ© 3 comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU Lesser General Public License, as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version. Please see the file LICENSE-LGPL for details.
 *
 * Web Page: https://github.com/acceslibreinfo/synthe3
 *
 * This software is maintained by ShÃ©rab <Sebastien.Hinderer@ens-lyon.org>.
 */

////////////////////////////////////
// Sortie vers la carte-son
// (commune à Alsa et Direct-Sound)
////////////////////////////////////

#include "synSon.h"
#include "synGlobal.h"
#include "synMain.h"
#ifndef WIN32
	#include <unistd.h>
#endif

classSon* synSon;

//Constructeur : initialise les paramètres des échantillons, les variables globables et les objets de gestion du son.
//Prend en paramètre la fréq éch, le nb de voies (MONO ou STEREO) et le type des échantillons (16 ou 8 bits).
classSon::classSon(int frequence, short nbVoies, short typeEchVoie) {
	snd_ok = false;	//direct sound pas encore essayé d'ouvrir
	iEchEcr = 0;	//position écriture au début du buffer (en nb de paquets)
	iEchPaquet = 0;	//position dans le paquet
	iEchPosLec=0;	//position de lecture
	iEchPosLecSauv=0;	//copie pour restauration
	iEchPosLecAv=0;	//indice lecture précédent (avant positionLecture)
	marqJoueInit = false;	//n'a pas commencé à jouer
	marqJoue = false;	//ne joue pas
	pauseEnCours = false;	//pas de pause en cours
	// Paramètres du son
	fEchCarte = frequence;	// 44100, 22050 ou 11025
//	fEchCarte=22050;	//permet de forcer la fréquence de la carte et d'adapter la sortie des éch
	nbVoiesSon = nbVoies;	// MONO ou STEREO
	nbBitsParEchVoie = typeEchVoie;	// 8 ou 16 bits
	nbOctetsParEch = nbBitsParEchVoie*nbVoiesSon/8;
	// Paramètres du tampon
	short nbEchUnite=(short)(UNITE_TEMPS*fEchCarte+.5);	//2 (ne sert que pour les arrondis)
	nbEchPaquet = (long)(TEMPS_PAQUET*fEchCarte+.5)/nbEchUnite*nbEchUnite;
#ifndef WIN32
	nbEchPaquet=2048;
#endif
	nbEchSecuEcr = (long)(TEMPS_SECU_ECR*fEchCarte+.5)/nbEchUnite*nbEchUnite;
	nbEchSecuLec = (long)(TEMPS_SECU_LEC*fEchCarte+.5)/nbEchUnite*nbEchUnite;
	nbPaquetsLpBuffer=(short)((long)(TEMPS_TAMPON*fEchCarte+.5)/nbEchPaquet);
#ifndef WIN32
	nbPaquetsLpBuffer=8;
#endif
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
	set_snd_params(nbVoies,typeEchVoie,frequence);
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
		etatSon=1;	//lit jusqu'à la limite limLit
		limLit=iEchEcr;	//arrondi à la taille du paquet
		limLitPaquet=iEchPaquet;	//position dans le paquet
	} else
		etatSon=0;	//prêt pour lecture (ce n'est pas la fin)
}

//Transfère les échantillons dans le buffer secondaire.
//Prend en paramètre l'échantillon à envoyer à la carte son. 2 valeurs en stéréo
//En cas de stop, return false pour stopper tout
//bool classSon::transfert(LPVOID ptEchG, LPVOID ptEchD) {	//localisation
bool classSon::transfert(LPVOID ptEch) {
	synGlobal.incrCtEch();	//incrémente le compteur d'échantillons pour gérer le tableau d'index (position de lecture sous Linux)
	if (iEchPaquet==0) {
		indexAEcrire=synGlobal.getNbIndexEcr();	//met à jour l'index à écrire à chaque nouveau paquet
		//Sleep(30);	//ralentit l'écriture pour test gestion lecture
	}
	if (synGlobal.getDemandeStop()) {
		pauseSiJoue();
		return false;	//le stop agit immédiatement (à chaque échantillon)
	}
	if (etatSon>0 && iEchPaquet==0) {	//cas de la fin normale du message, nouveau paquet
		positionLecture();	//demande position à la carte son (->iEchPosLec) (une fois par paquet sinon ralentit)
		/*if (!marqJoueInit)
			iEchPosLec=0;	//redondant au cas où positionLecture retournerait une valeur fausse
		else if (!marqJoue)
			iEchPosLec=iEchPosLecAv;	//idem*/
		//Si l'UC a été sollicitée, la lecture peut avoir légèrement dépassé la limite, d'où les précautions qui suivent
		if (etatSon==1) {	//1er état de fin : avant bouclage
			if (iEchPosLec<limLit || iEchPosLec<iEchPosLecAv-nbEchLpBuffer2) {
				//Si la lecture est avant la limite ou a bouclé
				etatSon=2;	//indique que la lecture ne va plus boucler
				limLit+=limLitPaquet;	//position exacte de la fin, mais il fallait attendre que la lecture ait avancé (et reparte pour un tour)
					//car le paquet n'est pas encore écrit et la limite réelle est en avance (UC rapide) (sinon la dernière seconde est coupée)
			}	//Sinon : la lecture est derrière la limite, on ne fait rien, on attend le bouclage
		}
		else if (etatSon==2) {	//2e état de fin: après bouclage
			if (iEchPosLec>=limLit || iEchPosLec<iEchPosLecAv-nbEchLpBuffer2) {
				//Si la lecture a rattrapé ou dépassé la limite ou bouclé, c'est fini
				pauseSiJoue();	//stop normal car lecture terminée (sous directSound)
				return false;
			}	//Sinon : la lecture n'a pas atteint la limite, on attend
		}
		iEchPosLecAv=iEchPosLec;   	//repère la dernière position de lecture pour détecter bouclage plus tard
	}
	// Ajoute un éch dans le paquet
	switch (nbBitsParEchVoie) {
	case 8 : 
		paquet8[iEchPaquet*nbVoiesSon] = *((char*) ptEch);
		if (nbVoiesSon==STEREO)
			paquet8[iEchPaquet*nbVoiesSon+1] = *((char*) ptEch);
		break;
	case 16 :
		paquet16[iEchPaquet*nbVoiesSon] = *((short*)ptEch);
		if (nbVoiesSon==STEREO)
			paquet16[iEchPaquet*nbVoiesSon+1] = *((short*) ptEch);
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
		if (iEchEcr >= nbEchLpBuffer)
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
	short nbPaquetsSecuEcr=nbEchSecuEcr/nbEchPaquet;	//nb paquets d'écart min quand ecr rattrape lec
	short nbPaquetsSecuLec=nbEchSecuLec/nbEchPaquet;	//nb paquets d'écart min quand lec rattrape ecr
	short iPaquetEcr;
	short iPaquetLecAvant;	//repère de sécurité avant le paquet en cours de lecture
	short iPaquetLecApres;	//repère de sécurité après le paquet en cours de lecture
	short iPaquetLec;

#ifdef WIN32
	initWindow();	//si le handle de la fenêtre courante à changé, l'objet DirectSound est remis à jour
#endif
	//Si la lecture est en cours, attend qu'il y ait la place d'écrire un paquet plein sans déborder pt lecture
	while (true) {
		//Récupère la position de lecture
		positionLecture();	//fournit iEchPosLec (à chaque paquet terminé)
		/*if (!marqJoueInit)
			iEchPosLec=0;	//redondant pour Jaws au cas où positionLecture retournerait une valeur fausse
		else if (!marqJoue)
			iEchPosLec=iEchPosLecAv;	//essayer : ne donne rien*/
		iEchPosLecAv=iEchPosLec;   	//repère la dernière position de lecture pour détecter bouclage plus tard
		iPaquetLec=(short)(iEchPosLec/nbEchPaquet);
		iPaquetEcr=(short)(iEchEcr/nbEchPaquet);
		iPaquetLecAvant=iPaquetLec-nbPaquetsSecuEcr;	//repère à ne pas dépasser en écriture
		if (iPaquetLecAvant<0)
			iPaquetLecAvant+=nbPaquetsLpBuffer;	//bouclage
		iPaquetLecApres=iPaquetLec+nbPaquetsSecuLec;	//repère
		if (iPaquetLecApres>=nbPaquetsLpBuffer)
			iPaquetLecApres-=nbPaquetsLpBuffer;	//bouclage
		if (!marqJoue)
			break;	//si on ne joue pas, on peut écrire
		//Si on joue, met à jour l'index (au moins une fois par paquet) pour indiquer à Synthé où il en est
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
		//Quand on joue, tant que le paquet à transférer est dans le secteur qui précède le paquet en cours de lecture, on attend pour écrire
		if ((iPaquetLecAvant<iPaquetLec &&	//si les 2 repères dans la même boucle
			(iPaquetEcr<iPaquetLecAvant ||	//et écriture avant le premier repère
			iPaquetEcr>iPaquetLec)) ||	//ou après la lecture -> on peut écrire
			(iPaquetLecAvant>iPaquetLec &&	//si bouclage entre les 2 repères
			iPaquetEcr<iPaquetLecAvant &&	//et écriture avant le 1er repère
			iPaquetEcr>iPaquetLec))	//et derrière la lecture -> on peut écrire
			break;	//on peut écrire
		Sleep(0);	//en cas d'attente, on donne la main aux autres threads
	}
	if (!marqJoueInit) {	//pas commencé
		if (iPaquetEcr>iPaquetLecApres) {
			joueSiPause();	//si l'écart est suffisant, on démarre
			marqJoueInit=true;	//c'est commencé
		}
	//Si le paquet à transférer est dans le secteur qui suit le paquet en cours de lecture, on arrête la lecture le temps qu'il s'éloigne
	} else if ((iPaquetLecApres>iPaquetLec &&	//si les 2 repères dans la même boucle
		iPaquetEcr<=iPaquetLecApres &&	//et écriture entre les 2 repères
		iPaquetEcr>iPaquetLec) ||	//donc pas assez loin de la lecture -> pause lecture
		(iPaquetLecApres<iPaquetLec &&	//si bouclage entre les 2 repères
		(iPaquetEcr<=iPaquetLecApres ||	//et écriture devant le 1er repère
		iPaquetEcr>iPaquetLec)))	//ou derrière la lecture (donc pas assez loin) -> pause lecture
		pauseSiJoue();	//si la lecture rattrape l'écriture, on la stoppe un moment.
	else
		joueSiPause();	//sinon, l'écart est suffisant, on reprend
}

