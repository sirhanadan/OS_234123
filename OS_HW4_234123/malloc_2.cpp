//
// created by el_DuDu on 30.3.24
// happy easter {}

#include  <unistd.h>
#include <stdio.h>
#include <string.h>

size_t num_of_free_blocks = 0;
size_t num_of_free_bytes = 0 ;
size_t num_of_allocated_blocks = 0;
size_t num_of_allocated_bytes = 0;
size_t num_of_meta_data_bytes = 0;

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};


MallocMetadata* globalPointersList = NULL;

void* smalloc(size_t size)
{
    if(size == 0)
    {
        return NULL;
    }
    if(size > 100000000)
    {
        return NULL;
    }
    MallocMetadata* tempptr = globalPointersList;


        // if there is enough size in the free list and the list is not empty 
        while (tempptr != NULL) {
            if (tempptr->size >= size && tempptr->is_free == true) {
                num_of_free_blocks--;
                num_of_free_bytes -= tempptr->size;
                tempptr->is_free = false;
                tempptr++;
                return (void *) tempptr;
            }
            tempptr = tempptr->next;
        }

        /// if there is no enough space
    void* newPointer= sbrk(size + sizeof(MallocMetadata));
    if(newPointer == (void*)(-1))
    {
        return NULL;
    }
    num_of_meta_data_bytes =num_of_meta_data_bytes+ sizeof(MallocMetadata);
 
    num_of_allocated_bytes=num_of_allocated_bytes+ size;
       num_of_allocated_blocks++;
//converting our block to metadata and updating the thing
    MallocMetadata* newBlock =  (MallocMetadata*) newPointer;
    newBlock->is_free = false;
      newBlock->next = NULL;
    newBlock->size =size;
  
  
    /// if the list is empty
    if(globalPointersList == NULL)
    {
        globalPointersList = newBlock;
        newBlock->prev = NULL;
        newBlock->next = NULL;

    }
    else
    {
        tempptr = globalPointersList;
        while (tempptr->next != NULL)
        {
            tempptr = tempptr->next;
        }
        newBlock->prev = tempptr;
        tempptr->next = newBlock;
        newBlock->next = NULL;
    }


    return (void*)(++newBlock);

}



void* scalloc(size_t num, size_t size)
{
    if(size == 0 || num == 0)
    {
        return NULL;
    }
    if(size*num > 100000000)
    {
        return NULL;
    }
    MallocMetadata* tempptr = globalPointersList;

            while(tempptr != NULL)
        {
            if(tempptr->size >= size*num && tempptr->is_free == true)
            {
                num_of_free_blocks--;
                num_of_free_bytes -= tempptr->size;
                tempptr->is_free = false;
                tempptr++;
                memset(tempptr, 0 ,size*num );
                return (void*)tempptr;
            }
            tempptr = tempptr->next;
         }


    void* ptr= sbrk(size*num + sizeof(MallocMetadata));
    if(ptr == (void*)(-1))
    {
        return NULL;
    }
    num_of_allocated_blocks++;
    num_of_allocated_bytes =num_of_allocated_bytes+ size*num;
    num_of_meta_data_bytes = num_of_meta_data_bytes+ sizeof(MallocMetadata);

    MallocMetadata* newBlock =  (MallocMetadata*) ptr;
    newBlock->size =size*num;
    newBlock->is_free = false;
    if(globalPointersList == NULL)
    {
        globalPointersList = newBlock;
        newBlock->prev = NULL;
        newBlock->next = NULL;

    }
    else
    {
        tempptr = globalPointersList;
        while (tempptr->next != NULL)
        {
            tempptr = tempptr->next;
        }
        newBlock->prev = tempptr;
        tempptr->next = newBlock;
        newBlock->next = NULL;
    }

    ++newBlock;
    memset(newBlock , 0 , num*size);
    return (void*)(newBlock);
}

void sfree(void* p)
{

    if(p == NULL)
    {
        return;
    }
    MallocMetadata* meantTOfREE = ((MallocMetadata*)p - 1);
    if(meantTOfREE->is_free == true)
    {
        return;
    }
    meantTOfREE->is_free = true;
    num_of_free_blocks++;
    num_of_free_bytes =num_of_free_bytes+meantTOfREE->size;

}


void* srealloc(void* oldp, size_t size)
{
    if( oldp == NULL)
    {
        return smalloc(size);
    }
    if(size == 0)
    {
        return NULL;
    }
    if(size > 100000000)
    {
        return NULL;
    }

    MallocMetadata* reallocPointer = ((MallocMetadata*)oldp - 1);
    if(size <= reallocPointer->size)
    {
        return oldp;
    }

    MallocMetadata* tempptr = globalPointersList;

        while (tempptr != NULL) {
            if (tempptr->size >= size && tempptr->is_free == true) {
                num_of_free_blocks--;
                num_of_free_bytes -= tempptr->size;
                tempptr->is_free = false;
                tempptr++;
                memcpy(tempptr, oldp, size);
                sfree(oldp);
                return (void *) tempptr;
            }
            tempptr = tempptr->next;
        }


    void* ptr= sbrk(size + sizeof(MallocMetadata));
    if(ptr == (void*)(-1))
    {
        return NULL;
    }
    num_of_allocated_blocks++;
    num_of_allocated_bytes = num_of_allocated_bytes+size;
    num_of_meta_data_bytes =num_of_meta_data_bytes+ sizeof(MallocMetadata);

    MallocMetadata* newBlock =  (MallocMetadata*) ptr;
    newBlock->size =size;
    newBlock->is_free = false;

    if(globalPointersList == NULL)
    {
        globalPointersList = newBlock;
        newBlock->prev = NULL;
        newBlock->next = NULL;

    }
    else
    {
        tempptr = globalPointersList;
        while (tempptr->next != NULL)
        {
            tempptr = tempptr->next;
        }
        newBlock->prev = tempptr;
        tempptr->next = newBlock;
        newBlock->next = NULL;
    }
    newBlock++;
    memcpy(newBlock,oldp,size);
    sfree(oldp);
    return (void*)(newBlock);
}


size_t _num_free_blocks()
{

    return num_of_free_blocks;
}

size_t _num_free_bytes()
{
    return num_of_free_bytes;
}

size_t _num_allocated_blocks()
{
    return num_of_allocated_blocks;
}

size_t _num_allocated_bytes()
{

    return num_of_allocated_bytes;
}

size_t _num_meta_data_bytes()
{

    return num_of_meta_data_bytes;
}

size_t _size_meta_data()
{size_t returnSize = sizeof(MallocMetadata);
    return returnSize;

}
