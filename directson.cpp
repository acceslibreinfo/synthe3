/*
 * DirectX output module for Synthé 3 - use DirectX to let SynthÃ© 3 speak aloud text
 *
 * Copyright (C) 1985-2014 by the CRISCO laboratory,
 * University of Caen (France).
 *
 * This DirectX output module for Synthé 3 comes with ABSOLUTELY NO WARRANTY.
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

#include "synSon.h"
#include "synCalcul.h"
#include "synGlobal.h"

//Ouvre la carte son
void classSon::open_snd() {
	DSVolume = 0;	//volume max de direct sound
	// Pointeurs renvoyÃ©s par GetCurrentPosition
	ptIOctLecDS = new DWORD;
	ptIOctEcrDS = new DWORD;
	// CrÃ©e l'objet DirectSound
	HRESULT hr;
	hr = DirectSoundCreate(NULL, &snd_dev, NULL);	//crÃ©e un objet snd_dev pour la carte son
	if (hr != DS_OK) return;
	// L'objet DirectSound se base sur le handle de la fenÃªtre active pour jouer les Ã©chantillons.
	Handle = GetForegroundWindow();	//initialisation du Handle de la fenÃªtre courante
	hr = snd_dev->SetCooperativeLevel(Handle, DSSCL_PRIORITY);
	if (hr != DS_OK) return;
	//Modifie le format du tampon principal
	modifieBufferPrimaire();
	// CrÃ©e le buffer secondaire
	hr = creeBufferSecondaire(&lpBuffer);	//crÃ©e le buffer circulaire pour la lecture en DMA
	if (hr != DS_OK) return;
	//Positionne le pointeur de lecture en dÃ©but de buffer
	hr = lpBuffer->SetCurrentPosition(0);
	snd_ok = (hr == DS_OK);
}

//Ferme la carte son
void classSon::close_snd() {
	delete (int*)ptIOctLecDS;
	delete (int*)ptIOctEcrDS;
	HRESULT hr;
	// Destruction du buffer circulaire
	hr = lpBuffer->Release();
	if (hr != DS_OK) return;
	lpBuffer = NULL;
	// Destruction de l'objet DirectSound
	hr = snd_dev->Release();
	if (hr != DS_OK) return;
	snd_dev = NULL;
}

void classSon::set_snd_params(int channels, int bits, int rate) {}
void classSon::get_snd_params() {}

//Fin du message : termine le buffer proprement
//Indique la fin d'Ã©criture du son et transfÃ¨re des Ã©chantillons nuls en attendant l'arrÃªt de la lecture
//Evite que le dÃ©passement du dernier point d'Ã©criture ne produise un son inopportun
//Permet de dÃ©marrer la lecture si le message est trÃ¨s court (Ã©cart trop faible pour dÃ©marrer sans la suite nulle)
void classSon::sonExit() {
	finirSon(true);	//etatFin=1 (dit Ã  transfert qu'il faudra s'arrÃªter Ã  limLit), dÃ©finit la limite limLit
	short ech=0;
	while (transfert((LPVOID)&ech));	//la lecture s'arrÃªtera Ã  limLit
}

//Si le handle de la fenÃªtre courante Ã  changÃ©, l'objet DirectSound est remis Ã  jour
bool classSon::initWindow() {
	HWND hwnd = GetForegroundWindow();
	if (hwnd!=Handle) {
		Handle = hwnd;
		HRESULT hr = snd_dev->SetCooperativeLevel(Handle, DSSCL_PRIORITY);
		if (hr!=DS_OK) return false;
	}
	return true;
}

//ArrÃªte le son rÃ©versiblement
bool classSon::pauseSiJoue() {
	HRESULT hr;

	if (!marqJoue)	//rien s'il ne joue pas
		return true;
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	hr = lpBuffer->Stop();
	if (hr == DS_OK) marqJoue = false;
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	if (hr == DS_OK) return true;
	return false;
}

//Lance ou relance le son
bool classSon::joueSiPause() {
	HRESULT hr;

	marqJoueInit=true;	//c'est commencÃ©
	if (marqJoue)	//rien s'il joue dÃ©jÃ 
		return true;
	//Joue le buffer secondaire en boucle
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	hr = lpBuffer->Play(0, 0, DSBPLAY_LOOPING);
	if (hr == DSERR_BUFFERLOST) {
		hr = lpBuffer->Restore();
		if (hr == DS_OK) hr = lpBuffer->Play(0, 0, DSBPLAY_LOOPING);
	}
	if (hr == DS_OK) {
		hr = lpBuffer->SetVolume(DSVolume);	// RÃ©gle le volume
		marqJoue = true;	//il joue
	}
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	if (hr == DS_OK) return true;
	return false;
}

//Position de lecture (avec retouches si nÃ©cessaire)
void classSon::positionLecture() {
	iEchPosLecAnc=iEchPosLec;	//sauvegarde la position de lecture
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	HRESULT hr = lpBuffer->GetCurrentPosition(ptIOctLecDS, ptIOctEcrDS);	//demande la nouvelle position Ã  la carte son
	if (hr == DSERR_BUFFERLOST) {
		hr = lpBuffer->Restore();
		if (hr == DS_OK) hr = lpBuffer->GetCurrentPosition(ptIOctLecDS, ptIOctEcrDS);
	}
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	if (hr == DS_OK) {
		iEchPosLec = (long)*ptIOctLecDS/nbOctetsParEch;
	}
	long variation=(iEchPosLec-iEchPosLecAnc+nbEchLpBuffer)%nbEchLpBuffer;
}

//CrÃ©e le buffer secondaire au format voulu.
//AppelÃ©e dans SonInit, renvoie vrai si la crÃ©ation s'est bien dÃ©roulÃ©e.
HRESULT classSon::creeBufferSecondaire(LPDIRECTSOUNDBUFFER *lplpDsb) {
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbdesc;

	// Format des Ã©chantillons sonores Ã  envoyer
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT)); // allocation
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;	// format wave traditionnel
	pcmwf.wf.nChannels = nbVoiesSon;	// nb voies (1=mono, 2=stÃ©rÃ©o)
	pcmwf.wf.nSamplesPerSec = fEchCarte;	// nb Ã©chantillons par seconde
	pcmwf.wBitsPerSample = nbBitsParEchVoie;	// nb bits par Ã©chantillon voie
	pcmwf.wf.nBlockAlign = pcmwf.wf.nChannels*pcmwf.wBitsPerSample/8;	//nb octets par Ã©chantillon complet
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;	//nombre moyen d'octets/s
	
	// Description du buffer directsound (circulaire)
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
//	dsbdesc.dwFlags = DSBCAPS_CTRLDEFAULT;
	//Seuls la position et le volume nous intÃ©ressent (Ã©conomie de ressources)
	//Mais DSBCAPS_GLOBALFOCUS permet d'Ã©viter les boÃ®tes de dialogue muettes
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
	// Nombre d'octets dans le buffer secondaire
	dsbdesc.dwBufferBytes = nbEchLpBuffer*nbOctetsParEch;
	// On associe le format des Ã©chantillons Ã  la description
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	// CrÃ©ation du buffer directsound Ã  partir de la description
	HRESULT hr = snd_dev->CreateSoundBuffer(&dsbdesc, lplpDsb, NULL);
	if (hr != DS_OK) *lplpDsb = NULL;
	return hr;
}

//Essaie de modifier le format du tampon principal
bool classSon::modifieBufferPrimaire() {
	HRESULT hr;
	LPDIRECTSOUNDBUFFER lpDsb;
	DSBUFFERDESC dsbdesc;
	WAVEFORMATEX wfm;

	// Description du tampon principal (crÃ©e structure DSBUFFERDESC)
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbdesc.dwBufferBytes = 0;	//doit Ãªtre 0 pour le tampon principal
	dsbdesc.lpwfxFormat = NULL;	//doit Ãªtre NULL pour le tampon principal
	//CrÃ©e structure pour le format dÃ©sirÃ©
	memset(&wfm,0,sizeof(WAVEFORMATEX));
	wfm.wFormatTag = WAVE_FORMAT_PCM;	// format wave traditionnel
	wfm.nChannels = nbVoiesSon;	// nb voies (1=mono, 2=stÃ©rÃ©o)
	wfm.nSamplesPerSec = fEchCarte;	// nb Ã©chantillons par seconde
	wfm.wBitsPerSample = nbBitsParEchVoie;	// nb bits par Ã©chantillon voie
	wfm.nBlockAlign = wfm.nChannels*wfm.wBitsPerSample/8;	//nb octets par Ã©chantillon complet
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;	//nombre moyen d'octets/s
	//AccÃ¨s au tampon principal
	hr = snd_dev->CreateSoundBuffer(&dsbdesc,&lpDsb,NULL);
	if (hr != DS_OK) return false;
	//Modifie le format du tampon principal. Si erreur, garde le format par dÃ©faut
	hr = lpDsb->SetFormat(&wfm);
	if (hr != DS_OK) return false;
	return true;
}

// Ecrit un paquet dans le buffer secondaire (en section critique).
// Si la copie n'a pu avoir lieu, renvoie faux
bool classSon::transferePaquet(LPDIRECTSOUNDBUFFER lpDsb, DWORD dwOffset, LPVOID lpData, DWORD dwSoundBytes) {
	LPVOID lpvPtr1;	// 1er pointeur pour l'Ã©criture
	DWORD dwBytes1;	// nb octets pointÃ©s par lpvPtr1
	LPVOID lpvPtr2;	// 2e pointeur pour l'Ã©criture 
	DWORD dwBytes2;	// nb octets pointÃ©s par lpvPtr2
	char* lpSoundData8 = (char*) lpData;
	short* lpSoundData16 = (short*) lpData;
	HRESULT hr;

	// On bloque Ã  partir de dwOffset une portion de taille dwSoundBytes.
	// lpvPtr1 pointe le premier bloc du buffer qui doit Ãªtre bloquÃ©.
	// Si dwBytes1 < dwSoundBytes, alors lpvPtr2 pointera sur un second bloc (dÃ©but du buffer).
	hr = lpDsb->Lock(dwOffset, dwSoundBytes, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);	
	if (hr == DSERR_BUFFERLOST) {
		hr = lpDsb->Restore();
		if (hr==DS_OK) hr = lpDsb->Lock(dwOffset, dwSoundBytes, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	}
	if (hr != DS_OK) return false;
	// Copie le contenu de lpSoundData de taille dwBytes1 dans lpvPtr1
	switch (nbBitsParEchVoie) {
	case 8 :
		CopyMemory(lpvPtr1, lpSoundData8, dwBytes1); break;
	case 16 :
		CopyMemory(lpvPtr1, lpSoundData16, dwBytes1); break;
	}
	// Si la partie Ã  Ã©crire a atteint la fin du buffer, on reboucle au dÃ©but
	if (lpvPtr2 != NULL) {
		switch (nbBitsParEchVoie) {
		case 8 :
			CopyMemory(lpvPtr2, lpSoundData8 + dwBytes1, dwBytes2);
			break;
		case 16 :
			CopyMemory(lpvPtr2, lpSoundData16 + dwBytes1, dwBytes2);
			break;
		}
	}
	hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	if (hr == DSERR_BUFFERLOST) {
		hr = lpDsb->Restore();
		if (hr==DS_OK) hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	}
	if (hr == DS_OK) return true;
	return false;
}

