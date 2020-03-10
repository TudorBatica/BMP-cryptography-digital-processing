#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//STRUCTURA PENTRU STOCAREA COORDONATELOR CARTEZIENE A UNUI PUNCT
typedef struct
{
    int x;
    int y;
}pct;

// STRUCTURA PENTRU FEREASTRA
typedef struct
{
    int width, height;
    float corelatie;
    int centru;
    int cifra;
    pct central;
    pct coltStSus;
    pct coltDrJos;
}Fereastra;

typedef union
{
    unsigned char octet[4];
    unsigned int nr;
}Octet;

                                        //CRIPTARE + DECRIPTARE
// GENERATOR DE NUMERE PSEUDOALEATOARE
unsigned int xorshift32(unsigned int numar)
{
	numar ^= numar << 13;
	numar ^= numar >> 17;
	numar ^= numar << 5;
	return numar;
}

// FUNCTIE PENTRU INCARCAREA IN MEMORIA INTERNA A UNEI IMAGINI, IN FORMA LINIARIZATA
void incarcareImg (char * cale, unsigned int *width, unsigned int *height, unsigned char **header, unsigned char **imgLin)
{
    //deschidem fisierul pt citire
    FILE *f = fopen(cale, "rb");
    //verificam daca a putut fi deschis
    if(f == NULL)
    {
        printf("Eroare la deschiderea fisierului");
        return;
    }
    //citim latimea
    fseek(f, 18, SEEK_SET);
    fread(&(*width), sizeof(unsigned int), 1, f);
    //citim inaltimea
    fseek(f, 22, SEEK_SET);
    fread(&(*height), sizeof(unsigned int), 1, f);
    //citim headerul
    (*header) = (unsigned char *)malloc (54*sizeof(unsigned char));
    fseek(f, 0, SEEK_SET);
    fread(&(**header), 54, 1, f);

    //calculam paddingul imaginii
    int padding;
    if((*width) % 4 != 0) padding = 4 - (3 * (*width)) % 4;
    else padding = 0;

    unsigned char alCateleaOctet[3]; // alCateleaOctet[i] va reprezenta a i-a culoare(R / G / B) din fiecare pixel
    //alocam memorie pentru imaginea in forma liniarizata (cate 3 octeti pentru fiecare pixel)
    (*imgLin) = (unsigned char *) malloc (3*(*height)*(*width)*sizeof(unsigned char));
    //copiam pixelii din fisier in imaginea in forma liniarizata
    int indiceLinie=0;
    int i,j,octet;
    fseek(f, 54, SEEK_SET);
	for(i = 0; i < (*height); i++)
	{
		for(j = 0; j < (*width); j++)
		{
			fread(alCateleaOctet, 3, 1, f);
        	for(octet = 0; octet < 3; octet++)
            {
                (*imgLin)[indiceLinie] = alCateleaOctet[octet];
                indiceLinie++;
            }
		}
		//sarim octetii de padding, daca acestia exista
		fseek(f,padding,SEEK_CUR);
	}
	//inchidem fisierul
	fclose(f);
}

//  FUNCTIE PENTRU SALVAREA UNEI IMAGINI IN MEMORIA EXTERNA
void salvareImg (char * cale, unsigned int width, unsigned int height, unsigned char *header, unsigned char *imgLin)
{
    //deschidem fisierul pt scriere
    FILE *f = fopen(cale, "wb");
    //verificam daca s-a putut deschide fisierul
    if(f == NULL)
    {
        printf("Eroare la deschiderea fisierului");
        return;
    }

    //copiam headerul
    int i;
    for(i=0; i<54; i++)
    fwrite(&header[i], 1, 1, f);

    int indiceLinie = 0;
    int l,j;
    unsigned char ptPadding = 0;
    //calculam padding-ul
    int padding;
    if(width % 4 != 0) // daca imaginea are padding, copiam pixelii din fisierul "f" si adaugam numarul corespunzator de octeti pt padding
    {
        padding = 4 - (3 * width) % 4;
        for(i=0; i<height; i++)
            {
            for(j=1; j <= 3*width+padding; j++)
                {
                if(j <= 3*width)
                    {
                    fwrite(&imgLin[indiceLinie], 1, 1, f);
                    indiceLinie++;
                    }
                else
                    fwrite(&ptPadding, 1, 1, f);
                }
            }
    }
    else
    {
            //daca imaginea nu are padding, doar copiam toti octetii din "f"
            for(i = 0; i < 3*height*width; i++)
                fwrite(&imgLin[i], 1, 1, f);
    }

    //inchidem fisierul
    fclose(f);
}


