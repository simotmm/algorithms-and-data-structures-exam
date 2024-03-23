// Esame Algoritmi e Programmazione 
// 09/02/2022
// Simone Tumminelli 

#include <stdio.h>
#include <stdlib.h>
#define GRIGLIA "griglia.txt"       //definizione file in input;
#define PROPOSTA "proposta.txt"
#define SU 0                        //definizione delle mosse;
#define GIU 1
#define DX 2
#define SX 3
#define INIZIO -1                   //definizione della condizione iniziale;

//funzioni: parte di acquisizione
int **leggiMatrice(FILE *fin, int *r, int *c);
int *leggiProposta(FILE *fin, int *n);
//funzioni: parte di verifica
int contaBianche(int **m, int r, int c);
int cambioDir(int d1, int d2);
int **copiaMatrice(int **m, int r, int c);
int checkSol(int **m, int r, int c, int *sol, int n);
//funzioni: parte di ottimizzazione
void solve(int **m, int r, int c);
void solveR(int pos, int *val, int k, int *sol, int *bestSol, int *mark, int n, int **m, int r, int c, int *bestCambi, int *cambi);
int posizioneIdonea(int *sol, int n, int **m, int r, int c, int *cambi);
int *vInit(int n, int x);
int *copiaVettore(int *v, int n);
//altre funzioni
void stampaSol(int **m, int r, int c, int *sol, int n, int bestCambi);
void stampaProposta(int *sol, int n);

int main(){
    int r, c, ok, n;                                            //dati: 1. n. di righe "r" e n. di colonne "c" della matrice;
                                                                //      2. flag "ok" per la verifica;
    //parte I: acquisizione                                     //      3. n. di mosse "n" del vettore di mosse della proposta;
    int **griglia=leggiMatrice(fopen(GRIGLIA, "r"), &r, &c);    //      4. matrice "griglia";
    int *proposta=leggiProposta(fopen(PROPOSTA, "r"), &n);      //      5. vettore delle mosse "proposta";
    if(griglia==NULL || proposta==NULL){
        printf("errore in lettura dei file\n"); return -1;
    }

    //parte II: verifica
    printf("--verifica:--");
    ok=checkSol(griglia, r, c, proposta, n);
    stampaSol(griglia, r, c, proposta, n, ok);
    if(ok==-1) stampaProposta(proposta, n);
    printf("la proposta %se' valida\n", ok==(-1)?"non ":"");

    //parte III: ottimizzazione
    printf("\n--ricerca della soluzione con numero minimo di cambi di direzione:--");
    solve(griglia, r, c);

    return 0;
}

//funzioni: parte di acquisizione
int **leggiMatrice(FILE *fin, int *r, int *c){ //funzione di lettura della matrice da file;
    if(fin==NULL) return NULL;

    fscanf(fin, "%d %d", r, c);
    int i, j, **m=calloc(*r, sizeof(int*));
    for(i=0;i<*r;i++){
        m[i]=calloc(*c, sizeof(int));
        for(j=0;j<*c;j++){
            fscanf(fin, "%d", &m[i][j]);
        }
    }
    fclose(fin);
    return m;
}

int *leggiProposta(FILE *fin, int *n){           // funzione di lettura della matrice da file;
    if(fin==NULL) return NULL;                   //(formato del file: prima riga: numero n di mosse;)
    fscanf(fin, "%d", n);                        //(                  successive n righe: intero che)
    int i, *v=calloc(*n, sizeof(int));           //(                  rappresenta le mosse definite )
    for(i=0;i<*n;i++)                            //(                  dai #define;                  )
        fscanf(fin, "%d", &v[i]);
    fclose(fin);
    return v;
}

//funzioni: parte di verifica
int contaBianche(int **m, int r, int c){        //funzione che conta le caselle bianche (0) di una matrice;
    int i, j, cont=0;
    for(i=0;i<r;i++)
        for(j=0;j<c;j++)
            if(m[i][j]==0)
                cont++;
    return cont;
}

int cambioDir(int d1, int d2){                  //funzione che confronta due mosse per verificare un cambio di direzione;
    if(d1==INIZIO || d2==INIZIO) return 0;      //caso speciale: nel primo passo della verifica il passo precedente �
    return d1!=d2;                              //inizializzato a "INIZIO" (-1), per cui, non essendoci un effettivo passo
}                                               //precedente non c'e' nessun cambio di direzione; in tutti gli altri casi: confronto;

int **copiaMatrice(int **m, int r, int c){      //funzione di copia di una matrice;
    int i, j, **copia=calloc(r, sizeof(int));
    for(i=0;i<r;i++){
        copia[i]=calloc(c, sizeof(int));
        for(j=0;j<c;j++)
            copia[i][j]=m[i][j];
    }
    return copia;
}

