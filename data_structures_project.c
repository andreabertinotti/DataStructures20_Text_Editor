#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_LEN 1024         
#define MAX_COM_LEN 100

//______  VARIABILI GLOBALI_____
char bufferCommand[MAX_COM_LEN+1];
char bufferInput[MAX_STR_LEN+1];
//FILE *fp;


//____DEFINIZIONE TIPI DI DATO__
typedef enum { false, true } boolean;

typedef struct Command {
    char type;
    int begin;
    int end;
    int actionsNum;
} Command;


//___struttura per il testo attuale______
typedef struct Line {
    struct Line *next;
    struct Line *prev;
    char *text;
} Line;
//____struttura che tiene i puntatori a testa e coda della lista____
typedef struct list {
    Line *head;
    Line *tail;
    int lineTotNum;
} List;
List list = {NULL, NULL, 0};

//struttura per il salvataggio della storia
typedef struct HistoryNode {
    struct HistoryNode *next;
    struct HistoryNode *prev;
    int begin, end;
    int skip;
    int number;
    char commandType;
    char *savedLines[];
} HistoryNode;

//______struttura che tiene i puntatori alla lista della storia
typedef struct ListH {
    HistoryNode *head;
    HistoryNode *tail;
    HistoryNode *curr;
} ListH;
ListH listH = {NULL, NULL};

//____struttura per salvare il testo in un array d'appoggio quando lo modifico nell'array originale_______
//_____FUNZIONI______
//______utilità_______

//  restituisce l'input in una striga delle demensioni corrette

char* crop(char *input) {

    int inputLen;
    char *output;

    inputLen = strlen(input) + 1;

    output = malloc(sizeof(char)*inputLen);
    strcpy(output, input);

    return output;

}


// analisi comando
Command commandParser(char* rawCommand, Command currentCommand) {
    int part2Lenght;
    char *part1, *part2;
    if(strcmp(rawCommand, "q\n") == 0) {        //gestione caso particolare comando q
        currentCommand.type = 'q';
        currentCommand.begin = 0;
        currentCommand.end = 0;
        currentCommand.actionsNum = 0;
        return currentCommand;
    }
    if(strrchr(rawCommand, ',')) {
        part1 = strtok(rawCommand, ",");
        part2 = strtok(NULL, ",");
    } else {
        part2 = rawCommand;
    }
    part2Lenght = strlen(part2);
    currentCommand.type = part2[part2Lenght-2];
    switch (currentCommand.type) {
        case 'c':
        case 'd':
        case 'p':
            currentCommand.begin = atoi(part1);
            currentCommand.end = atoi(part2);
            currentCommand.actionsNum = 0;
            break;
        case 'u':
        case 'r':
            currentCommand.actionsNum = atoi(part2);
            currentCommand.begin = 0;
            currentCommand.end = 0;
            break;
        default:
            currentCommand.begin = 0;
            currentCommand.end = 0;
            currentCommand.actionsNum = 0;
            break;
    }
    return currentCommand;
}

void addToHistory(int begin, int end, int skip, char Command /*'d' per delete e 'c' per change*/, char **toSave) {
    HistoryNode *toDelete = NULL, *tmp = NULL;
    HistoryNode *newHistoryNode;
    if(skip == 1) {
        newHistoryNode = malloc(sizeof(HistoryNode) + sizeof(char *));
        newHistoryNode->savedLines[0] = NULL;
    } else
        newHistoryNode = malloc(sizeof(HistoryNode) + sizeof(char*) * (end-begin+1));
    newHistoryNode->commandType = Command;
    newHistoryNode->begin = begin;
    newHistoryNode->end = end;
    newHistoryNode->skip = skip;

    if(skip != 1) {
        int i;
        for (i = 0; i < end - begin + 1; i++) {       //copio gli indirizzi delle linee di testo
            newHistoryNode->savedLines[i] = toSave[i];
        }
    }
    newHistoryNode->next = NULL;           //il nuovo nodo della storia lo inserisco dopo il nodo listH.current e sarà l'ultimo della lista
    listH.tail = newHistoryNode;            //visto che è l'ultimo nodo sarà la tail di listH
    newHistoryNode->prev = listH.curr;

    if(newHistoryNode->prev == NULL)        //se non ha un nodo che lo precede è il primo e ha come numero progressivo 1
        newHistoryNode->number = 1;
    else                                    //se ha un nodo che lo precede ha come numero progressivo quello successivo a quello del nodo che lo precdede
        newHistoryNode->number = newHistoryNode->prev->number + 1;

    if(listH.curr == NULL) {               //se curr è NULL allora ho eseguito tutti gli undo possibili
        toDelete = listH.head;
        listH.head = NULL;
        listH.tail = NULL;
    }
    if(listH.head == NULL) {               //se lista è vuota, newHistoryNode è sia head che curr che tail
        listH.head = newHistoryNode;
        listH.curr = newHistoryNode;
        listH.tail = newHistoryNode;
    } else {
        toDelete = listH.curr->next;       //salvo tutto quello che seguiva curr, per poi cancellarlo (visto che non servirà più, perchè sto modificando)
        listH.curr->next = newHistoryNode;
        listH.curr = newHistoryNode;
    }
    //cancello tutta la parte di cronologia dopo curr (inutile perchè è stata effettuata una modifica)
    while(toDelete != NULL) {
        if(toDelete->skip != 1) {
            int i;
            for (i = 0; i < toDelete->end - toDelete->begin + 1; i++) {
                //if (toDelete->savedLines[i] != NULL)
                //free(toDelete->savedLines[i]); //libero la stringa contenuta nella i-esima riga dell'array
                //realloc(toDelete->savedLines[i], sizeof(char));
                //toDelete->savedLines[i] = NULL;
            }
        }
        if(toDelete->savedLines != NULL) {
            //free(toDelete->savedLines);
        }
        tmp = toDelete->next;
        free(toDelete);
        toDelete = tmp;
    }

    return;
}