void criptare(char * cale_decriptata, char * cale_criptata, char * chei)
{
    unsigned int width;
    unsigned int heigth;
    unsigned char *header;
    unsigned char *imgLin;
    int i,j,l;

    //incarcam in memoria interna imaginea ce trebuie criptata
    incarcareImg(cale_decriptata, &width, &heigth, &header, &imgLin);

    //alocam memoria pentru secventa "R" de numere pseudealeatoare
    unsigned int *R = (unsigned int *) malloc(2*width*heigth*sizeof(unsigned int));
    unsigned int SV;

    //deschidem fisierul text in care se afla cheile secrete
    FILE *cheiSecrete = fopen(chei, "r");
    //initializam primul numar din secventa cu prima cheie din fisier
    fscanf(cheiSecrete, "%u", &R[0]);
    //citim si cheia pentru substitutie aici, pentru a putea inchide apoi fisierul
    fscanf(cheiSecrete, "%u", &SV);
    fclose(cheiSecrete);

    //generam secventa "R" cu xorshift32
    for(i=1; i < 2*heigth*width; i++)
    R[i] = xorshift32(R[i-1]);

    //permutarea pixelilor
    unsigned int lungimeaPermutarii = width*heigth;
    unsigned int *deLa0LalungimeSigma = (unsigned int *) malloc(lungimeaPermutarii*sizeof(unsigned int));
    unsigned int *sigmaPermutat = (unsigned int *) malloc(lungimeaPermutarii*sizeof(unsigned int));

    for(i=0; i<lungimeaPermutarii; i++)
    deLa0LalungimeSigma[i] = i;

    //Cream permutarea aleatoare folosind algoritmul lui Durstenfeld
    unsigned int roll, aux;
    for(i = 0; i < width * heigth; i++)
    {
        roll = R[i]%(lungimeaPermutarii);
        sigmaPermutat[width*heigth - i - 1] = deLa0LalungimeSigma[roll];
        aux = deLa0LalungimeSigma[roll];
        deLa0LalungimeSigma[roll] = deLa0LalungimeSigma[lungimeaPermutarii-1];
        deLa0LalungimeSigma[lungimeaPermutarii-1] = aux;
        lungimeaPermutarii--;
    }

    //permutam pixelii din imagine, folosind "sigmaPermutat"
    //mai intai, alocam memorie pentru imaginea permutata in forma liniarizata
    unsigned char *imgLinPerm = (unsigned char *) malloc (3 * width * heigth * sizeof(unsigned char));
    for(i=0; i<width*heigth; i++)
    {
        for(j=0; j<3; j++) //mutam "blocuri de cate 3 octeti din imaginea liniarizata in imaginea permutata liniarizata
        imgLinPerm [3*i+j] = imgLin [3*sigmaPermutat[i]+j];
    }

    //impartim cheia SV in octeti
    unsigned char SV_octet_1 = SV % 256;
    unsigned char SV_octet_2 = (SV / 256) % 256;
    unsigned char SV_octet_3 = ((SV / 256) / 256) % 256;

    //alocam memorie pentru imaginea criptata in forma liniarizata
    unsigned char *imgLinCript = (unsigned char *) malloc (3 * heigth * width * sizeof(unsigned char));

    Octet auxiliar; // acesta va avea rolul de a imparti fiecare numar din secventa "R" in octeti

    //substituim fiecare pixel
    for(j=0; j<3; j++)
        {
        auxiliar.nr = R[width * heigth];
        imgLinCript[j] = SV_octet_1 ^ imgLinPerm[0] ^ auxiliar.octet[j];
        }
    for(i=1; i<width * heigth; i++)
    {
            for(j=0; j<3; j++)
            {
                    auxiliar.nr = R[width * heigth + i];
                    imgLinCript[3*i+j] = imgLinCript[3*i+j-3] ^ imgLinPerm[3*i+j] ^ auxiliar.octet[j];
            }
    }

    //salvam imaginea criptata in memoria externa
    salvareImg(cale_criptata, width, heigth, header, imgLinCript);
}

