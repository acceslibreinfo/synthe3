///////////////////////////////////////////////////////
// *** SYNTHE ***
// Synthé logiciel (fonctions publiques)
///////////////////////////////////////////////////////

#ifdef WIN32
	#define EXTERNE extern "C" _declspec(dllexport)	//pour Synthe.h
#else // linux ...
	#define EXTERNE extern "C"
#include <unistd.h>
	#include <pthread.h>
#endif

#define SYNTHE_VERSION "Synthé version 1.1"
//#define SYNTHE_VOIX "" //Chemin et nom du fichier de voix
//#define SYNTHE_TAB "" //Chemin et nom du fichier des tables

#include <ctime>
#include <string.h>
#include "SynMain.h"
#include "Synthe.h"
#include "SynVoix.h"

//Variables de Synthe.cpp
HANDLE hThread;
short posLec[NM_INDEX+1];	//il y a une place de plus que d'index
short nbIndex;
Voix** tVoix;	//tableau des voix
Tab* tab;	//les tables de Synthé (objet, ne pas confondre avec tTab, table qui fait partie de cet objet)

//////////////////////////////////////////////////////////////////
// Fonctions publiques pour faire parler Synthé
//////////////////////////////////////////////////////////////////

//Les fonctions de Synthé sont en mode _stdcall pour les appels à partir d'autres langages (Visual Basic, ...)
//Le nombre d'index est calculé avant de créer le thread, pour être bon dès le début (min 1 pendant le son)

//Envoi d'un texte à lire par Synthé
//Le paramètre texte est obligatoire, les autres sont facultatifs, la valeur -1 indique la conservation de la valeur courante
void _stdcall synTexte(
	char* texte,	//peut être constitué de plusieurs paragraphes, sans dépasser NM_CAR_TEX caractères.
	short volume,	//0 à 15 par pas de 25 % (par défaut 10)
	short debit,	//0 à 15 par pas de 12 % (par défaut 4)
	short hauteur,	//0 à 15 par pas de 12 % (par défaut 4)
	short phon, 	//1, le texte est phonétique
	short modeLecture,	//0, normal, 1, dit la ponctuation
	short modeCompta,	//0, le séparateur de milliers reste, 1, le séprateur de milliers est enlevé
	short sortieSon,		//sortie sur la carte-son
	short sortieWave,		//sortie sous forme de fichier wave
	char* nomFicWave	//nom éventuel du fichier à construire
	) {
	
	typeParamThread* lpParamThread=new typeParamThread;
#ifdef WIN32
	DWORD idThread;
#endif
	char* bufferMessage;
	long longTex;	//longueur texte

	demandeStopEtAttendFinThread();	//stop texte en cours
	if (!texte) {
		delete lpParamThread;
		return;
	}
	longTex=strlen((char*)texte);
	if (longTex>NM_CAR_TEX) longTex=NM_CAR_TEX;
	bufferMessage=new char[longTex*2+10];	//au pire : "a b c d ..."
	//Demande la version de Synthé
	if (strncmp(texte,"nversionsyn",9)==0) strcpy (bufferMessage, SYNTHE_VERSION);
	copieEtAjouteIndexSiPas(texte, bufferMessage);	//compte les index
	//Crée le thread
	lpParamThread->texte=bufferMessage;
	lpParamThread->phon=phon;
	lpParamThread->volume=volume;
	lpParamThread->debit=debit;
	lpParamThread->hauteur=hauteur;
	lpParamThread->modeLecture=modeLecture;
	lpParamThread->modeCompta=modeCompta;
	lpParamThread->sortieSon=sortieSon;
	lpParamThread->sortieWave=sortieWave;
	lpParamThread->nomFicWave=nomFicWave;
	synGlobal.setThreadOK(false);
#ifdef WIN32
	hThread=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&fThAlpha, lpParamThread, 0, &idThread);
	SetPriorityClass(hThread, HIGH_PRIORITY_CLASS);
	Sleep(0);	//accélère la création du thread
#else
	pthread_create(&hThread, NULL, (void*(*)(void*))fThAlpha, lpParamThread);
	sleep(0);	//accélère la création du thread
#endif
}

//Retourne la valeur de l'index de lecture (va du nb d'index à 0 en fin de lecture)
short synIndex() {
	return synGlobal.getNbIndexLec();
}

//Retourne la position de lecture du texte
short _stdcall synPosLec() {
	short n=synGlobal.getNbIndexLec();
	if (n<1) return -1;
	return posLec[nbIndex-n];
}

//Stop parole : arrête la lecture (effet immédiat)
//Indispensable quand on arrête un programme utilisant Synthé avec DirectX
//	sous peine de bouclage irréversible du tampon de lecture
void _stdcall synStop() {
	demandeStopEtAttendFinThread();
}

/////////////////////////////
//	Fonctions privées
/////////////////////////////

//Démarrage du thread : synthèse à partir du texte
void fThAlpha(void* lpParam) {
	synTex(lpParam);
}

