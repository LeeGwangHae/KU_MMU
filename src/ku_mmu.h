#include <string.h>
#include <stdio.h>

typedef struct NODE{
    void* address;
    struct NODE* next;
}node;

void* pmem;
void* swapSpace;
node* freeHeadAddress;
node* freeTailAddress;

void addFreeList(void* pgAddress){
    node* addNode = (node*)malloc(sizeof(node));
    addNode->address = pgAddress;
    addNode->next = NULL;
    freeTailAddress->next = addNode;
    freeTailAddress = addNode;
}

void* popFreeList(){
    node* popNode = freeHeadAddress;
    void* getPgAddress = popNode->address;
    freeHeadAddress = freeHeadAddress->next;
    free(popNode);

    return getPgAddress;
}

node* freeList(int size){
    node* current;
    node* head = (node*)malloc(sizeof(node));
    head->address = pmem;
    head->next = NULL;
    current = head;
    //printf("%p\n", head->address);
    for(int i = 4; i < 4 * size; i+=4){
        node* newNode = (node*)malloc(sizeof(node));
        newNode->address = pmem + i;
        newNode->next = NULL;
        if(i == 4 * size -4){
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
    swapSpace = malloc(swapSize);
    memset(pmem, 0, pmemSize);
    freeHeadAddress = freeList(pageNum);
    
    return pmem;
}

int ku_run_proc(){

}

int ku_page_fault(){

}