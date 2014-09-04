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

#ifndef __SYN_SON_H_
#define __SYN_SON_H_

////////////////////////////////////
// Sortie vers la carte-son
////////////////////////////////////

#ifdef WIN32
	#define CARTESON LPDIRECTSOUND
	#include <dsound.h>
#else
	#define LPVOID void*
	#define DWORD int
	#define Sleep sleep
	#include <sys/time.h>
	#ifdef ALSA
		#define CARTESON snd_pcm_t*
		#define ALSA_PCM_NEW_HW_PARAMS_API
		#include <alsa/asoundlib.h>
	#else // OSS
		#define CARTESON int
		#include <fcntl.h>
		#include <sys/ioctl.h>
		#include <linux/soundcard.h>
	#endif
#endif

#include <cstdio>

const double UNITE_TEMPS=1./11025;	//unité de temps représentant 1 ou 2 échantillons
const double TEMPS_PAQUET=.02;	//20 ms par paquet
//const double TEMPS_PAQUET=.1;	//100 ms par paquet
const double TEMPS_SECU=.2;	//200 ms pour incertitude position
const double TEMPS_TAMPON=1;	//1000 ms pour le tampon secondaire

class classSon {

protected :
#ifdef WIN32 // partie dédiée DirectSound
	LPDIRECTSOUNDBUFFER lpBuffer; // buffer circulaire DirectSound (secondaire)
	LPDWORD ptIOctLecDS; // pointeur sur position de lecture du buffer direct sound (en octets)
	LPDWORD ptIOctEcrDS; // pointeur sur position d'écriture du buffer direct sound (en octets, non exploité)
	HWND Handle; // Handle de la fenêtre courante
	long DSVolume;	// volume du buffer DirectSound
#endif
	CARTESON snd_dev;       // id de la carte son (DirectSound, ALSA et OSS)
	long nbEchLpBuffer;	//nb éch dans buffer secondaire
	long nbEchLpBuffer2;	//idem / 2
	char* paquet8;	// paquet de mots 8 bits
	short* paquet16;	// paquet de mots 16 bits
	short* tIndex;		// table des index
	short nbVoiesSon;	// nb voies : 1 mono, 2 stéréo
	long nbEchPaquet;	//nb éch par paquet
	long nbEchSecu;	//nb éch pour incertitude position lecture
	long nbOctetsPaquet;	//taille en octets du paquet
	short nbPaquetsLpBuffer;
	short nbBitsParEchVoie;	// 8 ou 16 bits
	short nbOctetsParEch;	//1 à 4
	int fEchCarte;          // fréquence d'échantillonnage de la carte son
	short indexAEcrire;     // index à écrire dans le tableau tIndex (en début de paquet)
	long iEchEcr;	// position d'écriture dans le buffer circulaire "lpBuffer"
	long iEchPaquet;	// position dans le paquet
	long iEchPosLec;	//position de lecture dans le buffer circulaire "lpBuffer"
	long iEchPosLecAnc;	//id val précédente
	long iEchPosLecAv;	//variante pour finir son
	bool marqJoue;	// sortie du son en cours
	bool marqJoueInit;	//sortie du son initialisée et commencée
	bool pauseEnCours;	// son en pause
	short etatFin;	//0=normal, 1=lire derrière la limite, 2=lire jusqu'à la limite
	long limLit;	//=iEchEcr final du message
	long limLitPaquet;	//=iEchPaquet final du message
	bool snd_ok;	// carte son ouverte avec succès

private :
	long echTot;
	short nbTot;
	long cumul;

public :
	classSon(int frequence, short typeEchVoie); 
	~classSon();
	bool ouvertOK();
	bool transfert(LPVOID);
	void sonExit();	//fin du message : termine le buffer proprement
	void finirSon(bool marqFin);
	bool pauseSiJoue();	//stoppe le son sous DirectSound
	bool joueSiPause();	//démarre le son sous DirectSound
	void attendSiEcrOuLecRattrape();
	void positionLecture();
#ifdef WIN32
	bool initWindow();
#endif
	static char *yes_no(int condition) {
		if (condition) return "yes"; else return "no";
	}
private:
#ifdef WIN32
	bool transferePaquet(LPDIRECTSOUNDBUFFER, DWORD, LPVOID, DWORD);
	HRESULT creeBufferSecondaire(LPDIRECTSOUNDBUFFER*);
	bool modifieBufferPrimaire();
#else
	bool transferePaquet(LPVOID, DWORD);
#endif
	void open_snd();
	void close_snd();
	void get_snd_params();
	void set_snd_params(int, int, int);
};

#endif
