#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
//AQUI TEM A STRING DE REFERENCIA DOS ESLAIDES
int teste[20] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
int teste2[11] = {6, 6 ,3, 0, 4, 8, 2, 3, 3,9, 0};
void FIFOFunction(int tamSR);
void LRUFunction(int tamSR);
void CLOCKFunction(int tamSR);
void CLOCKBFunction(int tamSR);

typedef struct Fila {
	struct Frame* first;
	struct Frame* last;
	struct Frame* it;
	int pageIn;
	int pageOut;
	int qtPag;
} FILA;

typedef struct Frame {
	int pag;
	int bitRef;
	int bitMod;
	struct Frame *next;
	struct Frame *prev;
}FRAME;

FILA* criaFila() {
	FILA* fila = (FILA*) malloc(sizeof(FILA));
	fila->first = NULL;
	fila->last = NULL;
	fila->it = NULL;
	fila->pageIn = 0;
	fila->pageOut = 0;
	fila->qtPag = 0;
	return fila;
}


FRAME* criaFreeFrame() {
	FRAME* novo = (FRAME*) malloc(sizeof(FRAME));
	novo->pag = -1;
	novo->bitRef = 0;
	novo->bitMod = 0;
	novo->prev = NULL;
	novo->next = NULL;
	return novo;
}

void appendLast(FILA* fila, FRAME *nodo) {
	FRAME *aux;
	aux = fila->last;
	if(aux == NULL) {
		fila->first = nodo;
		fila->last = nodo;	
		fila->it = nodo;
	}
	else {
		aux->next = nodo;
		nodo->prev = aux;
		nodo->next = NULL;
		fila->last = nodo;
	}
}

void appendFirst(FILA* fila, FRAME *nodo) {
	FRAME *aux;
	aux = fila->first;
	if(aux == NULL) {
		fila->first = nodo;
		fila->last = nodo;
		fila->it = nodo;
		return;
	}
	aux->prev = nodo;
	nodo->next = aux;
	fila->first = nodo;
	return;
}

FRAME* DeleteAtIterator(FILA *fila){
	FRAME *deleted = fila->it;
	if(fila == NULL)
		return NULL;
	if(fila->it == fila->first) {
		fila->first = fila->it->next;
		fila->first->prev = NULL;
		return deleted;
	}
	if(fila->it == fila->last){
		fila->last = fila->it->prev;
		fila->last->next = NULL;
		return deleted;
	}
	deleted->prev->next = deleted->next;
	deleted->next->prev = deleted->prev;
	return deleted;
}

void FirstFila(FILA *fila){
	fila->it = fila->first;
}

void LastFila(FILA *fila){
	fila->it = fila->last;
}

//Seta o iterador no primeiro frame livre encontrado. Retorna 1 se acha e 0 se n'ao acha
int setIteratorFreeFrame(FILA *fila) {
	FRAME *aux;
	for(aux = fila->first; aux != NULL; aux = aux->next){
		if(aux->pag < 0){
			fila->it = aux;
			return 1;
		}
	}
		return 0;
}

int procuraMemoria(FILA *fila, int pag){
	FRAME* aux;
	for(aux = fila->first; aux != NULL; aux = aux->next){
		if(aux->pag == pag){
			fila->it = aux;
			return 1;
		}
	}
	return 0;
}

