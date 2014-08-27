////////////////////////////////////////////////////////////////////////////
//	SYNTHÉ : Module de Sortie vers la carte son
//	ELECTREL : été 1998 (stage Pascal MONNIER)
////////////////////////////////////////////////////////////////////////////

// Ce fichier contient les membres publiques de la classe classSon
// gérant les allocations mémoires pour l'initialisation et le transfert des données avec DirectSound.				 

#include "synSon.h"
#include "synCalcul.h"
#include "synGlobal.h"

//Ouvre la carte son
void classSon::open_snd() {
	DSVolume = 0;	//volume max de direct sound
	// Pointeurs renvoyés par GetCurrentPosition
	ptIOctLecDS = new DWORD;
	ptIOctEcrDS = new DWORD;
	// Crée l'objet DirectSound
	HRESULT hr;
	hr = DirectSoundCreate(NULL, &snd_dev, NULL);	//crée un objet snd_dev pour la carte son
	if (hr != DS_OK) return;
	// L'objet DirectSound se base sur le handle de la fenêtre active pour jouer les échantillons.
	Handle = GetForegroundWindow();	//initialisation du Handle de la fenêtre courante
	hr = snd_dev->SetCooperativeLevel(Handle, DSSCL_PRIORITY);
	if (hr != DS_OK) return;
	//Modifie le format du tampon principal
	modifieBufferPrimaire();
	// Crée le buffer secondaire
	hr = creeBufferSecondaire(&lpBuffer);	//crée le buffer circulaire pour la lecture en DMA
	if (hr != DS_OK) return;
	//Positionne le pointeur de lecture en début de buffer
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
//Indique la fin d'écriture du son et transfère des échantillons nuls en attendant l'arrêt de la lecture
//Evite que le dépassement du dernier point d'écriture ne produise un son inopportun
//Permet de démarrer la lecture si le message est très court (écart trop faible pour démarrer sans la suite nulle)
void classSon::sonExit() {
	finirSon(true);	//etatFin=1 (dit à transfert qu'il faudra s'arrêter à limLit), définit la limite limLit
	short ech=0;
	while (transfert((LPVOID)&ech));	//la lecture s'arrêtera à limLit
}

//Si le handle de la fenêtre courante à changé, l'objet DirectSound est remis à jour
bool classSon::initWindow() {
	HWND hwnd = GetForegroundWindow();
	if (hwnd!=Handle) {
		Handle = hwnd;
		HRESULT hr = snd_dev->SetCooperativeLevel(Handle, DSSCL_PRIORITY);
		if (hr!=DS_OK) return false;
	}
	return true;
}

//Arrête le son réversiblement
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

	marqJoueInit=true;	//c'est commencé
	if (marqJoue)	//rien s'il joue déjà
		return true;
	//Joue le buffer secondaire en boucle
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	hr = lpBuffer->Play(0, 0, DSBPLAY_LOOPING);
	if (hr == DSERR_BUFFERLOST) {
		hr = lpBuffer->Restore();
		if (hr == DS_OK) hr = lpBuffer->Play(0, 0, DSBPLAY_LOOPING);
	}
	if (hr == DS_OK) {
		hr = lpBuffer->SetVolume(DSVolume);	// Régle le volume
		marqJoue = true;	//il joue
	}
	quitteSectionCritiqueGlobal();	// Q Q Q Q Q Q Q Q Q Q Q Q
	if (hr == DS_OK) return true;
	return false;
}

//Position de lecture (avec retouches si nécessaire)
void classSon::positionLecture() {
	iEchPosLecAnc=iEchPosLec;	//sauvegarde la position de lecture
	entreSectionCritiqueGlobal();	// E E E E E E E E E E E E E
	HRESULT hr = lpBuffer->GetCurrentPosition(ptIOctLecDS, ptIOctEcrDS);	//demande la nouvelle position à la carte son
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

//Crée le buffer secondaire au format voulu.
//Appelée dans SonInit, renvoie vrai si la création s'est bien déroulée.
HRESULT classSon::creeBufferSecondaire(LPDIRECTSOUNDBUFFER *lplpDsb) {
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbdesc;

	// Format des échantillons sonores à envoyer
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT)); // allocation
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;	// format wave traditionnel
	pcmwf.wf.nChannels = nbVoiesSon;	// nb voies (1=mono, 2=stéréo)
	pcmwf.wf.nSamplesPerSec = fEchCarte;	// nb échantillons par seconde
	pcmwf.wBitsPerSample = nbBitsParEchVoie;	// nb bits par échantillon voie
	pcmwf.wf.nBlockAlign = pcmwf.wf.nChannels*pcmwf.wBitsPerSample/8;	//nb octets par échantillon complet
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;	//nombre moyen d'octets/s
	
	// Description du buffer directsound (circulaire)
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
//	dsbdesc.dwFlags = DSBCAPS_CTRLDEFAULT;
	//Seuls la position et le volume nous intéressent (économie de ressources)
	//Mais DSBCAPS_GLOBALFOCUS permet d'éviter les boîtes de dialogue muettes
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
	// Nombre d'octets dans le buffer secondaire
	dsbdesc.dwBufferBytes = nbEchLpBuffer*nbOctetsParEch;
	// On associe le format des échantillons à la description
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	// Création du buffer directsound à partir de la description
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

	// Description du tampon principal (crée structure DSBUFFERDESC)
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbdesc.dwBufferBytes = 0;	//doit être 0 pour le tampon principal
	dsbdesc.lpwfxFormat = NULL;	//doit être NULL pour le tampon principal
	//Crée structure pour le format désiré
	memset(&wfm,0,sizeof(WAVEFORMATEX));
	wfm.wFormatTag = WAVE_FORMAT_PCM;	// format wave traditionnel
	wfm.nChannels = nbVoiesSon;	// nb voies (1=mono, 2=stéréo)
	wfm.nSamplesPerSec = fEchCarte;	// nb échantillons par seconde
	wfm.wBitsPerSample = nbBitsParEchVoie;	// nb bits par échantillon voie
	wfm.nBlockAlign = wfm.nChannels*wfm.wBitsPerSample/8;	//nb octets par échantillon complet
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;	//nombre moyen d'octets/s
	//Accès au tampon principal
	hr = snd_dev->CreateSoundBuffer(&dsbdesc,&lpDsb,NULL);
	if (hr != DS_OK) return false;
	//Modifie le format du tampon principal. Si erreur, garde le format par défaut
	hr = lpDsb->SetFormat(&wfm);
	if (hr != DS_OK) return false;
	return true;
}

// Ecrit un paquet dans le buffer secondaire (en section critique).
// Si la copie n'a pu avoir lieu, renvoie faux
bool classSon::transferePaquet(LPDIRECTSOUNDBUFFER lpDsb, DWORD dwOffset, LPVOID lpData, DWORD dwSoundBytes) {
	LPVOID lpvPtr1;	// 1er pointeur pour l'écriture
	DWORD dwBytes1;	// nb octets pointés par lpvPtr1
	LPVOID lpvPtr2;	// 2e pointeur pour l'écriture 
	DWORD dwBytes2;	// nb octets pointés par lpvPtr2
	char* lpSoundData8 = (char*) lpData;
	short* lpSoundData16 = (short*) lpData;
	HRESULT hr;

	// On bloque à partir de dwOffset une portion de taille dwSoundBytes.
	// lpvPtr1 pointe le premier bloc du buffer qui doit être bloqué.
	// Si dwBytes1 < dwSoundBytes, alors lpvPtr2 pointera sur un second bloc (début du buffer).
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
	// Si la partie à écrire a atteint la fin du buffer, on reboucle au début
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

