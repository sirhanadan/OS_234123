//
// Created by asus on 31/03/2024.
//
#include  <unistd.h>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <sys/mman.h>

/****************************************  METADATA STRUCT  **********************************************/
struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
    /*additional - malloc_2:*/
    void* address; //the address of the data in the memory
    bool is_mmap; //mmap or sbrk - we need the distinction because they are freed differently plus reallocated differently
};

typedef struct MallocMetadata MallocMetadata; //just because i dont wanna keep adding the word struct everywhere


void metaDataInit(MallocMetadata* md_ptr, size_t size, bool is_fr, MallocMetadata* nxt, MallocMetadata* prv, void* adrs)
{
    if(md_ptr == nullptr){
        return;
    }
    md_ptr->size = size;
    md_ptr->is_free = is_fr;
    md_ptr->next = nxt;
    md_ptr->prev = prv;
    md_ptr->address = adrs;
    md_ptr->is_mmap = false;
}


/******************************************  FUNCS DECS  ************************************************/
//challenges:
void splitBlocks_ChallengeZero(MallocMetadata* md, int index, int spliN);
void* findExactSizeBlock_ChallengeZero(size_t size);
void* findNeighboringBlock_ChallengeOne(MallocMetadata* md);
//requested:
void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();


/******************************************  useful variables:  *******************************************/
#define MAX_ORDER 10
#define SIZE_128K 131072
bool firstMalloc = true;

/*
 order -    size
 0     -    128
 1     -    128*(2^1) = 265
 2     -    128*(2^2) = 512
 3     -    128*(2^3) = 1024
 4     -    128*(2^4) = 1048
 5     -    128*(2^5) = 4069
 6     -    128*(2^6) = 8192
 7     -    128*(2^7) = 16384
 8     -    128*(2^8) = 32768
 9     -    128*(2^9) = 65536
 10    -    128*(2^10) = 131072
 */
size_t blocksSizesArr[MAX_ORDER + 1] = {128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};



/******************************************  useful functions:  ******************************************/

//given a size, get the index that matches that size in the sizes list
int findSizeIndexInList(size_t size)
{
    for(int i = 0; i <= MAX_ORDER; i++)
    {
        if(blocksSizesArr[i] == size)
        {
            return i;
        }
    }
    return -1;
}


bool invalidSize(size_t size){
    if(size == 0 || size > 100000000)
    {
        return true;
    }
    return false;
}


/******************************************  CHALLENGE 0:  ***********************************************/
//the challenge is to create a list of lists, where the nodes are the MetaDatas
//the list of MetaDatas should be ordered based on the address
//the list of lists should be ordered by size - there are only 11 available sizes so we can use a simple array



class MDList {
public:
    MallocMetadata* head;
    MallocMetadata* tail;
    //size_t list_size; - deleted to save more space for the extra data - we don't want anything over 64 bytes

    //constructor
    MDList(): head(nullptr), tail(nullptr){};

    //is the list completely empty:
    bool isEmpty(){
        if(this->head == nullptr){
            return true;
        }
        return false;
    }

    bool isNull(MallocMetadata* md){
        if(md == nullptr){
            return true;
        }
        return false;
    }

    //is there exactly one element in the list:
    bool isAlmostEmpty(){
        if(this->head != nullptr){
            if(this->head == this->tail) {
                return true;
            }
        }
        return false;
    }


    //find a block which is free and matches the requested size - return the address of the block itself not the md
    void* findFreeSizedBlock(size_t requestedSize){
        MallocMetadata* tmp = this->head;
        while(!isNull(tmp))
        {
            if((tmp->is_free) && (requestedSize <= tmp->size))
            {
                tmp->is_free = false;
                return tmp->address;
            }
            tmp = tmp->next;
        }
        return nullptr;
    }


    //add a new element in order of addresses:
    void addMD(MallocMetadata* md){

        //1. if the list is empty
        if(this->isEmpty()){
            this->head = md;
            this->tail = md;
            return;
        }
        //2. if the list is not empty, look for the correct place to add the new md node:
        MallocMetadata* nxtTmp = this->head;
        MallocMetadata* prevTmp = nullptr;

        while(!isNull(nxtTmp)){ //traverse the list based on addresses
            if(md->address < nxtTmp->address){
                break;
            }
            prevTmp = nxtTmp;
            nxtTmp =nxtTmp->next;
        }

        //2.1 - the new md is the new tail:
        if(isNull(nxtTmp)){
            prevTmp->next = md;
            md->prev = prevTmp;
            this->tail = md;
            return;
        }

        //2.2 - the new md is the new head:
        if(nxtTmp == this->head){
            md->next = this->head;
            this->head = md;
            this->head = md;
            return;
        }

        //2.3 - otherwise add in mid :
        prevTmp->next = md;
        md->prev = prevTmp;
        md->next = nxtTmp;
        nxtTmp->prev = md;
        return;

    }