int procuraProxClock(FILA *fila, FRAME* stopped) {
	FRAME *aux;
	for(aux = stopped; aux != NULL; aux = aux->next){
		if(aux->bitRef == 0){
			fila->it = aux;
			return 1;
		}	
		aux->bitRef = 0;
	}
	for(aux = fila->first; aux != stopped; aux = aux->next){
		if(aux->bitRef == 0){
			fila->it = aux;
			return 1;
		}	
		aux->bitRef = 0;
	}
	FirstFila(fila);
	return 1;
}
int procuraProxClockA(FILA *fila, FRAME* stopped) {
	FRAME *aux;
	for(aux = stopped; aux != NULL; aux = aux->next){
		if(aux->bitRef == 0){
			fila->it = aux;
			return 1;
		}	
	}
	for(aux = fila->first; aux != stopped; aux = aux->next){
		if(aux->bitRef == 0){
			fila->it = aux;
			return 1;
		}	
	}
	return 0;
}
int procuraProxClockB(FILA *fila, FRAME* stopped) {
	FRAME *aux;
	for(aux = stopped; aux != NULL; aux = aux->next){
		if(aux->bitMod == 0){
			fila->it = aux;
			return 1;
		}	
		aux->bitRef = 0;
	}
	for(aux = fila->first; aux != stopped; aux = aux->next){
		if(aux->bitMod == 0){
			fila->it = aux;
			return 1;
		}	
		aux->bitRef = 0;
	}
	return 1;
}

int *SR, *bitMod;

FILA *FIFO, *LRU, *CLOCK, *CLOCKB;
FRAME* itClock;



int main (int argc, char *argv[]) {
	int i; 
	int pag = 1024;
	int frame = 256;
	int string = 5000;
	for (i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-v") == 0)
			pag = atoi(argv[i+1]);
		else if (strcmp(argv[i], "-f") == 0)
			frame = atoi(argv[i+1]);
		else if (strcmp(argv[i], "-s") == 0)
			string = atoi(argv[i+1]);
	}
	srand(time(NULL));

//	printf("String de referência: ");
	//Allocando espaço para a string de referencia
	SR = (int*) malloc(string*sizeof(int));
	bitMod = (int*) malloc(string*sizeof(int));
	for(i = 0; i < string; i++) {
		*(SR+i) = rand() % pag;
		if(*(SR+i) % 16 < 13)
			*(bitMod+i) = 0;
		else
			*(bitMod+i) = 1;
	}
	//Cria as filas 
	FIFO = criaFila();
	LRU = criaFila();
	CLOCK = criaFila();
	CLOCKB = criaFila();
	//Cria os frames passados pelo usuario, ou não
	for(i = 0; i < frame; i++){
		appendLast(FIFO, criaFreeFrame());
		appendLast(LRU, criaFreeFrame());
		appendLast(CLOCK, criaFreeFrame());
		appendLast(CLOCKB, criaFreeFrame());
	}
	FIFOFunction(string);
	printf("%d,%d,%d\n",FIFO->pageIn,FIFO->pageOut,FIFO->qtPag);

	LRUFunction(string);
	printf("%d,%d,%d\n",LRU->pageIn,LRU->pageOut,LRU->qtPag);

	CLOCKFunction(string);
	printf("%d,%d,%d\n",CLOCK->pageIn,CLOCK->pageOut,CLOCK->qtPag);

	CLOCKBFunction(string);
	printf("%d,%d,%d\n",CLOCKB->pageIn,CLOCKB->pageOut,CLOCKB->qtPag);

}



void FIFOFunction(int tamSR){
	int i,j,pagAtual;
	FRAME *aux, *aux2;
	for(i = 0; i < tamSR; i++){
		//Se a página não está na memória
		if(!procuraMemoria(FIFO, *(SR+i))){
			FIFO->qtPag++;
			//Mas há frames livres
			if(setIteratorFreeFrame(FIFO)){
				aux = DeleteAtIterator(FIFO);
				aux->pag = *(SR+i);
				FIFO->pageIn++;
				appendLast(FIFO, aux);
		}
			//Se não há frames livres, pega o primeiro e o atualiza
			else {
				FirstFila(FIFO);
				aux = DeleteAtIterator(FIFO);
				aux->pag = *(SR+i);
				FIFO->pageIn++;
				FIFO->pageOut++;
				appendLast(FIFO, aux);
			}	
		}
		//Pedaço de código que mostra o estado da memória a cada iteração. Bem legal :)
		/*aux2 = FIFO->first;
		printf("Como está o estado da memória \n");
		for(j = 0; j < 3; j++){
			printf("%d  ", aux2->pag);
			aux2 = aux2->next;
		}
		printf("\n");*/
	}
}