//______GESTIONE DEI COMANDI________
//__vado al nodo #begin e inizio a sostituire i nodi vecchi (che metto nella lista old)
//con nuovi nodi contenenti il nuovo testo__
void changeHandlerHead(int begin, int end, boolean saveHistory) {
    int cont = 0;
    Line *curr = list.head, *prevCurr = NULL;
    
    char *toSave[end - begin + 1];
    
    char *tmp;
    

    //ciclo fino a che mi trovo nel range di interesse
    while(++cont <= end) {
        if(cont >= begin) {
            if(saveHistory == true) {
                fgets(bufferInput, MAX_STR_LEN + 1, stdin);
            }
            //verifico esistenza nodo e se non esiste lo creo
            if(curr == NULL) {
                list.lineTotNum++;  //incremento contatore numero tot righe
                curr = malloc(sizeof(Line));
                curr->next = NULL;
                curr->prev = prevCurr;
                
                if(prevCurr != NULL)        //se non è primo nodo in assoluto
                    prevCurr->next = curr;
                else                        //se è primo nodo in assoluto
                    list.head = curr;
                //salvo nella storia le linee eliminate dall'editor
                if(saveHistory == true)     //non è una redo ma è una change nuova
                    
                    toSave[cont-begin] = NULL;
                else {      //è la redo di una change
                    
                    tmp = listH.curr->savedLines[cont-begin];
                    listH.curr->savedLines[cont-begin] = NULL;
                    
                    curr->text = tmp;
                }
                list.tail = curr;
            } else {
                //salvo nella storia le linee eliminate dall'editor
                if(saveHistory == true)
                    
                    toSave[cont-begin] = curr->text;
                else {
                    
                    tmp = listH.curr->savedLines[cont - begin];
                    listH.curr->savedLines[cont - begin] = curr->text;
                    curr->text = tmp;
                }
            }
            if(saveHistory == true) {
                //cambio il contenuto del nodo
                //rimuovo il carattere a capo '\n'
                curr->text = crop(bufferInput);
                char *pos = 0;
                if ((pos = strchr(curr->text, '\n')) != NULL)
                    *pos = '\0';
            }

        }
        prevCurr = curr;
        curr = curr->next;
    }
    //aggiungo alla storia il comando con l'array di puntatori alle stringe eliminate
    if(saveHistory == true)
        addToHistory(begin, end, 0, 'c', toSave);
    if(curr == NULL)
        list.tail = prevCurr;

    return;
};