    //delete existing element:
    void delMD(MallocMetadata* md){
        MallocMetadata* prevTmp = nullptr;
        MallocMetadata* currTmp = this->head;


        if(isEmpty()){ //there is nothing to delete
            return;
        }

        //find the MD node:
        while(!isNull(currTmp)){ //traverse the list based on addresses
            if(md->address == currTmp->address){
                break;
            }
            prevTmp = currTmp;
            currTmp =currTmp->next;
        }

        if(isNull(currTmp)){
            //we've reached the end and not found theMD we're looking for
            return;
        }

        //1. deleting the head:
        if(currTmp == this->head){
            if(this->isAlmostEmpty()){
                this->head = nullptr;
                this->tail = nullptr;
            }
            else{
                this->head = this->head->next;
                this->head->prev = nullptr;
            }
        }else{
            //2. deleting the tail:
            if(currTmp == this->tail){
                this->tail = prevTmp;
                prevTmp->next = nullptr;
            }else{

                //3. otherwise - delete mid:
                prevTmp->next = currTmp->next;
                (currTmp->next)->prev = prevTmp;

            }
        }

        currTmp->prev = nullptr;
        currTmp->next = nullptr;
        return;

    }

    //number of free blocks:
    size_t freeBlocksNum(){
        size_t sum = 0;
        MallocMetadata* tmp = this->head;
        while(!isNull(tmp)){
            if(tmp->is_free){
                sum++;
            }
            tmp = tmp->next;
        }
        return sum;
    }

    //number of free bytes:
    size_t freeBlocksNumBytes(){

        size_t sum = 0;

        size_t size_of_mallocMetaData = sizeof(MallocMetadata); //we need this excluded
        MallocMetadata* tmp = this->head;
        while(!isNull(tmp)){
            if(tmp->is_free){
                sum = sum + (tmp->size - size_of_mallocMetaData);
            }

            tmp = tmp->next;
        }
        return sum;
    }

    //number of all blocks:
    size_t allBlocksNum(){
        size_t sum = 0;
        MallocMetadata* tmp = this->head;
        while(!isNull(tmp)){
            sum++;
            tmp = tmp->next;
        }
        return sum;
    }

    //number of all bytes:
    size_t allBlocksNumBytes(){

        size_t sum = 0;

        size_t size_of_mallocMetaData = sizeof(MallocMetadata); //we need this excluded
        MallocMetadata* tmp = this->head;
        while(!isNull(tmp)){
            sum = sum + (tmp->size - size_of_mallocMetaData);

            tmp = tmp->next;
        }
        return sum;
    }

};


/////////////////////////////////// challenge 0 data structures:
MDList mdListArr[MAX_ORDER + 1];
MDList mappedList;


/******************************************  CHALLENGE 1:  ***********************************************/


//part the block
void splitBlocks_ChallengeZero(MallocMetadata* md, int index, int spliN)
{
    size_t md_size = sizeof(MallocMetadata);
    if(spliN <= 0)
    {
        return;
    }
    mdListArr[index].delMD(md);
    for(int i=spliN; i>0; i--){
        MallocMetadata* newMD = reinterpret_cast<MallocMetadata*>(reinterpret_cast<char*>(md) + blocksSizesArr[index - 1]);
        metaDataInit(newMD, blocksSizesArr[index - 1], true, nullptr, nullptr, reinterpret_cast<void *>(reinterpret_cast<char *>(md) + blocksSizesArr[index - 1] + md_size));
        mdListArr[index - 1].addMD(newMD);
        index--;
    }

    MallocMetadata* firstMD = md;
    metaDataInit(firstMD, blocksSizesArr[index], false, nullptr, nullptr, md->address);
    mdListArr[index].addMD(firstMD);
}