void decriptare(char * cale_criptata, char * cale_decriptata, char * chei)
{
    unsigned int width;
    unsigned int height;
    unsigned char *header;
    unsigned char *imgLinCriptata;
    int i,j,l;

    //incarcam imaginea ce trebuie decriptata
    incarcareImg(cale_criptata, &width, &height, &header, &imgLinCriptata);
    //incarcareImg(cale_criptata, &w, &h, &header, &criptareL);

    // generare xorshift32
    //alocam memorie pentru secventa "R" si pentru cheia secreta "SV"
    unsigned int *R = (unsigned int *) malloc(2 * width * height * sizeof(unsigned int));
    unsigned int SV;

    //deschidem fisierul text in care se afla cheile secrete
    FILE *cheiSecrete = fopen(chei, "r");
    //initializam primul numar din secventa cu prima cheie din fisier
    fscanf(cheiSecrete, "%u", &R[0]);
    //citim si cheia pentru substitutie aici, pentru a putea inchide apoi fisierul
    fscanf(cheiSecrete, "%u", &SV);
    fclose(cheiSecrete);

    //generam secventa "R" cu xorshift32
    for(i = 1; i < 2*height * width; i++)
    R[i] = xorshift32(R[i-1]);
    Octet auxiliar; //va imparti cheia SV in octeti
    auxiliar.nr = SV;
    unsigned char SV_octet_1 = auxiliar.octet[0];
    unsigned char SV_octet_2 = auxiliar.octet[1];
    unsigned char SV_octet_3 = auxiliar.octet[2];
    //alocam memorie pentru liniarizarea imaginii decriptate
    unsigned char *imgLinDecriptata = (unsigned char *) malloc ( 3* height * width * sizeof(unsigned char));

    for(l=0; l<3; l++)
            {
                auxiliar.nr = R[width*height]; //va imparti numerele din secventa "R" in octeti
                imgLinDecriptata[l] = SV_octet_1 ^ imgLinCriptata[0] ^ auxiliar.octet[l];
            }
    for(i = 1; i < width*height; i++)
    {
            for(l=0; l<3; l++)
            {
                    auxiliar.nr = R[width*height+i];
                    imgLinDecriptata[3*i+l] = imgLinCriptata[3*i+l-3] ^ imgLinCriptata[3*i+l] ^ auxiliar.octet[l];
            }
    }

    //alocam memorie pentru sigma
    unsigned int lungimeaPermutarii = width*height;
    unsigned int *deLa0LalungimeSigma = (unsigned int *) malloc(lungimeaPermutarii*sizeof(unsigned int));
    unsigned int *sigmaPermutat = (unsigned int *) malloc(lungimeaPermutarii*sizeof(unsigned int));
    unsigned int *sigmaInvers = (unsigned int *) malloc(lungimeaPermutarii*sizeof(unsigned int));

    for(i=0; i<lungimeaPermutarii; i++)
    deLa0LalungimeSigma[i] = i;

    //Cream permutarea aleatoare folosind algoritmul lui Durstenfeld
    unsigned int roll, aux;
    for(i = 0; i < width * height; i++)
    {
        roll = R[i]%(lungimeaPermutarii);
        sigmaPermutat[width*height - i - 1] = deLa0LalungimeSigma[roll];
        aux = deLa0LalungimeSigma[roll];
        deLa0LalungimeSigma[roll] = deLa0LalungimeSigma[lungimeaPermutarii-1];
        deLa0LalungimeSigma[lungimeaPermutarii-1] = aux;
        lungimeaPermutarii--;
    }

    //generam inversa permutarii sigma
    for(i = 0; i < width * height; i++)
    sigmaInvers[ sigmaPermutat[i] ] = i;

    //alocam memorie pentru imaginea finala, decriptata, in forma liniarizata
    unsigned char *initialLiniarizata = (unsigned char *) malloc (3* width * height * sizeof(unsigned char));

    //mutam pixelii la pozitia initiala(cea din imaginea originala)
    for(i=0; i < width * height; i++)
    {
        for(j=0; j<3; j++)
        initialLiniarizata[3*i+j] = imgLinDecriptata [ 3*sigmaInvers [i] + j];
    }

    //salvam imaginea decriptata in memoria externa
    salvareImg(cale_decriptata, width, height, header, initialLiniarizata);
}