//Si thread en cours : demande stop puis attend la fin du thread
void demandeStopEtAttendFinThread() {
#ifdef WIN32
	if (WaitForSingleObject(hThread,0)==WAIT_TIMEOUT) {
		synGlobal.setDemandeStop(true);	//demande le stop
		WaitForSingleObject(hThread,INFINITE); //et attend la fin du Thread
		synGlobal.setDemandeStop(false);	//OK, on peut envoyer un nouveau message
	}
#else
	if (hThread) {
		synGlobal.setDemandeStop(true);	//demande le stop (positionne une globale)
		//pthread_cancel(hThread);
		pthread_join(hThread, NULL);
		synGlobal.setDemandeStop(false);	//OK, on peut envoyer un nouveau message
	}
#endif
	synGlobal.setNbIndexLec(-1);	//-1 -> 0 indique lecture terminée (à 0 -> 1, il reste ce qui suit le dernier index)
	initWave(false);	//termine une éventuelle sortie wave
	sonDestruction();	//détruit l'objet son synSon (donc le buffer circulaire et l'objet carte-son snd_dev)
}

//Indexation automatique : copie la chaine en ajoutant des index si elle n'en comporte pas (et les compte)
void copieEtAjouteIndexSiPas (char* chaineLec, char* chaineEcr) {
	char* lec;
	char* ecr=chaineEcr;
	bool marqValide=false;

	posLec[0]=0;	//position initiale
	nbIndex=1;	//init comptage à 1 pour posLec car 0 représente le 1er mot (mais on décrémente après)
	for (lec=chaineLec; *lec!=0; lec++) {
		if (*lec=='ø') {
			if (lec[1]=='í') {	//index trouvé
				if (nbIndex<=NM_INDEX)
					posLec[nbIndex++]=lec-chaineLec;	//repère l'index et compte
				lec++;
			}
		}
	}
	if (nbIndex>1) {
		//La chaine comporte des index
		ecr=chaineEcr;
		for (lec=chaineLec; *lec!=0 && lec<chaineLec+NM_CAR_TEX; lec++)	//limite la lecture à NM_CAR_TEX carac
			*ecr++=*lec;	//recopie simplement
		*ecr=0;
		synGlobal.setNbIndexLec(nbIndex-1);	//valeur pour Synthé avant mise à jour par lecture tampon
		synGlobal.setNbIndexMax(nbIndex-1);	//pour index sous linux
		return;
	}
	//Pas d'index : on les place
	ecr=chaineEcr;
	for (lec=chaineLec; *lec!=0 && lec<chaineLec+NM_CAR_TEX; lec++) {	//limite la lecture à NM_CAR_TEX carac
		if (*lec==0) {
			break;	//si finit par marqueur
		}
		if (!caracValide((char)*lec)) {
			if (marqValide) {	//place l'index derrière le car valide et le repère
				if (nbIndex<NM_INDEX) {
					posLec[nbIndex++]=lec-chaineLec;
					*ecr++=MARQ_MARQ; *ecr++=MARQ_INDEX;
				}
			}
			marqValide=false;
		} else
		  marqValide=true;  //prêt à placer le prochain index
		*ecr++=*lec;
	}
	*ecr++=MARQ_MARQ; *ecr++=MARQ_INDEX;*ecr=0;	//finit par un index
	posLec[nbIndex]=lec-chaineLec;	//position du dernier index (inutile car il sera ignoré et la vraie fin de lecture renverra -1)
	synGlobal.setNbIndexEcr(nbIndex);	//à écrire dans le tampon circulaire (min 1 pendant le son)
	synGlobal.setNbIndexLec(nbIndex);	//à retourner avant mise à jour par lecture tampon (min 1 pendant le son)
	synGlobal.setNbIndexMax(nbIndex);	//Romain pour index sous linux
}

//Détermine s'il s'agit d'un caractère ou d'un séparateur
bool caracValide(char carac) {
	if (carac==' ' || carac==10 || carac==13) return false;	//on pourrait affiner avec qques carac ou un tableau
	return true;
}

////////////////////////////////////
// Initialisation de Synthé
////////////////////////////////////

//Au lancement
void initSynthe() {
	initVariablesSectionCritiqueGlobal();
	synGlobal.initTNEch(NM_INDEX);	//pour index sous Linux
	synGlobal.setNbIndexLec(0);
	synGlobal.setDemandeStop(false);
	synGlobal.setVolume(10);
	synGlobal.setDebit(4);
	synGlobal.setHauteur(6);
	synGlobal.setPhon(0);
	synGlobal.setModeLecture(0);
	synGlobal.setModeCompta(1);
	synGlobal.setSortieSon(1);
	synGlobal.setSortieWave(0);
	tVoix=new Voix*[1];	//une seule voix (prévu pour plusieurs)
	tVoix[0]=new Voix(0, "Michel.seg");	//construit la voix 0 (la seule)
	tab=new Tab("Synthe.tab");	//construit les tables
	//		synTexte("synthé prêt");
}

//Pour quitter
void quitteSynthe() {
	demandeStopEtAttendFinThread();	//stoppe le message en cours, index à -1, détruit l'objet son synSon, donc le buffer circulaire et l'objet carte-son snd-dev
	synGlobal.destTNEch();	//index sous Linux
	synGlobal.setNomFichierWave(NULL);
	detruitVariablesSectionCritiqueGlobal();
	delete tab;
	delete tVoix[0];
	delete[] tVoix;
}

//Initialisation chargement/déchargement la dll (pour Linux, il faut appeler initSynthe et quitteSynthe
#ifdef WIN32
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, void* lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		initSynthe();
		break;
	case DLL_PROCESS_DETACH:
		quitteSynthe();
		break;
	}
	return true;
}
#endif