void changeHandlerTail(int begin, int end, boolean saveHistory) {
    int cont = list.lineTotNum;
    Line *curr = list.tail, *prevCurr = NULL;
    
    char *toSave[end - begin + 1];
    
    int inputLen;
    //il primo nodo da scrivere va creato (stiamo appendendo)
    if(begin == cont + 1) {
        prevCurr = curr;
        curr = curr->next;
        cont ++;
    }
    //porto curr alla prima cella da modificare
    while(cont > begin) {
        curr = curr->prev;
        cont--;
    }
    //ciclo fino a che mi trovo nel range di interesse
    while(cont <= end) {

        if(saveHistory == true) {                           
            fgets(bufferInput, MAX_STR_LEN + 1, stdin);
            inputLen = strlen(bufferInput + 1);
        } else {
            inputLen = strlen(listH.curr->savedLines[cont - begin]+1);
        }

        //verifico esistenza nodo e se non esiste lo creo
        if(curr == NULL) {
            list.lineTotNum++;  //incremento contatore numero tot righe
            curr = malloc(sizeof(Line));
            curr->next = NULL;
            curr->prev = prevCurr;
            
            //se non è primo nodo in assoluto
            prevCurr->next = curr;
            //salvo nella storia le linee eliminate dall'editor
            if(saveHistory == true)
               
                toSave[cont-begin] = NULL;
            list.tail = curr;
        } else {
            //salvo nella storia le linee eliminate dall'editor
            if(saveHistory == true)

                
                toSave[cont-begin] = curr->text;
        }
        //cambio il contenuto del nodo
        
        //rimuovo il carattere a capo '\n'
       
        curr->text = crop(bufferInput);

        char *pos = 0;
        if ((pos=strchr(curr->text, '\n')) != NULL)
            *pos = '\0';
        prevCurr = curr;
        curr = curr->next;
        cont ++;
    }
    //aggiungo alla storia il comando con l'array di puntatori alle stringe eliminate
    if(saveHistory == true)
        addToHistory(begin, end, 0, 'c', toSave);
    if(curr == NULL)
        list.tail = prevCurr;
    return;
}

void deleteHandler(int begin, int end, boolean saveHistory) {
    int cont = 0;
    Line *curr = list.head;
    Line *p1 =NULL, *p2 = NULL; //p1 è il l'ultimo nodo prima del blocco dei nodi cancellati, p2 è il primo nodo dopo i nodi cancellati
    Line *temp = NULL;          //punta a curr next per salvare il suo indirizzo prima di liberare curr
    int skip = 0;

    int stop = (list.lineTotNum < end ? list.lineTotNum : end);   //è il minimo tra totLineNum e end, serve a non creare sforare
    //char toSave[end - begin +1][MAX_STR_LEN+1];
    char *toSave[end - begin + 1];
    //CONTROLLO SE LA LISTA è VUOTA, se lo è non lavoro
    if(list.head == NULL || begin > list.lineTotNum) {      //casi in cui la delete non ha effetto
        skip = 1;
    }
    while(curr != NULL && ++cont <= stop && skip == 0) {
        if(cont == begin -1)
            p1 = curr;
        if(cont >= begin) {
            list.lineTotNum--;  //decremento contatore numero tot righe
            temp = curr->next; //salvo valore curr->next
            //salvo nella storia le linee eliminate dall'editor
            if(saveHistory == true)
                
                toSave[cont-begin] = curr->text;
            
            free(curr);
            curr = temp;
        } else {
            curr = curr->next;
        }
    }
    //aggiungo alla storia il comando con l'array di puntatori alle stringe eliminate
    if(saveHistory == true)
        addToHistory(begin, stop, skip, 'd', toSave);
    if(skip == 1) {
        return;
    }
    p2 = curr;
    if(p1 == NULL)
        list.head = p2;
    else
        p1->next = p2;

    if(p2 == NULL)
        list.tail = p1;
    else
        p2->prev = p1;

    return;
}



void deleteHandlerTail(int begin, int end, boolean saveHistory) {
    int cont = list.lineTotNum;
    Line *curr = list.tail;
    Line *p1 =NULL, *p2 = NULL; //p1 è il primo nodo dopo il blocco dei nodi cancellati, p2 è l'ultimo nodo prima dei nodi cancellati
    Line *temp = NULL;          //punta a curr next per salvare il suo indirizzo prima di liberare curr
    int skip = 0;

    int stop = (list.lineTotNum < end ? list.lineTotNum : end);   //è il minimo tra totLineNum e end, serve a non creare sforare
    //char toSave[end - begin +1][MAX_STR_LEN+1];
    char *toSave[end - begin +1];
    //CONTROLLO SE LA LISTA è VUOTA, se lo è non lavoro
    if(list.head == NULL || begin > list.lineTotNum) {      //casi in cui la delete non ha effetto
        skip = 1;
    }

    while(curr!=NULL && --cont >= begin && skip == 0) {
        if(cont == stop)
            p1 = curr->next;    //salvo primo nodo dopo quelli eliminati
        if(cont <= stop) {
            list.lineTotNum--;  //decremento contatore numero tot righe
            temp = curr->prev;  //salvo valore curr prev, cioè il precedente a quello che sto per eliminare
            //salvo nella storia le linee eliminate dall'editor
            if(saveHistory == true)
                
                toSave[cont-begin] = curr->text;
            
            free(curr);
            curr = temp;
        } else {
            curr = curr->prev;
        }
    }

    //aggiungo alla storia il comando con l'array di puntatori alle stringe eliminate
    if(saveHistory == true)
        addToHistory(begin, stop, skip, 'd', toSave);
    if(skip == 1) {
        return;
    }                                                       
    p2 = curr; //nodo prima di quelli eliminati
    if(p2 == NULL)      //ho cancellato fino dalla testa
        list.head = p1;
    else
        p2->next = p1;

    if(p1 == NULL)          //ho cancellato fino alla coda
        list.tail = p2;
    else
        p1->prev = p2;
    return;

}