//testul chi-patrat
void chiPatrat(char *cale)
{
    unsigned int width;
    unsigned int height;
    unsigned char *header;
    unsigned char *imgLin;
    int i;

    //incarmca imaginea
    incarcareImg(cale, &width, &height, &header, &imgLin);
    //calculam frecventa
    float frecventa = (float)(width*height) / 256.0;
    //alocam memorie pentru fiecare culoare
    unsigned int *R = (unsigned int *) calloc(256, sizeof(unsigned int));
    unsigned int *G = (unsigned int *) calloc(256, sizeof(unsigned int));
    unsigned int *B = (unsigned int *) calloc(256, sizeof(unsigned int));
    //calculez frecventa fiecarei valori posibile a fiecarei culori
    for(i=0; i<width*height; i++)
    {
        B[(unsigned int)imgLin[3*i]]++;
        G[(unsigned int)imgLin[3*i+1]]++;
        R[(unsigned int)imgLin[3*i+2]]++;
    }
    float rosu=0, verde=0, albastru=0;
    for(i = 0; i <= 255; i++)
    {
        rosu += ((R[i] - frecventa) * (R[i] - frecventa)) / frecventa;
        verde += ((G[i] - frecventa) * (G[i] - frecventa)) / frecventa;
        albastru += ((B[i] - frecventa) * (B[i] - frecventa)) / frecventa;
    }

    printf("%.2f %.2f %.2f\n", rosu, verde, albastru);
}

                                    //TEMPLATE MATCHING

//FUNCTIE PENTRU TRANSFORMAREA UNEI IMAGINI COLOR IN GRAYSCALE
void grayScale (char *cale_color, char *cale_gray)
{
    unsigned int width;
    unsigned int height;
    unsigned char *header;
    unsigned char *imgLin;
    unsigned char aux;
    int i;
    //incarcam imaginea ce trebuie transformata in grayscale
    incarcareImg(cale_color, &width, &height, &header, &imgLin);

    //schimbam culoarea fiecarui pixel
    for(i=0; i<width*height; i++)
    {
        aux = 0.299*imgLin[3*i+2] + 0.587*imgLin[3*i+1] + 0.114*imgLin[3*i];
        imgLin[3*i+2] = imgLin[3*i+1] = imgLin[3*i] = aux;
    }

    //salvam imaginea in memoria externa
    salvareImg(cale_gray, width, height, header, imgLin);
}

//functie pt qsort, pentru sortarea in functie de corelatie
int cmp(const void *a, const void *b)
{
    Fereastra x = *(Fereastra *) a;
    Fereastra y = *(Fereastra *) b;
    if(x.corelatie < y.corelatie)
        return 1;
    if(x.corelatie > y.corelatie)
        return -1;
    return 0;
}
int getR(int cifra)
{
    unsigned char R;
    if (cifra == 0)
        R = 255;
    else if (cifra == 1)
        R = 255;
    else if (cifra == 2)
        R = 0;
    else if (cifra == 3)
        R = 0;
    else if (cifra == 4)
        R = 255;
    else if (cifra == 5)
        R = 0;
    else if (cifra == 6)
        R = 192;
    else if (cifra == 7)
        R = 255;
    else if (cifra == 8)
        R = 128;
    else if (cifra == 9)
        R = 128;
    return R;
}

int getG (int cifra)
{
    unsigned char G;
    if (cifra == 0)
        G = 0;
    else if (cifra == 1)
        G = 255;
    else if (cifra == 2)
        G = 255;
    else if (cifra == 3)
        G = 255;
    else if (cifra == 4)
        G = 0;
    else if (cifra == 5)
        G = 0;
    else if (cifra == 6)
        G = 192;
    else if (cifra == 7)
        G = 140;
    else if (cifra == 8)
        G = 0;
    else if (cifra == 9)
        G = 0;
    return G;
}

