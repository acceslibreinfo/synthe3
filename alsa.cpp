/*
 * ALSA output module for Synthé 3 - use ALSA to let Synthé 3 speak aloud text
 *
 * Copyright (C) 1985-2015 by the CRISCO laboratory,
 * university of Caen (France).
 *
 * This ALSA output module for Synthé 3 comes with ABSOLUTELY NO WARRANTY.
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

#ifndef WIN32	//totalité du fichier (utilisé exclusivement sous Linux)

#include "SynSon.h"
#include <iostream>
#include <unistd.h>

void classSon::open_snd() {
	// Name of the PCM device, like plughw:0,0. The first number is the number of the
	// soundcard, the second number is the number of the device.
	char* pcm_name=strdup("default");;
	int rc;

	while (true) {
		// Open PCM. The last parameter of this function is the mode. If this is set
		// to 0, the standard mode is used. Possible other values are SND_PCM_NONBLOCK
		// and SND_PCM_ASYNC. If SND_PCM_NONBLOCK is used, read / write access to the
		// PCM device will return immediately. If SND_PCM_ASYNC is specified, SIGIO will
		// be emitted whenever a period has been completely processed by the soundcard.
		if ((rc = snd_pcm_open(&snd_dev, pcm_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			std::cerr << "L'ouverture de la carte son a échoué : " << snd_strerror(rc) << std::endl;
			usleep(1000);
		} else {
			snd_ok=true;
			break;
		}
	}
}

void classSon::close_snd() {
	snd_pcm_drain(snd_dev);
	snd_pcm_close(snd_dev);
}

void classSon::set_snd_params(int channels, int bits, int rate) {
	int rc;
	snd_pcm_hw_params_t *params;
	int exact_rate;   /* Sample rate returned by snd_pcm_hw_params_set_rate_near */ 
	int dir;          /* exact_rate == rate --> dir = 0 */
	                  /* exact_rate < rate  --> dir = -1 */
	                  /* exact_rate > rate  --> dir = 1 */
	snd_pcm_uframes_t periodsize = nbEchPaquet; /* Periodsize (bytes) */
	
	snd_pcm_hw_params_alloca(&params);

	// Initialisation des paramètres
	if (snd_pcm_hw_params_any(snd_dev, params) < 0) {
		std::cerr << "Impossible d'initialiser les paramètres." << std::endl;
	}
	// Interleaved mode
	if (snd_pcm_hw_params_set_access(snd_dev, params,SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		std::cerr << "Impossible de configurer le type d'accès." << std::endl;
	}
	// Format des échantillons : signed 16-bit little-endian
	if (snd_pcm_hw_params_set_format(snd_dev, params,SND_PCM_FORMAT_S16_LE) < 0) {
		std::cerr << "Impossible de configurer le format." << std::endl;
	}
	// Nombre de voies (mono ou stéréo)
	if (snd_pcm_hw_params_set_channels(snd_dev, params, /*channels*/2) < 0) {
		std::cerr << "Impossible de configurer le nombre de voies." << std::endl;
	}
	// Fréquence. Si la fréquence exacte n'est pas supportée par la carte, on utilise la plus proche.
	exact_rate = rate;
	if (snd_pcm_hw_params_set_rate_near(snd_dev, params, (unsigned int*)&exact_rate, &dir) < 0) {
		std::cerr << "Impossible de configurer la fréquence." << std::endl;
	}
	if (rate != exact_rate) {
		std::cerr << "La fréquence " << rate << " n'est pas supportée, valeur utilisée : " << exact_rate << std::endl;
	}
	// Nombre de périodes
	if (snd_pcm_hw_params_set_periods(snd_dev, params, nbPaquetsLpBuffer, 0) < 0) {
		std::cerr << "Impossible de configurer la période à " << nbPaquetsLpBuffer << "." << std::endl;
	}
	// Taille du buffer (en frames).
	// The resulting latency is given by latency = periodsize * nbPaquetsLpBuffer / rate
	if (snd_pcm_hw_params_set_buffer_size(snd_dev, params, (periodsize * nbPaquetsLpBuffer)) < 0) {
		std::cerr << "Impossible de configurer la taille du buffer." << std::endl;
	}
	// Ecriture des paramètres sur la carte
	if ((rc = snd_pcm_hw_params(snd_dev, params)) < 0) {
		std::cerr << "Impossible de configurer la carte son : " << snd_strerror(rc) << std::endl;
	}
}

void classSon::sonExit() {}	//fin du message : termine le buffer proprement sous DirectSound
bool classSon::pauseSiJoue() { return false; }	//stoppe le son sous DirectSound
bool classSon::joueSiPause() { return false; }	//démarre le son sous DirectSound
void classSon::positionLecture() {}	//fournit iEchPosLec sous DirectSound

bool classSon::transferePaquet(void* lpData, int dwSoundBytes) {
	int rc;
	int frames=nbEchPaquet;

	while ((rc = snd_pcm_writei(snd_dev, lpData, frames)) < 0) {
		snd_pcm_prepare(snd_dev);
		std::cerr << "underrun occurred" << std::endl;
	}
	
	return false;
}

#endif	//WIN32