void undoHandler(int actionsNum) {
    int cont = 0;
    Line *pBegin = NULL;       // pBegin è il nodo da modificare (per change) e il nodo successivo a cui appendere il nodo da ripristinare (per delete)
    while(actionsNum-- > 0 && listH.curr != NULL) {
        if(listH.curr->skip == 0) {
            if (list.lineTotNum == 0 || listH.curr->begin < list.lineTotNum / 2) {      //era al contrario il <
                //se la linea da inserire o cambiare è nella prima metà della lista allora inizio a carcare dalla testa
                pBegin = list.head;
                cont = 1;
                while (cont < listH.curr->begin) {      //mi porto al nodo numero begin della lista dell'editor
                    pBegin = pBegin->next;
                    cont++;
                }
            } else {
                //altrimenti inizio a carcare dalla coda
                pBegin = list.tail;
                cont = list.lineTotNum;
                while (cont > listH.curr->begin) {
                    pBegin = pBegin->prev;
                    cont--;
                }
            }
        }
        switch(listH.curr->commandType) {
            case 'c': {
                Line *prev = NULL, *tmp = NULL;
                int inputLen;
                boolean first = false;
                int i;
                for (i = 0; i < listH.curr->end - listH.curr->begin + 1; i++) {
                    if (listH.curr->savedLines[i] == NULL) {      //se la riga è stata creata con la change, salvo la riga e elimino il nodo
                        if(first == false) {
                            first = true;
                            prev = pBegin->prev;
                            if(prev == NULL) {                //caso in cui pBegin è il primo nodo della lista
                                list.head = NULL;
                                list.tail = NULL;
                            } else {                          //caso in cui prev diventa l'ultimo nodo
                                prev->next = NULL;
                                list.tail = prev;
                            }
                        }
                        list.lineTotNum--;
                        //salvo il testo della riga che elimino per poi copiarlo se faccio redo
                        inputLen = strlen(pBegin->text) + 1;
                        
                        listH.curr->savedLines[i] = pBegin->text;
                        
                        tmp = pBegin;
                        pBegin = pBegin->next;
                        free(tmp);
                    } else {        //se la riga da ripristinare non è stata creata con la change in questione, scambio semplicemente i puntatori alle stringhe
                       
                        char *tmp;
                        tmp = pBegin->text;
                        pBegin->text = listH.curr->savedLines[i];
                        listH.curr->savedLines[i] = tmp;

                        pBegin = pBegin->next;
                    }
                }
                listH.curr = listH.curr->prev;
            }
                break;
            case 'd':
                if(listH.curr->skip != 1) {
                    //se il nodo da inserire non è in testa allora vado indietro di un nodo
                    if (pBegin != NULL && listH.curr->begin <= list.lineTotNum)
                        pBegin = pBegin->prev;
                    //creo ed inserisco i nodi da ripristinare
                    int i;
                    for (i = 0; i < listH.curr->end - listH.curr->begin + 1; i++) {
                        int inputLen = strlen(listH.curr->savedLines[i]+1);
                        Line *restored = malloc(sizeof(Line));
                        restored->prev = pBegin;
                        restored->text = listH.curr->savedLines[i];
            
                        //se sto inserendo il nuovo nodo in testa
                        if (pBegin == NULL || list.head == NULL) {
                            restored->next = list.head;
                            list.head = restored;
                        } else {
                            restored->next = pBegin->next;
                            pBegin->next = restored;
                        }
                        if (restored->next != NULL)  //se restored non è l'ultimo nodo
                            restored->next->prev = restored;
                        else
                            list.tail = restored;
                        pBegin = restored;
                        //re-incremento il numero di linee
                        list.lineTotNum++;
                    }
                }
                listH.curr = listH.curr->prev;
                break;
        }
    }
}