int getB (int cifra)
{
    unsigned char B;
    if (cifra == 0)
        B = 0;
    else if (cifra == 1)
        B = 0;
    else if (cifra == 2)
        B = 0;
    else if (cifra == 3)
        B = 255;
    else if (cifra == 4)
        B = 255;
    else if (cifra == 5)
        B = 255;
    else if (cifra == 6)
        B = 192;
    else if (cifra == 7)
        B = 0;
    else if (cifra == 8)
        B = 128;
    else if (cifra == 9)
        B = 0;
    return B;
}
void templateMatching (char *cale_img, char *cale_img_gray, char *cale_cifra, char *cale_img_detectii, int cifra, Fereastra **D, int *indexFereastra, float prag)
{

    unsigned int width, widthSablon;
    unsigned int height, heightSablon;
    unsigned char *header, *headerSablon;
    unsigned char *imgLin, *imgSablon, *imgGray, *img_final;
    int i,j,l;

    //incarcam imaginea initiala
    incarcareImg(cale_img, &width, &height, &header, &imgLin);

    //daca suntem la prima apelare de templateMatching vom transforma imaginea initiala in gray scale
    if(cifra == 0) grayScale(cale_img, cale_img_gray);

    //incarcam imaginea grayscale
    incarcareImg(cale_img_gray, &width, &height, &header, &imgGray);

    //daca suntem la prima apelare a functiei incarcam in vectorul img final incarcam imaginea initiala in grayscale
    if(cifra == 0) incarcareImg(cale_img_gray, &width, &height, &header, &img_final);
    //altfel incarcam imaginea deja desenata de la apelurile anterioare
    else incarcareImg(cale_img_detectii, &width, &height, &header, &img_final);

    if(cifra == 0)
    salvareImg(cale_img_detectii, width, height, header, img_final);
    //incarcam sablonul
    incarcareImg(cale_cifra, &widthSablon, &heightSablon, &headerSablon, &imgSablon);
    //transformam sablonul in grayscale
    grayScale(cale_cifra, cale_cifra);
    //salvam sablonul
    salvareImg(cale_cifra, widthSablon, heightSablon, headerSablon, imgSablon);

    float c;
    float sigmaF, sigmaS, Sbarat = 0, Fbarat = 0, auxs, auxf;

    Fereastra * aux; // folosim o fereastra auxiliara pentru realocarea memoriei
    salvareImg(cale_img_gray, width, height, header, imgGray);
    //alocam memorie pentru un vector ce reprezenta liniarizarea fiecarei ferestre posibile
    unsigned char *liniarizareActuala = (unsigned char *) malloc (3*165*sizeof(unsigned char));

    int centru;
    int k=0;
    int index;
    int index2;
    int linie;
    int coloana;
    //parcurgem imaginea pixel cu pixel, mutand centrul ferestrei cu cate un pixel in stanga pana este posibil, apoi cu cate un pixel in sus
    for(index = 7; index <= height-8; index++)
    {
        linie = index;
        coloana = 4;
        for(centru = 5+index*width; centru <= (index+1)*width-6; centru++)
        {
            k=0;
            coloana ++;
            //liniarizam fereastra actuala
            for(i = centru - 5 - 7*width ; i <= centru - 5 + 7*width; i += width)
            {
                for(j = 0; j < 11; j++)
                {
                    for(l=0; l<3; l++)
                    {
                        liniarizareActuala[k] = imgGray[3*(i+j)+l];
                        k++;
                    }
                }
            }
            //reinitializam cu zero valorile specifice fiecarei ferestre
            sigmaF = 0;
            Fbarat = 0;
            sigmaS = 0;
            Sbarat = 0;
            c = 0;

            //calculam s barat si f barat
            for(index2 = 0; index2 < 165; index2++)
                {
                    Sbarat += imgSablon[3*index2];
                    Fbarat += liniarizareActuala[3*index2];
                }
            Sbarat /= 165;
            Fbarat /= 165;

            //calculam sigma S si sigma F
            for(index2 = 0; index2 < 165; index2++)
                {
                    sigmaS += (imgSablon[3*index2] - Sbarat) * (imgSablon[3*index2] - Sbarat);
                    sigmaF += (liniarizareActuala[3*index2] - Fbarat) * (liniarizareActuala[3*index2] - Fbarat);
                }
            sigmaF /= 164;
            sigmaS /= 164;
            sigmaF = sqrt(sigmaF);
            sigmaS = sqrt(sigmaS);

            //calculam acum si corelatia
            for(index2 = 0; index2 < 165; index2++)
            {
                c += (1 / (sigmaF * sigmaS)) * (imgSablon[3*index2] - Sbarat) * (liniarizareActuala[3*index2] - Fbarat);
            }
            c/=165;


            if(c >= prag)
            {
                aux = (Fereastra *) realloc (*D, ((*indexFereastra)+1) * sizeof(Fereastra));
                    (*D) = aux;
                    (*D)[*indexFereastra].corelatie = c;
                    (*D)[*indexFereastra].centru = centru;
                    (*D)[*indexFereastra].cifra = cifra;
                    (*D)[*indexFereastra].central.x = coloana;
                    (*D)[*indexFereastra].central.y = linie;
                    (*D)[*indexFereastra].coltStSus.x = (*D)[*indexFereastra].central.x - 5;
                    (*D)[*indexFereastra].coltStSus.y = (*D)[*indexFereastra].central.y + 7;
                    (*D)[*indexFereastra].coltDrJos.x = (*D)[*indexFereastra].central.x + 5;
                    (*D)[*indexFereastra].coltDrJos.y = (*D)[*indexFereastra].central.y - 7;

                    unsigned char R = getR(cifra);
                    unsigned char G = getG(cifra);
                    unsigned char B = getB(cifra);
                    contur(cale_img_detectii, R, G, B, centru, &img_final, width, height, header);
                    (*indexFereastra)++;
            }
        }
    }
}