int checkSol(int **m, int r, int c, int *sol, int n){  //funzione che ritorna il numero di cambi di direzione oppure -1 in caso di soluzione non valida;
    if(contaBianche(m, r, c)!=(n+1))                           //per una griglia con n caselle bianche sono valide esattamente n-1 mosse
        return -1;                                             //dati: 1. copia della matrice di partenza: serve per segnare le caselle attraversate
    int **copia=copiaMatrice(m, r, c), i=0, j=0, prec=INIZIO;  //         (la prima casella [0][0] e' segnata automaticamente come attraversata (1));
    int cont=0, k=0; copia[0][0]=1;                            //      2. indice i per scorrere la soluzione; 3. indici "k" (righe) e "j" (colonne) per lo spostamento nella
                                                               //         matrice (si parte da [0][0]); 4. "prec" per il confronto tra passo precedente
    for(i=0;i<n;i++){                                          //         e corrente e quindi per contare i cambi di direzione ("prec" inizializzato a "INIZIO", condizione opportunamente trattata da "cambioDir");
        if(cambioDir(prec, sol[i])) cont++;                    //in caso di cambio di direzione aggiorno il contatore;
        switch(sol[i]){                                        //switch per le mosse possibili;
            case SU:  if((k-1)<0  || copia[k-1][j]!=0) return -1;  //per ogni mossa valida (mossa valida: all'interno della matrice && casella non ancora attraversata)
                      copia[--k][j]=1; prec=sol[i]; break;         //la casella corrispondente della matrice viene segnata (1) e quindi non pi� attraversabile;
                                                                   //viene salvata la mossa corrente come precedente per il confronto con la successiva;
            case GIU: if((k+1)>=r || copia[k+1][j]!=0) return -1;  //aggiorno gli indici della matrice in base alla mossa corrispondente
                      copia[++k][j]=1; prec=sol[i]; break;         //(su: riga--, giu': riga++, destra: colonna++, sinistra: colonna--);

            case DX:  if((j+1)>=c || copia[k][j+1]!=0) return -1;
                      copia[k][++j]=1; prec=sol[i]; break;

            case SX:  if((j-1)<0 || copia[k][j-1]!=0)  return -1;
                      copia[k][--j]=1; prec=sol[i]; break;

            default: return -1;                                    //se una mossa non e' valida la soluzione non e' valida
        }
    }

    if(contaBianche(copia, r, c)==0)    //se tutte le caselle sono state attraversate: la soluzione e' valida e si ritorna il totale dei cambi di direzione
        return cont;
    return -1;
}

//funzioni: parte di ottimizzazione
void solve(int **m, int r, int c){     //funzione wrapper della funzione ricorsiva solveR
    int val[4]={SU, GIU, DX, SX}, bestCambi=-1, cambi=-1;                  //dati: 1. vettore "val" delle possibili direzioni; 2. contatori del numero di cambi
    int n=contaBianche(m, r, c)-1; int *sol=calloc(n, sizeof(int));        //         di direzione rispettivamente della soluzione migliore e della soluzione corrente;
    int *bestSol=calloc(n, sizeof(int)),  *mark=vInit(n, n);               //      3. vettori "sol" e "bestSol" rispettivamente della soluzione migliore e della
                                                                           //         soluzione corrente, la loro dimensione e' il numero di caselle bianche della matrice-1 (n);
    solveR(0, val, 4, sol, bestSol, mark, n, m, r, c, &bestCambi, &cambi); //      4. vettore dei markaggi multipli "mark" di dimensione n i cui valori sono tutti n: ogni
    stampaSol(m, r, c, bestSol, n, bestCambi);                             //         elemento di val puo' essere usato nelle soluzioni al piu' n volte;
}                                                                          //"solveR" calcola la soluzione migliore; "stampaSol" stampa la soluzione oppure comunica l'inesistenza delle soluzioni;