void redoHandler(int actionsNum) {
    if(listH.head == NULL)              //storia vuota
        return;

    while(actionsNum-- > 0) {
        if(listH.curr == NULL)              //è stato fatto l'undo di tutti i nodi
            listH.curr = listH.head;
        else if(listH.curr->next == NULL)   //oppure non ho altri comandi su cui fare redo
            return;
        else
            listH.curr = listH.curr->next;
        switch(listH.curr->commandType) {
            case 'c':
                changeHandlerHead(listH.curr->begin, listH.curr->end, false);
                
                break;
            case 'd':
                deleteHandler(listH.curr->begin, listH.curr->end, false);
                
                break;
        }
    }
}

void printHandler(int begin, int end) {
    const char* func = __PRETTY_FUNCTION__;
    
    int cont = 0;
    Line *curr = list.head;
    if(begin == 0) {      //caso eccezionale stampa riga 0
        printf(".\n");
        begin++;
    }

    if(curr == NULL)
        cont = begin - 1;

    while(curr != NULL && ++cont <= end) {
        if(cont >= begin) {
            
            puts(curr->text);
            
        }
        curr = curr->next;
    }


    while(++cont<=end) {       //se nella riga non è presente carattere stampo "."          //cont non parte da begin sono se curr è diverso da null per via delle condizioni del ciclo sopra
       
        fputc_unlocked('.', stdout);
        fputc_unlocked('\n', stdout);
    }
    return;
}


//________MAIN_______
int main() {
    Command currentCommand;

   

    do {
        
        fgets(bufferCommand, MAX_COM_LEN + 1, stdin);
        if (strcmp(bufferCommand, ".\n") !=0) {       //verifico se input comando è diverso dal . che viene messo alla fine degli input
            currentCommand = commandParser(bufferCommand, currentCommand);
            int virtualCurr = listH.curr != NULL ? listH.curr->number : 0;
            int listHsize = listH.tail != NULL ? listH.tail->number : 0;

            while (currentCommand.type == 'u' || currentCommand.type == 'r') {
                switch(currentCommand.type) {
                    case 'u': {
                        if(virtualCurr - currentCommand.actionsNum < 0) virtualCurr = 0;
                        else virtualCurr -= currentCommand.actionsNum;
                        break;
                    }
                    case 'r': {
                        if(currentCommand.actionsNum < listHsize - virtualCurr) virtualCurr += currentCommand.actionsNum;
                        else virtualCurr = listHsize;
                        break;
                    }
                    default:
                        break;
                }

                fgets(bufferCommand, MAX_COM_LEN + 1, stdin);
                if (strcmp(bufferCommand, ".\n") !=0) {       //verifico se input comando è diverso dal . che viene messo alla fine degli input
                    currentCommand = commandParser(bufferCommand, currentCommand);
                }
            };
            int currNumber = listH.curr != NULL ? listH.curr->number : 0;
            //chiamo la undo o redo di virtualCurr steps prima di eseguire il comando diverso
            if(virtualCurr < currNumber)
                undoHandler(currNumber - virtualCurr);
            else if(virtualCurr > currNumber)
                redoHandler(virtualCurr - currNumber);
            //se currNumber - virtualCurr == 0 non ho operazioni da eseguire perchè il numero di undo è uguale al numero di redo

            switch (currentCommand.type) {
                case 'c':
                    if (list.lineTotNum <= 1 || currentCommand.begin <= list.lineTotNum / 2)
                        changeHandlerHead(currentCommand.begin, currentCommand.end, true);
                    else
                        changeHandlerTail(currentCommand.begin, currentCommand.end, true);
                    
                    break;
                case 'd':
                    if (list.lineTotNum <= 1 || currentCommand.begin <= list.lineTotNum / 2)
                        deleteHandler(currentCommand.begin, currentCommand.end, true);
                    else
                        deleteHandlerTail(currentCommand.begin, currentCommand.end, true);
                    
                    break;
                case 'p':
                    printHandler(currentCommand.begin, currentCommand.end);
                    
                    break;
                case 'u':
                    undoHandler(currentCommand.actionsNum);
                    
                    break;
                case 's':
                    
                    break;
                case 'r':
                    redoHandler(currentCommand.actionsNum);
                    
                    break;
                case 'q':
                    
                    break;
                default:
                    break;
            }
        }
    } while (currentCommand.type != 'q');


    //fclose(fp);
    return 0;
}