//functie care creeaza conturul
void contur(char *caleImg, unsigned char R, unsigned char G, unsigned char B, unsigned int centru, unsigned char **imgLin, unsigned int width, unsigned int height, unsigned char *header)
{
    int i;
    //coloram partea de sus a chenarului
    for(i=-5; i<=5; i++)
    {
        (*imgLin)[3*(centru + 7*width + i)] = B;
        (*imgLin)[3*(centru + 7*width + i)+1] = G;
        (*imgLin)[3*(centru + 7*width + i)+2] = R;
    }
    //coloram partea de jos a chenarului
    for(i=-5; i<=5; i++)
    {
        (*imgLin)[3*(centru - 7*width + i)] = B;
        (*imgLin)[3*(centru - 7*width + i)+1] = G;
        (*imgLin)[3*(centru - 7*width + i)+2] = R;
    }
    //coloram partea din stanga a chenarului
    for(i=-7; i<=7; i++)
    {
        (*imgLin)[3*(centru - 5 + width*i)] = B;
        (*imgLin)[3*(centru - 5 + width*i)+1] = G;
        (*imgLin)[3*(centru - 5 + width*i)+2] = R;
    }
    //coloram partea din dreapta a chenarului
    for(i=-7; i<=7; i++)
    {
        (*imgLin)[3*(centru + 5 + width*i)] = B;
        (*imgLin)[3*(centru + 5 + width*i)+1] = G;
        (*imgLin)[3*(centru + 5 + width*i)+2] = R;
    }

    //salvam imaginea
    salvareImg(caleImg, width, height, header, *imgLin);

}

//functie care calculeaza daca 2 ferestre se suprapun, folosind coordonatele colturilor din stanga sus si dreaota jos a fiecarui ferestre
int suprapunereFerestre(Fereastra a, Fereastra b)
{
    if (a.coltStSus.x > b.coltDrJos.x || b.coltStSus.x > a.coltDrJos.x)
        return 0;

    if (a.coltStSus.y < b.coltDrJos.y || b.coltStSus.y < a.coltDrJos.y)
        return 0;
    return 1;
}

//functie care calculeaza minimul a 2 numere
int min(int nr1, int nr2)
{
    if(nr1 < nr2) return nr1;
    return nr2;
}

//functie care calculeaza maximum a 2 numere
int max(int nr1, int nr2)
{
    if(nr1 > nr2) return nr1;
    return nr2;
}

//functie care calculeaza modulul unei functii, necesara pentru calcularea ariei de suprapunere
int modul (int numar)
{
    if(numar >= 0) return numar;
    return numar * (-1);
}