void LRUFunction(int tamSR){
	int i, j, pagAtual;
	FRAME *aux, *aux2;
	for(i = 0; i < tamSR; i++){
		//Se a página não está na memória
		if(!procuraMemoria(LRU, *(SR+i))){
			LRU->qtPag++;
			//Mas há frames livres
			if(setIteratorFreeFrame(LRU)){
				aux = DeleteAtIterator(LRU);
				aux->pag = *(SR+i);
				LRU->pageIn++;
				appendFirst(LRU, aux);
		}
			//Se não há frames livres, pega o primeiro e o atualiza
			else {
				LastFila(LRU);
				aux = DeleteAtIterator(LRU);
				aux->pag = *(SR+i);
				LRU->pageIn++;
				LRU->pageOut++;
				appendFirst(LRU, aux);
			}
		}
		else {
			aux = DeleteAtIterator(LRU);
			appendFirst(LRU, aux);
		}
		//Pedaço de código que mostra o estado da memória a cada iteração. Bem legal :)
		/*aux2 = LRU->first;
		printf("Como está o estado da memória \n");
		for(j = 0; j < 3; j++){
			printf("%d  ", aux2->pag);
			aux2 = aux2->next;
		}
		printf("\n"); */

	}
}



void CLOCKFunction(int tamSR) {
	int i,j;
	FRAME *aux, *aux2;
	itClock = CLOCK->first;

	for (i = 0; i < tamSR; i++){
		if(!procuraMemoria(CLOCK, *(SR+i))){
			CLOCK->qtPag++;
			if(setIteratorFreeFrame(CLOCK)){
				aux = CLOCK->it;
				aux->pag = *(SR+i);
				CLOCK->pageIn++;
				aux->bitRef = 1;
			}
			else {
				CLOCK->pageOut++;
				procuraProxClock(CLOCK, itClock);
				aux = CLOCK->it;
				aux->pag = *(SR+i);
				aux->bitRef = 1;
				CLOCK->pageIn++;
			}
		}
		else{
			CLOCK->it->bitRef = 1;
		}
		itClock = aux->next;
		/*aux2 = CLOCK->first;
		printf("Como está o estado da memória \n");
		for(j = 0; j < 3; j++){
			printf("( %d, %d ) ", aux2->pag, aux2->bitRef);
			aux2 = aux2->next;
		}
		printf("\n");*/
	
	} 	
}

void CLOCKBFunction(int tamSR) {
	int i,j;
	FRAME *aux, *aux2;
	itClock = CLOCKB->first;

	for (i = 0; i < tamSR; i++){
		if(!procuraMemoria(CLOCKB, *(SR+i))){
			CLOCKB->qtPag++;
			if(setIteratorFreeFrame(CLOCKB)){
				aux = CLOCKB->it;
				aux->pag = *(SR+i);
				CLOCKB->pageIn++;
				aux->bitRef = 1;
				aux->bitMod = *(bitMod+i);
			}
			else {
				if(procuraProxClockA(CLOCKB, itClock)){
					aux = CLOCKB->it;
					aux->pag = *(SR+i);
					aux->bitRef = 1;
					aux->bitMod = *(bitMod+i);
					CLOCKB->pageIn++;
				}
				else{
					procuraProxClockB(CLOCKB, itClock);
					aux = CLOCKB->it;
					aux->pag = *(SR+i);
					aux->bitRef = 1;
					aux->bitMod = *(bitMod+i);
					CLOCKB->pageIn++;
					CLOCKB->pageOut++;
				}
			}
		}
		else
			CLOCKB->it->bitRef = 1;
		itClock = aux->next;

		/*aux2 = CLOCKB->first;
		printf("Como está o estado da memória \n");
		for(j = 0; j < 4; j++){
			printf("( %d, %d, %d) ", aux2->pag, aux2->bitRef, aux2->bitMod);
			aux2 = aux2->next;
		}
		printf("\n"); */
	} 	
}



