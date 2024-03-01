#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))

#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

struct block *head = NULL;  // Head of the list, empty

struct block {
    int size;            // Size in bytes
    int in_use;          // Boolean
    struct block *next;
};

void *myalloc(int size){

    if(head==NULL){                    //initialization case
        void *heap = mmap(NULL, 1024, PROT_READ|PROT_WRITE,
                MAP_ANON|MAP_PRIVATE, -1, 0);
        head=heap;
        int data_size = 1024 - PADDED_SIZE(sizeof(struct block));
        head->size = data_size;
        head->in_use = 0;
    }

    struct block *temp=head;
    while(temp){
        if(temp->in_use==0&&temp->size>=size){ 
            temp->in_use=1;
            if((long unsigned int)temp->size>=(PADDED_SIZE(size)+PADDED_SIZE(sizeof(struct block))+16)){
                struct block *new=PTR_OFFSET(temp,PADDED_SIZE(sizeof(struct block))+PADDED_SIZE(size));
                new->in_use=0;
                new->size=temp->size-PADDED_SIZE(size)-PADDED_SIZE(sizeof(struct block));
                temp->size=PADDED_SIZE(size);
                new->next=temp->next;
                temp->next=new;
            }
            return PTR_OFFSET(temp,PADDED_SIZE(sizeof(struct block)));
        }
        temp=temp->next;
    }
    return NULL;
}

void coalesce(){
    struct block *temp=head;

    while(temp->next){
        if(!temp->in_use&&!temp->next->in_use){
            temp->size=temp->size+PADDED_SIZE(sizeof(struct block))+temp->next->size;
            temp->next=temp->next->next;
        }
        else{
            temp=temp->next;
        }
    }
}

void myfree(void *p){
    struct block *temp=head;

    while(temp){
        if(PTR_OFFSET(temp,PADDED_SIZE(sizeof(struct block)))==p){
            temp->in_use=0;
            temp=NULL;
            coalesce();
        }else{
            temp=temp->next;
        }
    }
}

void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main(){
    
    void *p;
    p = myalloc(10); print_data();
    myfree(p); print_data();
    

    /*
    void *p, *q;
    p = myalloc(10); print_data();
    q = myalloc(20); print_data();
    myfree(p); print_data();
    myfree(q); print_data();
    */
    
    /*
    void *p, *q;

    p = myalloc(10); print_data();
    q = myalloc(20); print_data();

    myfree(q); print_data();
    myfree(p); print_data();
    */

    /*
    void *p, *q, *r, *s;
    p = myalloc(10); print_data();
    q = myalloc(20); print_data();
    r = myalloc(30); print_data();
    s = myalloc(40); print_data();
    myfree(q); print_data();
    myfree(p); print_data();
    myfree(s); print_data();
    myfree(r); print_data();
    */    
}