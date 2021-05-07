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

void* ku_mmu_pmem = 0;
void* ku_mmu_swapSpace = 0;
node* ku_mmu_swapHeadAddress;
node* ku_mmu_swapTailAddress;
node* ku_mmu_freeHeadAddress;
node* ku_mmu_freeTailAddress;
ku_pcb* ku_mmu_pcbHead;
ku_pcb* ku_mmu_pcbTail;
data* ku_mmu_usingPageHead;
data* ku_mmu_usingPageTail;

ku_pcb* searchPCB(char pid){
    ku_pcb* foundPCB = NULL;
    ku_pcb* tmp = (ku_pcb*)malloc(sizeof(ku_pcb));
    tmp->pid = pid;
    ku_pcb* current = ku_mmu_pcbHead;

    do{ 
        if(tmp->pid == ku_mmu_pcbHead->pid){
            tmp->pdbr = ku_mmu_pcbHead->pdbr;
            foundPCB = tmp;
            break;
        }else{
            ku_mmu_pcbHead = ku_mmu_pcbHead->next;
        }
    }while(ku_mmu_pcbHead != NULL);
    ku_mmu_pcbHead = current;

    return foundPCB;
}

void addPCB(ku_pcb* process){
    ku_pcb* addProcess = (ku_pcb*)malloc(sizeof(ku_pcb));
    addProcess->pid = process->pid;
    addProcess->pdbr = process->pdbr;
    addProcess->next = NULL;
    ku_mmu_pcbTail->next = addProcess;
    ku_mmu_pcbTail = addProcess;
}

void* popFreeList(){
    node* popNode = ku_mmu_freeHeadAddress;
    void* getPgAddress = popNode->address;
    ku_mmu_freeHeadAddress = ku_mmu_freeHeadAddress->next;
    free(popNode);

    return getPgAddress;
}
void addSwapList(void* address){
    node* addNode = (node*)malloc((sizeof(node)));
    addNode->address = address;
    addNode->next = NULL;
    ku_mmu_swapTailAddress->next = addNode;
    ku_mmu_swapTailAddress = addNode;
}

void* popSwapList(){
    node* popNode = ku_mmu_swapHeadAddress;
    void* getSwapAddress = popNode->address;
    ku_mmu_swapHeadAddress = ku_mmu_swapHeadAddress->next;
    free(popNode);

    return getSwapAddress;
}

data* popUsePage(){
    data* popData;
    if(ku_mmu_usingPageHead == NULL){
        popData = NULL;
    }else{
        popData = ku_mmu_usingPageHead;
        ku_mmu_usingPageHead = ku_mmu_usingPageHead->next;
    }
    
    return popData;
}

void addUsePage(void* pageAddress, char* bit){
    data* addAddress = (data*)malloc(sizeof(data));
    addAddress->address = pageAddress;
    addAddress->pte = bit;
    addAddress->next = NULL;
    if(ku_mmu_usingPageHead == NULL){
        ku_mmu_usingPageHead = addAddress;
        ku_mmu_usingPageTail = ku_mmu_usingPageHead;
    }else{
        ku_mmu_usingPageTail->next = addAddress;
        ku_mmu_usingPageTail = addAddress;
    }
}

