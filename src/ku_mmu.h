#include <string.h>
#include <stdio.h>

typedef struct NODE{
    void* address;
    struct NODE* next;
}node;

typedef struct KU_PCB{
    char pid;
    void *pdbr;
    struct KU_PCB* next;
}ku_pcb;

typedef struct DATA{
    void* address;
    char* pte;
    struct DATA* next;
}data;

void* pmem = 0;
void* swapSpace = 0;
node* swapHeadAddress;
node* swapTailAddress;
node* freeHeadAddress;
node* freeTailAddress;
ku_pcb* pcbHead;
ku_pcb* pcbTail;
data* usingPageHead;
data* usingPageTail;

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
void addSwapList(void* address){
    node* addNode = (node*)malloc((sizeof(node)));
    addNode->address = address;
    addNode->next = NULL;
    swapTailAddress->next = addNode;
    swapTailAddress = addNode;
}

void* popSwapList(){
    node* popNode = swapHeadAddress;
    void* getSwapAddress = popNode->address;
    swapHeadAddress = swapHeadAddress->next;
    free(popNode);

    return getSwapAddress;
}

data* popUsePage(){
    data* popData;
    printf("%p\n", usingPageHead->address);
    if(usingPageHead->address == NULL){
        popData = NULL;
    }else{
        popData = usingPageHead;
        usingPageHead = usingPageHead->next;
    }
    
    return popData;
}

void addUsePage(void* pageAddress, char* bit){
    data* addAddress = (data*)malloc(sizeof(data));
    addAddress->address = pageAddress;
    addAddress->pte = bit;
    addAddress->next = NULL;
    if(usingPageHead == NULL){
        usingPageHead = addAddress;
        usingPageTail = usingPageHead;
    }else{
        usingPageTail->next = addAddress;
        usingPageTail = addAddress;
    }
}

node* swapList(int pageNum){
    node* current;
    node* head = (node*)malloc(sizeof(node));
    head->address = swapSpace + 4;
    head->next = NULL;
    current = head;
    for(int i = 4; i < 4 * pageNum - 4; i+=4){
        node* newNode = (node*)malloc(sizeof(node));
        newNode->address = swapSpace + 4 + i;
        newNode->next = NULL;
        if(i == 4 * pageNum - 8){
            swapTailAddress = newNode;
        }
        head->next = newNode;
        head = newNode;
    }
    head = current;

    return head;
}