//"solveR" si basa sul modello delle disposizioni con ripetizione: vengono calcolate tutte le sequenze
//ordinate in cui i 4 (k) elementi di "val" (su, giu, dx, sx) si dispongono in un insieme di n elementi
//(n: dimensione dei delle soluzioni); tra queste disposizioni si verificano quelle idonee, e, tra
//queste  idonee, vengono selezionate quelle ottime, cioe' con minor numero di cambi di direzione; le
//disposizioni calcolate non sono effettivamente possibili -> pruning: la ricorsione puo' essere fermata
//se si verificano determinate condizioni per le quali e' impossibile trovare una soluzione ottima;
//dati: 1. posizione "pos"; 2. vettore dei valori delle mosse "val" e la sua dimensione "k";
//      3. vettori (di dimensione "n") delle soluzioni corrente e ottima "sol", "bestSol" e vettore dei
//         markaggi multipli "mark" (ogni ognuno dei k elementi di val puo' essere preso per un massimo di
//         n volte); 4. griglia di gioco "m" e le sue dimensioni "r" e "c"; 5. puntatori dei contatori
//         dei cambi di direzione per la soluzione ottima e corrente (rispettivamente "bestCambi" e "cambi");
void solveR(int pos, int *val, int k, int *sol, int *bestSol, int *mark, int n, int **m, int r, int c, int *bestCambi, int *cambi){
    int cambiCorrenti;                                                  //viene calcolato il numero di cambi di direzione  della soluzione corrente e un flag "ok" per indicare se
    int ok=posizioneIdonea(sol, pos, m, r, c, &cambiCorrenti);          //la posizione corrente nella matrice e' idonea e la soluzione corrente e' idonea;
    if( (*bestCambi>=0 && cambiCorrenti>*bestCambi) || ok==-1 )         //pruning: se si e' trovata una soluzione ottima (cioe' se i cambi della sol. ottima sono maggiori di 0)
        return;                                                         //e se i cambi della soluzione corrente sono maggiori dei cambi di quella ottima, oppure se la soluzione non e'
//                                                                      //idonea la ricorsione termina;
    if(pos>=n){                                                         //condizione di terminazione
        *cambi=checkSol(m, r, c, sol, n);                               //si calcola il numero di cambi della soluzione corrente e si confrantano con quelli della soluzione ottima: nel
        if(*cambi>=0){                                                  //caso di un nuovo minimo si aggiorna la soluzione ottima (se non esiste ancora una sol. ottima allora quella
            if(*bestCambi<0 || (*bestCambi>=0 && *cambi<*bestCambi) ){  //appena trovata e' la nuova soluzione ottima;
                *bestCambi=*cambi;
                free(bestSol);
                bestSol=copiaVettore(sol, n);
            }
        }
        return;
    }

    int i;
    for(i=0;i<k;i++){           //per ogni elemento in "val" (su, giu, dx, sx),
        if(mark[i]>0){          //se e' possibile prenderlo, si assegna come nuovo elemento della soluzione
            mark[i]--;
            sol[pos]=val[i];
            solveR(pos+1, val, k, sol, bestSol, mark, n, m, r, c, bestCambi, cambi); //ricorsione avanzando di posizione
            mark[i]++;          //backtrack
        }
    }
    return;
}

int posizioneIdonea(int *sol, int n, int **m, int r, int c, int *cambi){ //uguale a checkSol ma considera il caso in cui tutte le mosse siano idonee ma non bastano, serve per il pruning;
    int **copia=copiaMatrice(m, r, c), i=0, j=0, prec=INIZIO;            //(l'ho implementata per evitare di modificare "checkSol", che e' rimasta uguale alla versione cartacea);
    int cont=0, k=0; copia[0][0]=1;

    for(i=0;i<n;i++){
        if(cambioDir(prec, sol[i])) cont++;
        switch(sol[i]){
            case SU:  if((k-1)<0  || copia[k-1][j]!=0) return -1;
                      copia[--k][j]=1; prec=sol[i]; break;

            case GIU: if((k+1)>=r || copia[k+1][j]!=0) return -1;
                      copia[++k][j]=1; prec=sol[i]; break;

            case DX:  if((j+1)>=c || copia[k][j+1]!=0) return -1;
                      copia[k][++j]=1; prec=sol[i]; break;

            case SX:  if((j-1)<0 || copia[k][j-1]!=0)  return -1;
                      copia[k][--j]=1; prec=sol[i]; break;

            default: return -1;
        }
    }
    *cambi=cont;
    if(contaBianche(copia, r, c)==0)
        return cont;
    return -2;                           //codice speciale "-2": tutte le mosse sono valide ma non bastano per coprire tutte le caselle bianche;
}

int *vInit(int n, int x){                //funzione che inizializza un vettore di dimensione n con valori tutti uguali a x;
    int i, *v=malloc(n*sizeof(int));
    for(i=0;i<n;i++)
        v[i]=x;
    return v;
}

int *copiaVettore(int *v, int n){        //funzione di copia di un vettore;
    int i, *copia=malloc(n*sizeof(int));
    for(i=0;i<n;i++)
        copia[i]=v[i];
    return copia;
}

//altre funzioni
void stampaProposta(int *sol, int n){    //funzione di stampa di un vettore di mosse;
    int i, cont=0;
    printf("mosse (%d): \n{", n);
    for(i=0;i<n;i++){
        if(cont==5){
            printf("\n ");
            cont=0;
        }
        switch(sol[i]){
            case SU:  printf("%8s", "SU");       break;
            case GIU: printf("%8s", "GIU'");     break;
            case DX:  printf("%8s", "DESTRA");   break;
            case SX:  printf("%8s", "SINISTRA"); break;
            default:                             break;
        }
        printf("%s", i==(n-1)?"}\n":", ");
        cont++;
    }
}

void stampaSol(int **m, int r, int c, int *sol, int n, int cambi){ //funzione di stampa della soluzione;
    int i, j;
    printf("\ngriglia (%dx%d):\n", r, c);                          //stampa della griglia di gioco;
    for(i=0;i<r;i++)
        for(j=0;j<c;j++)
            printf("%d%s", m[i][j], j==(c-1)?"\n":" ");
    if(cambi<0)                                                    //stampa del vettore di mosse in caso di soluzione idonea;
        printf("soluzione non trovata\n");
    else{
        stampaProposta(sol, n);
        printf("cambi di direzione totali: %d\n", cambi);
    }
}