node* swapList(int pageNum){
    node* current;
    node* head = (node*)malloc(sizeof(node));
    head->address = ku_mmu_swapSpace + 4;
    head->next = NULL;
    current = head;
    for(int i = 4; i < 4 * pageNum - 4; i+=4){
        node* newNode = (node*)malloc(sizeof(node));
        newNode->address = ku_mmu_swapSpace + 4 + i;
        newNode->next = NULL;
        if(i == 4 * pageNum - 8){
            ku_mmu_swapTailAddress = newNode;
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
    head->address = ku_mmu_pmem + 4;
    head->next = NULL;
    current = head;
    for(int i = 4; i < 4 * pageNum - 4; i+=4){
        node* newNode = (node*)malloc(sizeof(node));
        newNode->address = ku_mmu_pmem + 4 + i;
        newNode->next = NULL;
        if(i == 4 * pageNum - 8){
            ku_mmu_freeTailAddress = newNode;
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

    ku_mmu_pmem = malloc(pmemSize);
    ku_mmu_swapSpace = malloc(swapSize);
    memset(ku_mmu_pmem, 0, pmemSize);
    ku_mmu_freeHeadAddress = freeList(pageNum);
    ku_mmu_swapHeadAddress = swapList(swapSpaceNum);
    ku_mmu_pcbHead = (ku_pcb *)malloc(sizeof(ku_pcb));
    ku_mmu_pcbHead->pid = 0;
    ku_mmu_pcbHead->pdbr = NULL;
    ku_mmu_pcbHead->next = NULL;
    ku_mmu_pcbTail = ku_mmu_pcbHead;

    return ku_mmu_pmem;
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
        if(ku_mmu_freeHeadAddress != NULL){
            pmd = popFreeList();
            *(char*)(tmp->pdbr + pdIndex) = (((char)(pmd - ku_mmu_pmem) / 4) << 2) | 0b00000001;
        }else{
            popPage = popUsePage();
            if(popPage == NULL){
                return 0;
            }
            popSwap = popSwapList();
            memcpy(popSwap, popPage->address, 4);
            *(popPage->pte) = (((char)(popSwap - ku_mmu_swapSpace) / 4) << 1);
            pmd = popPage->address;
            *(char*)(tmp->pdbr + pdIndex) = (((char)(pmd - ku_mmu_pmem) / 4) << 2) | 0b00000001;
        }
    }else{
        pmd = ku_mmu_pmem + (pde >> 2) * 4;
    }
    pmde = *(char*)(pmd + pmdIndex);
    if(pmde == 0b00000000){
        if(ku_mmu_freeHeadAddress != NULL){
            pt = popFreeList();
            *(char*)(pmd + pmdIndex) = (((char)(pt - ku_mmu_pmem) / 4) << 2) | 0b00000001;
        }else{
            popPage = popUsePage();
            if(popPage == NULL){
                return 0;
            }
            popSwap = popSwapList();
            memcpy(popSwap, popPage->address, 4);
            *(popPage->pte) = (((char)(popSwap - ku_mmu_swapSpace) / 4) << 1);
            pt = popPage->address;
            *(char*)(pmd + pmdIndex) = (((char)(pt - ku_mmu_pmem) / 4) << 2) | 0b00000001;
        }
    }else{
        pt = ku_mmu_pmem +(pmde >> 2) * 4;
    }
    pte = *(char*)(pt + ptIndex);
    if(pte == 0b00000000){
        if(ku_mmu_freeHeadAddress != NULL){
            page = popFreeList();
            *(char*)(pt + ptIndex) = (((char)(page - ku_mmu_pmem) / 4) << 2) | 0b00000001;
            addUsePage(page, (char*)(pt + ptIndex));
        }else{
            popPage = popUsePage();
            if(popPage == NULL){
                return 0;
            }
            popSwap = popSwapList();
            memcpy(popSwap, popPage->address, 4);
            *(popPage->pte) = (((char)(popSwap - ku_mmu_swapSpace) / 4) << 1);
            page = popPage->address;
            *(char*)(pt + ptIndex) = (((char)(page - ku_mmu_pmem) / 4) << 2) | 0b00000001;
            addUsePage(page, (char*)(pt + ptIndex));
        }
    }else{
        if((pte & 0b00000001) == 0){
            swapData = *(unsigned int *)(ku_mmu_swapSpace + (((pte & 0b11111110) >> 1) * 4));
            *(unsigned int *)(ku_mmu_swapSpace + (((pte & 0b11111110) >> 1) * 4)) = 0b00000000;
            addSwapList(ku_mmu_swapSpace + (((pte & 0b11111110) >> 1) * 4));
            popPage = popUsePage();
            if(popPage == NULL){
                return 0;
            }
            pmemData =  *(unsigned int*)(popPage->address);
            popSwap = popSwapList();
            *(unsigned int*)(popSwap) = pmemData;
            *(popPage ->pte) = (((char)(popSwap - ku_mmu_swapSpace) / 4) << 1);
            *(char*)(pt + ptIndex) = (((char)(popPage->address - ku_mmu_pmem) / 4) << 2) | 0b00000001;
            addUsePage(popPage->address, (char*)(pt + ptIndex));
        }else{
            page = ku_mmu_pmem + (pte >> 2) * 4;
        }
    }
    return 0;
}