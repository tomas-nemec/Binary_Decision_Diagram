#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>




/***     STRUKTURY       ***/
typedef struct bf{
    char *string;
}BF;

typedef struct bddnode{
    struct bf *boolf;

    struct bddnode *zero_child;
    struct bddnode *one_child;

}BDDnode;

typedef struct bdd{
    int pocetPremennych;
    int pocetUzlov;
    struct bddnode *root;
}BDD;

typedef struct  linkListBDD{

    struct bddnode *prvok;
    struct linkListBDD *next;

}LLbdd;



/***     GLOBALNA PREMENNA      ***/
LLbdd *zoznam = NULL;       //globalna premenna pre spajany zoznam v ktorom sa nachadzaju pointre na prvotne prvky



/***    POMOCNE FUNKCIE       ***/



int vectorLen(int pocPrem)      //na zaklade poctu premennych mi vypocita dlzku vektora
{
    int vecLen = 1;
    for(int i=0;i<pocPrem;i++)
    {
        vecLen *= 2;
    }
    return vecLen;
}

char *vectorGenerator(int velkost)      //tu si uz generujem nahodny vektor pre vstup
{
    char* vector;
    char a[] = "01";
    int size = velkost + 1;

    vector = (char*)malloc(sizeof(char) * size);
    for(int i = 0;i<velkost;i++)
    {
        int b = rand() % 2;
        vector[i] = a[b];
    }
    vector[velkost] = '\0';

    return vector;
}

int nodesNum(char* vector)      //funckia pre vypocet poctu nodes v BDD bez redukcie
{
    int lenght = strlen(vector);
    int pocetNodes = 1;
    while(1)
    {
        lenght = (lenght / 2);
        if(lenght < 1)
        {
            break;
        }
        pocetNodes += 2*lenght;
    }

    return pocetNodes;

}

int premenneNum(char* vector){      //z dlzky vektora vypocita pocet premennych
    int dlzka = strlen(vector);
    int pocPrem = 0;
    while(dlzka > 1){
        dlzka /= 2;
        pocPrem++;
    }
    return pocPrem;
}

char *generujUseVektor(int cislo, int pocetPrem)        //cislo ktore davam do binarky, pocetPrem bude ze aku dlzku bude mat to cislo
{
    /*** funckia kde si cislo prehodim do binarnej sustavy, cislo v binarnej sustave
     bude vlastne cesta v BDD*/
    char *final = (char*)malloc(sizeof(char) * (pocetPrem+1));
    for(int i = 0;i<pocetPrem;i++)
    {
        final[i] = '0';
    }
    final[pocetPrem] = '\0';

    int a;
    int i = 0;
    while(cislo  != 0)      //vypocet cisla do binarky
    {
        a = cislo % 2;
        if(a == 1){
            final[i++] = '1';
        }
        else{
            final[i++] = '0';
        }
        cislo = cislo / 2;
    }

    char *reverse = (char*)malloc(sizeof(char) * (pocetPrem+1));
    for(int j = 0; j < pocetPrem;j++)
    {
        reverse[j] = final[pocetPrem-j-1];
    }
    reverse[pocetPrem] = '\0';

    free(final);
    final = NULL;


    return reverse;

}

//  FREE LL
BDDnode *newNode(BF *a)
{
    BDDnode *new = (BDDnode*)malloc(sizeof(BDDnode));
    new->boolf = a;
    new->one_child = NULL;
    new->zero_child = NULL;

    return new;
}

LLbdd  *newLLNode(BDDnode *a)       //tu vsetko spravne
{
    LLbdd *new = (LLbdd*)malloc(sizeof(LLbdd));
    new->prvok = a;
    new->next = NULL;

    return new;
}


void uvolniZoznam()     //uvolni zoznam
{
    LLbdd *temp = NULL;
    while(zoznam != NULL){
        temp = zoznam;
        zoznam = zoznam->next;
        free(temp->prvok->boolf->string);       //uvolni alokovane miesto
        temp->prvok->boolf->string = NULL;      //nastav pointer na NULL
        free(temp->prvok->boolf);
        temp->prvok->boolf = NULL;
        temp->prvok->zero_child = NULL;
        temp->prvok->one_child = NULL;
        temp->prvok = NULL;
        temp->next = NULL;
        free(temp);
        temp = NULL;
    }
}

//  vypisy
void vypis(BDDnode *root)
{
    if(root == NULL)    //       root->one_child == NULL && root->zero_child == NULL
    {
        return;
    }

    vypis(root->zero_child);
    printf("\n%s", root->boolf->string);

    vypis(root->one_child);
}

void vypisLL(LLbdd *a){
    printf("\n\n\tLINKED LIST:\n");
    while(a != NULL){
        printf("\t(%s)", a->prvok->boolf->string);
        //vypis(a->prvok);
        a = a->next;
    }
}

void insertLL(BDDnode *a){
    LLbdd *akt = zoznam;
    LLbdd *temp = NULL;
    while(akt->next != NULL){
        akt = akt->next;
    }
    temp = newLLNode(a);
    akt->next = temp;
}





