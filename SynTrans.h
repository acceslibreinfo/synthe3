//Classe Transcription
class Transcription {
private:
	char* texteLec;
	char* texteEcr;
	short iLec;	//lecture
	short iEcr;	//écriture
	unsigned char carac;	//carac alpha
	unsigned char categ;	//sa catégorie
	short ctBava;
	char liaison;
	char* ptArbre;
	char nombre[12];

public:
	void phonemePhoneme(char* texte, char* texPhon);
	void minMajNFois(char* texteAlphaLec, char* texteAlphaEcr);
	void graphemePhoneme(char* texte, char* texPhon);
	void traiteNombre();
	void phonChif(char* ptChif);	//Phonétise le chiffre
	void finNombre(char& iN);
	unsigned char carSuiv();
	void ecritPhon (char c);
	void ecrit (char c);
};