node* freeList(int pageNum){
    node* current;
    node* head = (node*)malloc(sizeof(node));
    head->address = pmem + 4;
    head->next = NULL;
    current = head;
    for(int i = 4; i < 4 * pageNum - 4; i+=4){
        node* newNode = (node*)malloc(sizeof(node));
        newNode->address = pmem + 4 + i;
        newNode->next = NULL;
        if(i == 4 * pageNum - 8){
            freeTailAddress = newNode;
        }
        head->next = newNode;
        head = newNode;
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
    swapSpace = malloc(swapSize);
    memset(pmem, 0, pmemSize);
    freeHeadAddress = freeList(pageNum);
    swapHeadAddress = swapList(swapSpaceNum);
    pcbHead = (ku_pcb *)malloc(sizeof(ku_pcb));
    pcbHead->pid = 0;
    pcbHead->pdbr = NULL;
    pcbHead->next = NULL;
    pcbTail = pcbHead;

    return pmem;
}

int ku_run_proc(char pid, void** ku_cr3){
    ku_pcb* tmp = (ku_pcb*)malloc(sizeof(ku_pcb));
    ku_pcb* check = searchPCB(pid);
    if(check == NULL){
        tmp->pid = pid;
        tmp->pdbr = popFreeList();
        tmp->next = NULL;
        addPCB(tmp);
        ku_run_proc(pid, &(*ku_cr3));
    }else{
        *ku_cr3 = check->pdbr;
    }
    return 0;
}

int ku_page_fault(char pid, char va){
    ku_pcb* tmp = searchPCB(pid);
    char pdIndex = (va & 0b11000000) >> 6;
    char pmdIndex = (va & 0b00110000) >> 4;
    char ptIndex = (va & 0b00001100) >> 2;
    char pgIndex = va & 0b00000011;
    void* pmd;
    void* pt;
    void* page;
    void* swapOutAddress;
    char pde;
    char pmde;
    char pte;
    data* popPage;
    void* popSwap;
    unsigned int swapData;
    unsigned int pmemData;

    pde = *(char*)(tmp->pdbr + pdIndex);
    if(pde == 0b00000000){
        if(freeHeadAddress != NULL){
            pmd = popFreeList();
            *(char*)(tmp->pdbr + pdIndex) = (((char)(pmd - pmem) / 4) << 2) | 0b00000001;
        }else{
            printf("10\n");
            popPage = popUsePage();
            printf("11\n");
            popSwap = popSwapList();
            memcpy(popSwap, popPage->address, 4);
            *(popPage->pte) = (((char)(popSwap - swapSpace) / 4) << 1);
            pmd = popPage->address;
            *(char*)(tmp->pdbr + pdIndex) = (((char)(pmd - pmem) / 4) << 2) | 0b00000001;
        }
    }else{
        pmd = pmem + (pde >> 2) * 4;
    }
    pmde = *(char*)(pmd + pmdIndex);
    if(pmde == 0b00000000){
        if(freeHeadAddress != NULL){
            pt = popFreeList();
            *(char*)(pmd + pmdIndex) = (((char)(pt - pmem) / 4) << 2) | 0b00000001;
        }else{
            printf("12\n");
            popPage = popUsePage();
            printf("13\n");
            popSwap = popSwapList();
            memcpy(popSwap, popPage->address, 4);
            *(popPage->pte) = (((char)(popSwap - swapSpace) / 4) << 1);
            pt = popPage->address;
            *(char*)(pmd + pmdIndex) = (((char)(pt - pmem) / 4) << 2) | 0b00000001;
        }
    }else{
        pt = pmem +(pmde >> 2) * 4;
    }
    pte = *(char*)(pt + ptIndex);
    if(pte == 0b00000000){
        if(freeHeadAddress != NULL){
            page = popFreeList();
            *(char*)(pt + ptIndex) = (((char)(page - pmem) / 4) << 2) | 0b00000001;
            printf("1\n");
            addUsePage(page, (char*)(pt + ptIndex));
            printf("2\n");
        }else{
            printf("18\n");
            popPage = popUsePage();
            printf("17\n");
            if(popPage == NULL){
                return 0;
            }
            popSwap = popSwapList();
            memcpy(popSwap, popPage->address, 4);
            *(popPage->pte) = (((char)(popSwap - swapSpace) / 4) << 1);
            page = popPage->address;
            *(char*)(pt + ptIndex) = (((char)(page - pmem) / 4) << 2) | 0b00000001;
            printf("3\n");
            addUsePage(page, (char*)(pt + ptIndex));
            printf("4\n");
        }
    }else{
        if((pte & 0b00000001) == 0){
            swapData = *(unsigned int *)(swapSpace + (((pte & 0b11111110) >> 1) * 4));
            *(unsigned int *)(swapSpace + (((pte & 0b11111110) >> 1) * 4)) = 0b00000000;
            addSwapList(swapSpace + (((pte & 0b11111110) >> 1) * 4));
            popPage = popUsePage();
            pmemData =  *(unsigned int*)(popPage->address);
            popSwap = popSwapList();
            *(unsigned int*)(popSwap) = pmemData;
            *(popPage ->pte) = (((char)(popSwap - swapSpace) / 4) << 1);
            *(char*)(pt + ptIndex) = (((char)(popPage->address - pmem) / 4) << 2) | 0b00000001;
            printf("5\n");
            addUsePage(popPage->address, (char*)(pt + ptIndex));
            printf("6\n");
        }else{
            page = pmem + (pte >> 2) * 4;
        }
    }
    return 0;
}