void freeTree(BDDnode *root)
{
    if(root == NULL) return ;

    freeTree(root->zero_child);
    freeTree(root->one_child);
    free(root->boolf->string);
    root->boolf->string = NULL;
    root->boolf = NULL;
    root->one_child = NULL;
    root->zero_child = NULL;;
    free(root);
    root = NULL;

}


////////////////////////////////////////////////////////////////

BDDnode *createBDDtree_recur(BF *bfunkcia)      //rekurzivna funkcia na vytvorenie uplneho BDD
{
    if(strlen(bfunkcia->string) == 1 )      //ak uz je vektor s dlzkou 1 znak vytvorim nodes a seknem s rekurziou a zacnem z nej vychadzat
    {
        BDDnode *list = newNode(bfunkcia);
        return list;
    }

    BDDnode *prvok = newNode(bfunkcia);     //vytvorim novy node

    int stred = strlen(bfunkcia->string)/2;     //zistim si stred -> pre rozdelenie vektoru na polovicu

    //pomocne pointre pre uschovanie lavej a pravej strany stringu
    char *left = malloc((stred+1)*sizeof(char));
    char *right = malloc((stred+1)*sizeof(char));

    BF *zero = malloc(sizeof(BF));
    BF *one = malloc(sizeof(BF));


    //rozdel na polovice
    memcpy(left, bfunkcia->string, stred);  //prekopiruje do left prvu polovicu stringu
    *(left+stred) = '\0';
    memcpy(right, bfunkcia->string+stred, stred);   //prekopiruje do right druhu polovicu stringu
    *(right+stred) = '\0';

    zero->string = strdup(left);   //skopirujem uz do struktury BF aby som mohol pouzit ako argument funkcie
    one->string = strdup(right);
    //uvolnenie pomocnych pointerov
    free(left);
    free(right);
    left = NULL;
    right = NULL;


    prvok->zero_child = createBDDtree_recur(zero);  //volam funckiu nad lavou stranou
    prvok->one_child = createBDDtree_recur(one);    //potom nad pravou stranou

    return prvok;

}

BDD *BDD_create(BF *bfunkcia)       //vraciam vytvoreny strom uz
{
    BDD *headnode = (BDD*)malloc(sizeof(BDD));
    headnode->pocetUzlov = nodesNum(bfunkcia->string);
    headnode->pocetPremennych = premenneNum(bfunkcia->string);
    headnode->root = NULL;

    headnode->root = createBDDtree_recur(bfunkcia);

    return headnode;

}



void  reduceBDDtree(BDDnode *root, int* reducedNodes)
{
    if(root == NULL)    //koniec rekurzie
    {
        return;
    }

    LLbdd *temp,*akt;   //skokan na LL a alokator


    if(zoznam == NULL)      //ak je zoznam NULL, vytvorim ho a hned do neho vlozim root
    {
        temp = newLLNode(root);
        zoznam = temp;
    }


    if(root->zero_child != NULL && root->one_child != NULL)     //pozriem sa ci ma aktualny root childy
    {
        akt = zoznam;
        int add1,add2;      //premenne na zistenie ci budem pridavat do zoznamu
        add1 = add2 = 1;
        int equalCHILDEN = 0;

        if(strcmp(root->one_child->boolf->string,root->zero_child->boolf->string) == 0)     //ak maju childy rovnaky vektor, pravy uvolnim (zatial necham na NULL)
        {
            if(root->zero_child != root->one_child){
                freeTree(root->one_child);
                root->one_child = NULL;
            }

            equalCHILDEN = 1;       //su rovnake - zavisi od toho dalsi krok
        }

        if(equalCHILDEN)        //ak su teda rovnake childy, budem v zozname porovnavat iba lavy child
        {
            while(akt != NULL)      //prehladavanie spajaneho zoznamu s prvotnymi nodes zo stromu
            {
                if(strcmp(akt->prvok->boolf->string,root->zero_child->boolf->string) == 0)      //ak sa tam uz node s rovnakym vektorom nachadza
                {
                    add1 = 0;       //nebudem lavy child vkladat do zoznamu prvotnych prvkov lebo tam uz je
                    *reducedNodes += 2; //kedze su duple obidva =  2 nodes
                    if(!(akt->prvok == root->zero_child)){      //kontorla ci sa jedna uz o presmerovany prvok na prvotny, ak nie tak vykonaj podmienku
                        freeTree(root->zero_child);
                        root->zero_child = akt->prvok;      //presmeruj lavy childy aby ukazoval na prvotny prvok
                        root->one_child = root->zero_child; //presmeruj pravy child aby ukazoval tam kde lavy
                    }
                    break;  //ked uz najdem duple, ukonci

                }
                akt = akt->next;
            }
            if(add1)        //ak sa node s rovnakym vektorom ako lavy child v zozname nenachadzal, pridaj ho do zoznamu
            {
                *reducedNodes += 1;         //lavy je prvotny a pravy uvolnim
                root->one_child = root->zero_child;
                insertLL(root->zero_child);
            }
        }




        else        //odlisne childy rovnaky princip ako v pripade vyssie ale pri prechadzani zoznamu porovnavam oba childy
        {
            while(akt != NULL)
            {
                if(strcmp(akt->prvok->boolf->string,root->zero_child->boolf->string) == 0)      //kontrola ci vektor == vektoru laveho childa este nie je v zozname
                {
                    add1 = 0;       //uz tam je, nebudem pridavat do zoznamu
                    *reducedNodes += 1;
                    if(!(akt->prvok == root->zero_child)){      //ak sa nasiel duplikat v zozname, porovnam ci sa jedna o prvotny prvok, ak nie tak vykonaj podmeinku
                        freeTree(root->zero_child);     //uvolni lavy child
                        root->zero_child = akt->prvok;  //presmeruj lavy child na prvotny child zo zoznamu
                    }
                }

                if(strcmp(akt->prvok->boolf->string,root->one_child->boolf->string) == 0)       //rovnako ako v pripade hore ale pre pravy child sa vykonava
                {
                    add2 = 0;
                    *reducedNodes += 1;
                    if(!(akt->prvok == root->one_child))
                    {
                        freeTree(root->one_child);
                        root->one_child = akt->prvok;
                    }
                }
                if(add2 == 0 && add1 == 0)      //ak som nasiel duplikaty oboch, ukoncim vyhladavanie
                {
                    break;
                }
                akt = akt->next;
            }

            if(add1)    //ak nenasiel duplikat, pridam prvok na konie zoznamu
            {
                insertLL(root->zero_child);
            }
            if(add2)    //ak nenasiel duplikat, pridam prvok na konie zoznamu
            {
                insertLL(root->one_child);
            }
        }


    }

    reduceBDDtree(root->zero_child,reducedNodes);   //volanie rekurzivne
    reduceBDDtree(root->one_child,reducedNodes);    //volanie rekurzivne


}