void* findExactSizeBlock_ChallengeZero(size_t size){
    void* freeBlock;
    int splitNum = 0;
    size_t md_size =  sizeof(MallocMetadata);
    for(int i=0; i<=MAX_ORDER; i++)
    {
        if(blocksSizesArr[i] >= size)
        {
            freeBlock = mdListArr[i].findFreeSizedBlock(size);

            if(freeBlock != nullptr)
            {
                MallocMetadata *metaRes = reinterpret_cast<MallocMetadata*>(reinterpret_cast<char*>(freeBlock) - md_size);
                splitBlocks_ChallengeZero(metaRes, i, splitNum);
                metaRes->is_free = false;
                return (reinterpret_cast<MallocMetadata*>(reinterpret_cast<char*>(metaRes) + md_size));
            }
            splitNum++;
        }
    }
    return nullptr;
}

/******************************************  CHALLENGE 2:  ***********************************************/

void* findNeighboringBlock_ChallengeOne(MallocMetadata* md){

    unsigned long long ptrValue = reinterpret_cast<unsigned long long>(md);
    return reinterpret_cast<void*>(ptrValue ^ (md->size));
}


/*****************************************  USER FUNCS  ************************************************/

size_t _num_free_blocks(){
    int sum = 0;
    for(int i = 0; i <= MAX_ORDER; i++)
    {
        sum += mdListArr[i].freeBlocksNum();
    }
    return sum;
}

size_t _num_free_bytes(){
    size_t sum = 0;
    for(int i = 0; i <= MAX_ORDER; i++)
    {
        sum += mdListArr[i].freeBlocksNumBytes();
    }
    return sum;
}

size_t _num_allocated_blocks(){
    int blockCnt = 0;
    for(int i = 0; i <= MAX_ORDER; i++)
    {
        blockCnt += mdListArr[i].allBlocksNum();
    }

    blockCnt += mappedList.allBlocksNum();
    return blockCnt;
}


size_t _num_allocated_bytes(){
    int bytesCnt = 0;
    for(int i = 0; i < MAX_ORDER+1; i++)
    {
        bytesCnt += mdListArr[i].allBlocksNumBytes();
    }

    bytesCnt += mappedList.allBlocksNumBytes();
    return bytesCnt;
}

size_t _num_meta_data_bytes(){
    size_t md_size =  sizeof(MallocMetadata);
    return (_num_allocated_blocks() * md_size);

}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}

/****************************************  REQUESTED MALCS ***********************************************/

