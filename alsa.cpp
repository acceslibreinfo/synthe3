/*
 * ALSA output module for Synthé 3 - use ALSA to let Synthé 3 speak aloud text
 *
 * Copyright (C) 1985-2014 by the CRISCO laboratory.
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

#include <string.h>
#include <iostream>
#include "SynSon.h"
//#include "SynCalcul.h"

using namespace std;

void classSon::open_snd() {
	//Name of the PCM device, like plughw:0,0. The first number is the number of the soundcard, the second number is the number of the device.
	char* pcm_name=strdup("default");;
	int rc;

	while (true) {
		// Open PCM. The last parameter of this function is the mode. If this is set
		// to 0, the standard mode is used. Possible other values are SND_PCM_NONBLOCK
		// and SND_PCM_ASYNC. If SND_PCM_NONBLOCK is used, read / write access to the
		// PCM device will return immediately. If SND_PCM_ASYNC is specified, SIGIO will
		// be emitted whenever a period has been completely processed by the soundcard.
		if ((rc = snd_pcm_open(&snd_dev, pcm_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			cerr << "L'ouverture de la carte son a échoué : " << snd_strerror(rc) << endl;
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
	int periods = 4;  /* Number of periods */
	snd_pcm_uframes_t periodsize = 8192; /* Periodsize (bytes) */
	
	snd_pcm_hw_params_alloca(&params);

	// Initialisation des paramètres
	if (snd_pcm_hw_params_any(snd_dev, params) < 0) {
		cerr << "Impossible d'initialiser les paramètres." << endl;
	}
	// Interleaved mode
	if (snd_pcm_hw_params_set_access(snd_dev, params,SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		cerr << "Impossible de configurer le type d'accès." << endl;
	}
	// Format des échantillons : signed 16-bit little-endian
	if (snd_pcm_hw_params_set_format(snd_dev, params,SND_PCM_FORMAT_S16_LE) < 0) {
		cerr << "Impossible de configurer le format." << endl;
	}
	// Nombre de voies (mono ou stéréo)
	if (snd_pcm_hw_params_set_channels(snd_dev, params, /*channels*/2) < 0) {
		cerr << "Impossible de configurer le nombre de voies." << endl;
	}
	// Fréquence. Si la fréquence exacte n'est pas supportée par la carte, on utilise la plus proche.
	exact_rate = rate;
	if (snd_pcm_hw_params_set_rate_near(snd_dev, params, (unsigned int*)&exact_rate, &dir) < 0) {
		cerr << "Impossible de configurer la fréquence." << endl;
	}
	if (rate != exact_rate) {
		cerr << "La fréquence " << rate << " n'est pas supportée, valeur utilisée : " << exact_rate << endl;
	}
	// Nombre de périodes
	if (snd_pcm_hw_params_set_periods(snd_dev, params, periods, 0) < 0) {
		cerr << "Impossible de configurer la période à " << periods << "." << endl;
	}
	// Taille du buffer (en frames). The resulting latency is given by latency = periodsize * periods / (rate * bytes_per_frame)
	if (snd_pcm_hw_params_set_buffer_size(snd_dev, params, (periodsize * periods)>>2) < 0) {
		cerr << "Impossible de configurer la taille du buffer." << endl;
	}
	// Ecriture des paramètres sur la carte
	if ((rc = snd_pcm_hw_params(snd_dev, params)) < 0) {
		cerr << "Impossible de configurer la carte son : " << snd_strerror(rc) << endl;
	}
}

void classSon::get_snd_params() {
  	snd_pcm_hw_params_t *params;
 	unsigned int val, val2;
  	int dir;
  	snd_pcm_uframes_t frames;
 
	/* Display information about the PCM interface */

	printf("PCM snd_dev name = '%s'\n",snd_pcm_name(snd_dev));

	printf("PCM state = %s\n",snd_pcm_state_name(snd_pcm_state(snd_dev)));

	snd_pcm_hw_params_get_access(params, 
			(snd_pcm_access_t *) &val);
	printf("access type = %s\n", 
			snd_pcm_access_name((snd_pcm_access_t)val));

  //snd_pcm_hw_params_get_format(params, &val);
  //printf("format = '%s' (%s)\n",
  //  snd_pcm_format_name((snd_pcm_format_t)val),
  //  snd_pcm_format_description(
  //                           (snd_pcm_format_t)val));

	snd_pcm_hw_params_get_subformat(params,
			(snd_pcm_subformat_t *)&val);
	printf("subformat = '%s' (%s)\n",
			snd_pcm_subformat_name((snd_pcm_subformat_t)val),
			snd_pcm_subformat_description(
				(snd_pcm_subformat_t)val));

	snd_pcm_hw_params_get_channels(params, &val);
	printf("channels = %d\n", val);
	
	snd_pcm_hw_params_get_rate(params, &val, &dir);
	printf("rate = %d bps\n", val);

	snd_pcm_hw_params_get_period_time(params, &val, &dir);
	printf("period time = %d us\n", val);

	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	printf("period size = %d frames\n", (int)frames);

	snd_pcm_hw_params_get_buffer_time(params, &val, &dir);
	printf("buffer time = %d us\n", val);

	snd_pcm_hw_params_get_buffer_size(params,
			(snd_pcm_uframes_t *) &val);
	printf("buffer size = %d frames\n", val);

	snd_pcm_hw_params_get_periods(params, &val, &dir);
	printf("periods per buffer = %d frames\n", val);

	snd_pcm_hw_params_get_rate_numden(params, &val, &val2);
	printf("exact rate = %d/%d bps\n", val, val2);

	val = snd_pcm_hw_params_get_sbits(params);
	printf("significant bits = %d\n", val);

	snd_pcm_hw_params_get_tick_time(params, &val, &dir);
	printf("tick time = %d us\n", val);

	val = snd_pcm_hw_params_is_batch(params);
	printf("is batch = %d\n", val);

	val = snd_pcm_hw_params_is_block_transfer(params);
	printf("is block transfer = %d\n", val);

	val = snd_pcm_hw_params_is_double(params);
	printf("is double = %d\n", val);

	val = snd_pcm_hw_params_is_half_duplex(params);
	printf("is half duplex = %d\n", val);

	val = snd_pcm_hw_params_is_joint_duplex(params);
	printf("is joint duplex = %d\n", val);

	val = snd_pcm_hw_params_can_overrange(params);
	printf("can overrange = %d\n", val);

	val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
	printf("can mmap = %d\n", val);

	val = snd_pcm_hw_params_can_pause(params);
	printf("can pause = %d\n", val);

	val = snd_pcm_hw_params_can_resume(params);
	printf("can resume = %d\n", val);

	val = snd_pcm_hw_params_can_sync_start(params);
	printf("can sync start = %d\n", val);
}

void classSon::sonExit() {}	//fin du message : termine le buffer proprement sous DirectSound
bool classSon::pauseSiJoue() { return false; }	//stoppe le son sous DirectSound
bool classSon::joueSiPause() { return false; }	//démarre le son sous DirectSound
void classSon::positionLecture() {}	//fournit iEchPosLec sous DirectSound

bool classSon::transferePaquet(void* lpData, int dwSoundBytes) {
	int rc;
	snd_pcm_uframes_t periodsize = 8192;
	int frames=periodsize >> 2;

	while ((rc = snd_pcm_writei(snd_dev, lpData, frames)) < 0) {
		snd_pcm_prepare(snd_dev);
		cerr << "underrun occurred" << endl;
	}
	
	return false;
}
