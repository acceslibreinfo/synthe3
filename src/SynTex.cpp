////////////////////////////////////////////////////
// Synthé : fonction parole
////////////////////////////////////////////////////

#include "SynMain.h"
#include "SynParle.h"
#include "SynTrans.h"

//Variables de SynTex.cpp
char tamponAlpha[NM_CAR_TEX];
char tamponPhon[NM_CAR_TEX];

//Synthèse à partir du texte
void synTex(void* lpPara) {
	typeParamThread* lpParamThread=(typeParamThread*)lpPara;

	synGlobal.setThreadOK(true);
	synGlobal.setNbIndexLec(1);	//valeur de départ (pour cas phonétique, sans index)
	//Positionne les paramètres
	synGlobal.setPhon(lpParamThread->phon);
	synGlobal.setVolume(lpParamThread->volume);
	synGlobal.setDebit(lpParamThread->debit);
	synGlobal.setHauteur(lpParamThread->hauteur);
	synGlobal.setModeLecture(lpParamThread->modeLecture);
	synGlobal.setModeCompta(lpParamThread->modeCompta);
	synGlobal.setSortieSon(lpParamThread->sortieSon);
	synGlobal.setSortieWave(lpParamThread->sortieWave);
	synGlobal.setNomFichierWave(lpParamThread->nomFicWave);
	Transcription* synTranscription=new Transcription;
	if (!synGlobal.getPhon()) {	//si le texte est alphabétique
		synTranscription->minMajNFois(lpParamThread->texte, tamponAlpha);	//min->maj et n fois
		synTranscription->graphemePhoneme(tamponAlpha, tamponPhon);	//phonétise
	} else
		synTranscription->phonemePhoneme(lpParamThread->texte, tamponPhon);	//phonétise
	delete synTranscription;
	initWave(true);	//initialise un éventuel fichier wave
	Parle* synParle=new Parle;
	synParle->traiteTextePhonetique(tamponPhon);	//prononce le texte phonétique
	delete synParle;
	initWave(false);	//termine un éventuel fichier wave
	sonDestruction();	//détruit l'objet classSon
	delete[] lpParamThread->texte;
	delete (typeParamThread*)lpPara;
	synGlobal.setNbIndexLec(-1);	//indique la fin de la lecture
}