void* smalloc(size_t size)
{

    size_t md_size = sizeof(MallocMetadata);
    size_t tot_size = md_size + size;

    if(firstMalloc)
    {

        //1. malloc the 32 blocks of size 128KB
        firstMalloc = false;
        void* prevProgramBreak = sbrk(0);//retrieve the previous program break pointer
        unsigned long long pb = ((unsigned long long)prevProgramBreak % SIZE_128K); //to ensure alignment
        void * pdAdrs = sbrk(32 * SIZE_128K - pb + SIZE_128K); //should we add an extra SIZE_128K at the end to make sure we have enough space????
        //pdAdrs now points the previous program break

        void* res = reinterpret_cast<MallocMetadata*>(reinterpret_cast<char*>(pdAdrs) - pb + SIZE_128K);
        //res now points to the start of our blocks

        //init the lists of MDs
        for(int i=0; i<=MAX_ORDER; i++)
        {
            mdListArr[i] = MDList();
        }
        //init the MDs
        for(int i = 0; i < 32; i++)
        {
            MallocMetadata* mdNew = reinterpret_cast<MallocMetadata*>(res);
            metaDataInit(mdNew, SIZE_128K, true, nullptr, nullptr,reinterpret_cast<MallocMetadata *>(reinterpret_cast<char *>(res) + md_size));
            mdListArr[MAX_ORDER].addMD(mdNew);
            res = reinterpret_cast<char*>(res) + SIZE_128K;
        }

        mappedList = MDList();
    }

    //start of the normal malloc code:
    if(invalidSize(size))
    {
        return NULL;
    }


    //case 1: use the sbrked blocks
    if(tot_size <= SIZE_128K)
    {
        void* apprBlock = findExactSizeBlock_ChallengeZero(tot_size);
        if(apprBlock != nullptr)
        {
            return apprBlock;
        }
        return NULL;
    }

    /***************************************  CHALLENGE 3:  ********************************************/

    //case 2: mmap for large blocks
    void* res = mmap(NULL, size + md_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    MallocMetadata* mappedMD = reinterpret_cast<MallocMetadata*>(res);
    res = reinterpret_cast<MallocMetadata*>(reinterpret_cast<char*>(res) + md_size);
    metaDataInit(mappedMD, tot_size, false, nullptr, nullptr, res);
    mappedMD->is_mmap = true;
    mappedList.addMD(mappedMD);
    return mappedMD->address;


}

void* scalloc(size_t num, size_t size){

    size_t tot_size = size*num;
    if(size <= 0 || num <= 0){
        return NULL;
    }
    //if size or num are > 100000000 and neither are 0 then the mult should be > 100000000
    if(tot_size > 100000000){
        return NULL;
    }

    void* adrs = smalloc(tot_size);
    if(adrs == NULL){
        return NULL;
    }
    std::memset(adrs, 0, tot_size);
    return adrs;
}

void sfree(void* p){
    if(p == NULL){
        return;
    }

    size_t md_size = sizeof(MallocMetadata);

    //get the address of the relevant MD based on the address of the block itself
    MallocMetadata *res = reinterpret_cast<MallocMetadata*>(reinterpret_cast<char*>(p) - md_size);
    if(res->is_free){
        return;
    }
    //case 1. mapped or large blocks -> unmap :
    if(res->size > SIZE_128K || res->is_mmap)
    {
        mappedList.delMD(res);
        munmap(res, res->size);
        return;
    }

    //case 2. sbrked blocks :
    res->is_free = true;
    if(res->size == SIZE_128K){
        return;
    }
    MallocMetadata* neighMD =reinterpret_cast<MallocMetadata*>(findNeighboringBlock_ChallengeOne(res));
    //res->is_free = true;
    if(!neighMD->is_free)
    {
        return;
    }


    //combine free neighbors
    while(neighMD->is_free && res->size < SIZE_128K)
    {
        int i = findSizeIndexInList(res->size);
        mdListArr[i].delMD(res);
        mdListArr[i].delMD(neighMD);

        if(neighMD->address < res->address)
        {
            res = neighMD;
        }

        res->size = res->size * 2;
        res->is_free = true;
        mdListArr[i + 1].addMD(res);

        if(res->size == SIZE_128K)
        {
            break;
        }
        neighMD = reinterpret_cast<MallocMetadata*>(findNeighboringBlock_ChallengeOne(res));

    }
}


void* srealloc(void* oldp, size_t size){
    // srealloc will not be tested on large mapped areas

    if(size <= 0){
        return NULL;
    }

    if(oldp == nullptr){
        return smalloc(size);
    }

    size_t md_size = sizeof(MallocMetadata);
    size_t new_size = size + md_size;

    MallocMetadata *current = reinterpret_cast<MallocMetadata*>(reinterpret_cast<char*>(oldp) - md_size);

    //if the size is small enough there is no need to reallocate bc the block is already big enough
    if(current->size == new_size || (current->size >= new_size && !current->is_mmap)){
        return oldp;
    }

    // sure it is sbrked:

    void* newMD;
    MallocMetadata* neighMd = reinterpret_cast<MallocMetadata*>(findNeighboringBlock_ChallengeOne(current));
    //if the neighboring blocks are free combine them
    if(neighMd->is_free)
    {
        while(neighMd->is_free && neighMd->size < SIZE_128K)
        {
            int i = findSizeIndexInList(current->size);
            mdListArr[i].delMD(current);
            mdListArr[i].delMD(neighMd);
            current->size *= 2;
            mdListArr[i + 1].addMD(current);
            neighMd = reinterpret_cast<MallocMetadata*>(findNeighboringBlock_ChallengeOne(current));

            if(current->size >= new_size){
                newMD = current->address;
                break;
            }
        }
        //move the existing vals
        std::memmove(newMD, oldp, current->size);
    }
    else
    {
        current->is_free = true;
        newMD = smalloc(size);
        if(newMD == NULL){
            current->is_free = false;
            return NULL;
        }
        //move the existing vals
        std::memmove(newMD, oldp, current->size);
    }


    //there is a case in which we don;t have enough space in the MD lists, so we map a small area
    //in this case we should unmap it, the malloc we've used before will take care of the reallocation
    if(current->is_mmap)
    {
        munmap(current, current->size);
        mappedList.delMD(current);
    }
    return newMD;
}



