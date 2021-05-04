#include <string.h>
#include <stdio.h>

typedef struct NODE{
    void* address;
    struct NODE* next;
}node;

typedef struct KU_PTE{
    char info;
}ku_pte;

typedef struct KU_PCB{
    char pid;
    void *pdbr;
    struct KU_PCB* next;
}ku_pcb;

void* pmem = 0;
node* swapHeadAddress;
node* swapTailAddress;
node* freeHeadAddress;
node* freeTailAddress;
ku_pcb* pcbHead;
ku_pcb* pcbTail;

ku_pcb* searchPCB(char pid){
    ku_pcb* foundPCB = NULL;
    ku_pcb* tmp = (ku_pcb*)malloc(sizeof(ku_pcb));
    tmp->pid = pid;
    ku_pcb* current = pcbHead;

    do{ 
        if(tmp->pid == pcbHead->pid){
            tmp->pdbr = pcbHead->pdbr;
            foundPCB = tmp;
            break;
        }else{
            pcbHead = pcbHead->next;
        }
    }while(pcbHead != NULL);
    pcbHead = current;

    return foundPCB;
}

void addPCB(ku_pcb* process){
    ku_pcb* addProcess = (ku_pcb*)malloc(sizeof(ku_pcb));
    // printf("4\n");
    addProcess->pid = process->pid;
    addProcess->pdbr = process->pdbr;
    addProcess->next = NULL;
    pcbTail->next = addProcess;
    pcbTail = addProcess;
}

void* popFreeList(){
    node* popNode = freeHeadAddress;
    void* getPgAddress = popNode->address;
    freeHeadAddress = freeHeadAddress->next;
    free(popNode);

    return getPgAddress;
}

void* popSwapList(){
    node* popNode = swapHeadAddress;
    void* getSwapAddress = popNode->address;
    freeHeadAddress = freeHeadAddress->next;
    //free(popNode);

    return getSwapAddress;
}

node* swapList(int pageNum){
    node* current;
    node* head = (node*)malloc(sizeof(node));
    head->address = swapHeadAddress;
    head->next = NULL;
    current = head;
    //printf("%p\n", head->address);
    for(int i = 4; i < 4 * pageNum; i+=4){
        node* newNode = (node*)malloc(sizeof(node));
        newNode->address = NULL;
        newNode->next = NULL;
        if(i == 4 * pageNum -4){
            swapTailAddress = newNode;
        }
        head->next = newNode;
        head = newNode;
    }
}

node* freeList(int pageNum){
    node* current;
    node* head = (node*)malloc(sizeof(node));
    head->address = pmem;
    head->next = NULL;
    current = head;
    //printf("%p\n", head->address);
    for(int i = 4; i < 4 * pageNum; i+=4){
        node* newNode = (node*)malloc(sizeof(node));
        newNode->address = pmem + i;
        newNode->next = NULL;
        if(i == 4 * pageNum -4){
            freeTailAddress = newNode;
        }
        head->next = newNode;
        head = newNode;
        //printf("%p\n", head->address);
    }
    head = current;

    return head;
}

void* ku_mmu_init(unsigned int pmemSize, unsigned int swapSize){
    int pageSize = 4;
    int pageNum;
    int swapSpaceNum;
    
    pageNum = pmemSize / pageSize;
    swapSpaceNum = swapSize / pageSize;

    pmem = malloc(pmemSize);
    swapHeadAddress = malloc(swapSize);
    memset(pmem, 0, pmemSize);
    freeHeadAddress = freeList(pageNum);
    pcbHead = (ku_pcb *)malloc(sizeof(ku_pcb));
    pcbHead->pid = 0;
    pcbHead->pdbr = NULL;
    pcbHead->next = NULL;
    pcbTail = pcbHead;

    return pmem;
}

int ku_run_proc(char pid, void** ku_cr3){
    // printf("1\n");
    ku_pcb* tmp = (ku_pcb*)malloc(sizeof(ku_pcb));
    ku_pcb* check = searchPCB(pid);
    // printf("check: %p\n", tmp);
    // printf("tmp: %p\n", tmp);
    if(check == NULL){
        // printf("before : %p\n", pcbHead);
        // printf("before : %p\n", pcbTail);
        tmp->pid = pid;
        tmp->pdbr = popFreeList();
        tmp->next = NULL;
        // printf("%d\n", tmp->pid);
        // printf("%p\n", tmp->pdbr);
        // printf("%p\n", tmp->next);
        addPCB(tmp);
        // printf("after : %p\n", pcbHead);
        // printf("after : %p\n", pcbTail);
        // printf("3\n");
        ku_run_proc(pid, &(*ku_cr3));
    }else{
        // printf("5\n");
        // printf("%p\n", check->pdbr);
        *ku_cr3 = check->pdbr;
    }
    
    return 0;
}

int ku_page_fault(){

}