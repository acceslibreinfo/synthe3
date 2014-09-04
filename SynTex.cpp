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

