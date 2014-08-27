#ifndef __SYN_PARLE_H_
#define __SYN_PARLE_H_

//Structures
struct Marq {
	bool DSDecroit;	//début sous-diphone décroit
	bool DSCroit;	//début sous-diphone croit
	bool FSDecroit;	//fin sous-diphone décroit
	bool FSCroit;	//fin sous-diphone croit
};

struct UChar {
	unsigned char DSDecroit;	//début sous-diphone décroit
	unsigned char DSCroit;	//début sous-diphone croit
	unsigned char FSDecroit;	//fin sous-diphone décroit
	unsigned char FSCroit;	//fin sous-diphone croit
};

struct PUChar {
	unsigned char* DSDecroit;	//début sous-diphone décroit
	unsigned char* DSCroit;	//début sous-diphone croit
	unsigned char* FSDecroit;	//fin sous-diphone décroit
	unsigned char* FSCroit;	//fin sous-diphone croit
};

struct PSChar {
	char* DSDecroit;	//début sous-diphone décroit
	char* DSCroit;	//début sous-diphone croit
	char* FSDecroit;	//fin sous-diphone décroit
	char* FSCroit;	//fin sous-diphone croit
};

//////////////////////
// classe Parle
//////////////////////
class Parle {
private:
	float mulHauteur;
	float mulDebit;
	float mulVolume;
	short sortieSon;
	short sortieWave;
	char* texPhon;
	float xEcrEchelleDeLec;	//position écriture à l'échelle de la lecture pour comparaison
	float allonge;	//allongement du ':'
	long iLecPhon;	//indice lecture chaine phonétique
	long iLecPhonDecroit;	//id selon le point de vue
	long iLecPhonCroit;	//id selon le point de vue
	char phonGDecroit;	//1er phonème du diphone décroit
	char phonGCroit;	//1er phonème du diphone croit
	char phonDDecroit;	//2ème phonème du diphone décroit
	char phonDCroit;	//2ème phonème du diphone croit
	char phonMem;	//mémorisé
	char catGDecroit;	//catégorie de phonGDecroit
	char catGCroit;	//catégorie de phonGCroit
	char catDDecroit;	//catégorie de phonDDecroit
	char catDCroit;	//catégorie de phonDCroit
	char nSousDiphDecroit;	//0, 1, 2 n° de sous-diphone décroit
	char nSousDiphCroit;	//0, 1, 2 n° de sous-diphone croit
	Marq marq;	//présence d'un sous-diphone dans le mélange
	UChar nSeg;	//n° de segment
	UChar nSegIni;	//nSeg initial
	PSChar ptSeg;	//pt sur segment
	UChar perio;	//période
	float perioBase;	//période avant action de la hauteur
	unsigned char perioResult;		// période résultante
	PUChar ptAmp;	//pt sur amplitudes
	UChar amp;	//amplitude
	UChar ampAnc;	//amplitude précédente (pour interpolation linéaire)
	short iEch;	//échantillon dans la période

public:
	void traiteTextePhonetique(char* chainePhon);

private:
	//Sort une période du signal
	bool traiteUnePeriode();
	//Initialise tous les Décroit sur les Croit
	void initDecroitSurCroit();
	//Prépare les variables de la période suivante
	bool perioSuiv(bool& marqDS, bool& MarqFS, long& iLecPhonX, char& phonG, char& phonD, char& catG, char& catD,
		char& nSousDiph, unsigned char& nSegIniDS, unsigned char& nSegIniFS,
		unsigned char& nSegDS, unsigned char& nSegFS, char*& ptSegDS, char*& ptSegFS,
		unsigned char& perioDS, unsigned char& perioFS,
		unsigned char*& ptAmpDS, unsigned char*& ptAmpFS,
		unsigned char& ampDS, unsigned char& ampFS, bool mCroit);
	//Prépare les variables du sous-diphone décroit suivant
	bool nouveauSousDiph(long& iLecPhonX, char& phonG, char& phonD, char& catG, char& catD, char& nSousDiph,
		unsigned char& nSegIniDS, unsigned char& nSegDS, unsigned char*& ptAmpDS,
		unsigned char& ampDS);
	//Phonème suivant dans le texte phon
	char phonSuiv(long& iLecPhonX, char phonG);
	//Calcule la part croissante de la période
	short calculeEchPerioCroit(short x, char* ptSegDecroit, unsigned char perioDecroit, unsigned char ampDecroit);
	//Calcule la part décroissante de la période
	short calculeEchPerioDecroit(short x, char* ptSegCroit, unsigned char perioCroit, unsigned char ampCroit);
};

#endif