int BDD_reduce(BDD *bdd)
{
    if(bdd->root == NULL)
    {
        return -1;
    }

    int removedNodes = 0;   //premenna kde budem drzat pocet redukovanych nodes

    reduceBDDtree(bdd->root,&removedNodes);
    bdd->pocetUzlov -= removedNodes;



    return removedNodes;
}

char BDD_use(BDD *bdd, char *vstupy)
{
    BDDnode *move = bdd->root;  //tymto sa budem pohybovat v strome

    int inputL = strlen(vstupy);    //dlzka vstupu, aby som vedel kolko iterovat

    for(int i = 0; i<inputL;i++)
    {
        if(vstupy[i] == '1'){       //ak je na danom indexe znak 1 posuniem sa v strome do praveho childu
            move = move->one_child;
        }
        else if(vstupy[i] == '0'){  //ak je na danom indexe znak 0 posuniem sa v strome do lavehi childu
            move = move->zero_child;
        }
        else{           //v pripade nejakej chyby
            return 'k';
        }
    }

    //tu som uz na spodku stromu, kde su nodes s vektorom dlzky 1
    if(strcmp(move->boolf->string,"1") == 0)    //ak je vektor nodu 1, returnujem 1
    {
        return '1';
    }
    else if(strcmp(move->boolf->string,"0") == 0) //ak je vektor nodu 0, returnujem 0
    {
        return '0';
    }
    else{               //pripade nejakej chyby returnujem k
        return 'k';
    }
}


void tester(int pocetPremennych, int pocetStromov)
{
    clock_t begin1 = clock();
    double zredukovanost = 0;
    for(int i = 0;i<pocetStromov;i++)
    {
        BDD *diagram = NULL;
        BF *input = (BF*)malloc(sizeof(BF));

        input->string = strdup(vectorGenerator(vectorLen(pocetPremennych)));

        diagram = BDD_create(input);

        int a = BDD_reduce(diagram);
        float percentage = ((float)a/(float)nodesNum(diagram->root->boolf->string)) * 100 ;        //
        zredukovanost += percentage;

        /***    STROM JE SPRAVENY DOBRE ****/

        for(int i = 0; i < vectorLen(pocetPremennych);i++)
        {
            char *abc = generujUseVektor(i,pocetPremennych);

            if(BDD_use(diagram,abc) != diagram->root->boolf->string[i]){
                printf("\ni:(%d) : ERROR root:(%c)  return funckie: (%c) ",i, diagram->root->boolf->string[i],BDD_use(diagram,abc) );
                break;
            }
        }


        diagram->root = NULL;
        uvolniZoznam();
    }

    clock_t end1 = clock();
    double time_spent1 = (double)(end1 - begin1) / CLOCKS_PER_SEC;
    zredukovanost = zredukovanost / pocetStromov;
    printf("\nAverage reduction: %.2f",zredukovanost);
    printf("\nExecution time: %Lf",time_spent1);
}

int main() {

    tester(13,2000);

    return 0;
}