//functie care calculeaza aria suprapunerii a doua ferestre
float suprapunere (Fereastra a, Fereastra b)
{
    float s;
    float arieIntersectie;
    //verificam daca ferestrele se suprapun
    if(suprapunereFerestre(a,b) == 0) return 0;

    arieIntersectie = (float) modul((min(a.coltDrJos.x, b.coltDrJos.x) - max(a.coltStSus.x, b.coltStSus.x)) * (min(a.coltDrJos.y, b.coltDrJos.y) - max(a.coltStSus.y, b.coltStSus.y)));
    s = (float) arieIntersectie / (165 + 165 - arieIntersectie);
    return s;
}

//functie pentru eliminarea non maximelor
void eliminareNonMaxime(Fereastra **D, int *indexFereastra)
{
    int i,j,k;
    //parcurgem tot vectorul de detectii, comparand fiecare element cu toate elementele de dupa el
    for(i = 0; i < (*indexFereastra)-1; i++)
    {
        for(j = i+1; j < (*indexFereastra); j++)
        {
            //cand gasim doua detectii cu o suprapunere mai mare de 0.2, pastram elementul cu corelatia mai mare
            //vectorul fiind deja sortat descrescator in fct de corelatie inainte de apelarea functiei de eliminare a non maximelor,
            //elementul cu corelatia cea mai mare va fi intotdeauna primul ( D[i] )
            if(suprapunere((*D)[i], (*D)[j]) > 0.2)
            {
                for(k = j; k < (*indexFereastra); k++)
                    (*D)[k] = (*D)[k + 1];
                (*indexFereastra) --;
            }
        }
    }
}

//functie pentru colorarea imaginii folosind doar detectiile ramase
void colorareFinal (Fereastra *D, int indexFereastra, char *cale, char *cale_imagineFinala)
{
    unsigned int width;
    unsigned int height;
    unsigned char *header;
    unsigned char *imgLin;
    int i;
    //incarcam imaginea
    incarcareImg(cale, &width, &height, &header, &imgLin);
    //coloram detectiile ramase
    for(i=0; i<indexFereastra; i++)
    {
        unsigned char R = getR(D[i].cifra);
        unsigned char G = getG(D[i].cifra);
        unsigned char B = getB(D[i].cifra);
        contur(cale_imagineFinala, R, G ,B , D[i].centru, &imgLin, width, height, header);
    }
}

int main()
{
        //CRIPTARE + DECRIPTARE "peppers.bmp" + chiPatrat

    criptare("peppers.bmp", "peppers_criptat.bmp", "secret_key.txt");
    decriptare("peppers_criptat.bmp", "peppers_final.bmp", "secret_key.txt");
    chiPatrat("peppers.bmp");
    chiPatrat("peppers_criptat.bmp");

        //TEMPLATE MATCHING
    //alocam memorie pentru vectorul "D" de detectii
    Fereastra *D = (Fereastra *) calloc (1, sizeof(Fereastra));
    int indexFereastra = 0;
    //aplicam templateMatching pentru toate cifrele
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra0.bmp", "toateDetectiile.bmp",0, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra1.bmp", "toateDetectiile.bmp",1, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra2.bmp", "toateDetectiile.bmp",2, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra3.bmp", "toateDetectiile.bmp",3, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra4.bmp", "toateDetectiile.bmp",4, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra5.bmp", "toateDetectiile.bmp",5, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra6.bmp", "toateDetectiile.bmp",6, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra7.bmp", "toateDetectiile.bmp",7, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra8.bmp", "toateDetectiile.bmp",8, &D, &indexFereastra, 0.5);
    templateMatching("test.bmp", "testGrayScale.bmp", "cifra9.bmp", "toateDetectiile.bmp",9, &D, &indexFereastra, 0.5);
    //sortam vectorul de detectii
    qsort(D, indexFereastra, sizeof(Fereastra), cmp);
    //eliminam non maximele
    eliminareNonMaxime(&D, &indexFereastra);
    //coloram imaginea finala, doar cu detectiile ramase
    colorareFinal(D, indexFereastra, "test.bmp", "template_Matching_Final.bmp");

    return 0;
}
