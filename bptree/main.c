//
//  main.c
//  BPTree
//
//  Created by 孙硕磊 on 11/28/14.
//  Copyright (c) 2014 dhu.cst. All rights reserved.
//

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<time.h>

struct BPNode;
/*
 辅助队列，以链表形式实现
 */
struct QueueNode
{
    struct BPNode * bpNode;
    struct QueueNode * next;
};

struct Queue
{
    struct QueueNode * head;        //指向队列中的第一个元素
    struct QueueNode * rear;        //指向队列中的最后一个元素
};

/*
 创建队列，并初始化
 */
struct Queue createQueue()
{
    struct Queue q;
    q.head=q.rear=NULL;
    return q;
}
/*
 判断队列是否为空
 */
int isQueueEmpty(struct Queue *q)
{
    if(q->head==NULL||q->rear==NULL)
    {
        return 1;
    }
    return 0;
}
/*
 向队列中插入节点
 */
void insertToQueue(struct Queue *q,struct QueueNode * node)
{
    if(node==NULL)
    {
        return;
    }
    //队列为空
    if(q->head==NULL||q->rear==NULL)
    {
        q->head=node;
        q->rear=node;
    }
    else
    {
        q->rear->next=node;
        q->rear=node;
    }
}

/**
 删除节点
 */
void deleteFirstFromQueue(struct Queue *q)
{
    //队列为空
    if(q->head==NULL||q->rear==NULL)
    {
        return;
    }
    //队列中只有一个元素
    if(q->head==q->rear)
    {
        free(q->head);
        q->head=q->rear=NULL;
    }
    else
    {
        struct QueueNode * node=q->head;
        q->head=node->next;
        free(node);
    }
}

struct QueueNode * getFrontFromQueue(struct Queue *q)
{
    return q->head;
}

/*
 构建内存中的B+树
 */

#define T 10			                //B+树的阶数

//B+树节点
struct BPNode
{
    unsigned int n;	                   //节点有多少个关键字
    int isLeaf;		                   //判断是否为叶节点
    int  keys[T];                      //若为非叶节点，则Key为每个分支的最大关键字
    struct BPNode * keyPointers[T];    //关键字指针
    struct BPNode * parentPointer;     //父节点的指针
    struct BPNode * nextPointer;       //下一个兄弟指针
};

//KeyIndex，用于查找指定关键字的返回值
struct KeyIndex
{
    struct BPNode * bpNode;               //key所在的节点
    int index;                     //key在bpNode->keys中的位置
};

//B+树节点
struct BPTree
{
    struct BPNode * rootPointer;               //根节点指针
    struct BPNode * minPointer;                //最小关键字指针
};

struct BPTree bpTree;		                   //全局变量，b+树


/*
 创建B+树,并进行相应的初始化
 */
void createBPTree()
{
    
    struct BPNode * rootPointer=( struct BPNode *)malloc(sizeof(struct BPNode));
    bpTree.rootPointer = rootPointer;
    bpTree.minPointer=rootPointer;
    rootPointer->n = 0;
    rootPointer->isLeaf=1;
    rootPointer->nextPointer=NULL;
    rootPointer->parentPointer=NULL;
    memset(rootPointer->keyPointers,0,T*sizeof(struct BPNode *));
}

/**
 查询待插入关键字位置
 */
int findInsertingIndex(int keys[],int n,int key)
{
    int i=0;
    while(i<n&&key>keys[i])
    {
        i++;
    }
    return i;
}
/*
 更新指定节点的父节点关键字
 */
void updateParentKey(struct BPNode * p,int newMaxKey)
{
    struct BPNode * parent=p->parentPointer;
    while(parent!=NULL)
    {
        int i=0;
        while(parent->keyPointers[i]!=p)
        {
            i++;
        }
        parent->keys[i]=newMaxKey;
        //更新了父节点的最大关键字，则依次向上更新;否则，终止
        if(i==parent->n-1)
        {
            p=parent;
            parent=parent->parentPointer;
        }
        else
        {
            parent=NULL;
        }
    }
}

/*
  在分裂和合并节点时，要充分考虑发生发生改变节点的所有指针域是否发生改变
  (1)发生分裂或合并的叶节点
  (2)发生分裂或合并的为非叶节点
 */

/*
 分裂节点
 p指向待分裂的节点
 key为待插入关键字
 keyPointer为待插入关键字指针
 若 keyPointer为NULL，则为叶节点分裂；否则为非叶节点分裂
 */
void split(struct BPNode * p,int key,struct BPNode * keyPointer)
{
    //复制元素
    int keys[T+1];
    struct BPNode * keyPointers[T+1];
    memcpy(keys,p->keys,T*sizeof(int));
    memcpy(keyPointers,p->keyPointers,T*sizeof(struct BPNode *));
    //查找待插入位置
    int index=findInsertingIndex(keys,T, key);
    //后移key和keyPointer
    for(int i=T-1;i>=index;i--)
    {
        keys[i+1]=keys[i];
        keyPointers[i+1]=keyPointers[i];
    }
    //插入key和keyPointer
    keys[index]=key;
    keyPointers[index]=keyPointer;
    int n1=(T+1)/2;                       //分裂后p节点的关键字个数
    int n2=T+1-n1;                        //分裂后新申请节点的关键字个数
    //申请新的节点，并赋值
    struct BPNode * newBPNode=(struct BPNode *)malloc(sizeof(struct BPNode));
    newBPNode->n=n2;
    for(int i=0;i<n2;i++)
    {
        newBPNode->keys[i]=keys[i+n1];
        newBPNode->keyPointers[i]=keyPointers[i+n1];
    }
    if(keyPointer==NULL)                 //叶节点
    {
        newBPNode->isLeaf=1;
        memset(newBPNode->keyPointers,0,T*sizeof(struct BPNode *));
        memset(p->keyPointers,0,T*sizeof(struct BPNode *));
    }
    else
    {
        newBPNode->isLeaf=0;
        //更新p及newBPNode孩子节点的父指针
        for(int i=0;i<n1;i++)
        {
            p->keyPointers[i]->parentPointer=p;
        }
        for(int i=0;i<n2;i++)
        {
            newBPNode->keyPointers[i]->parentPointer=newBPNode;
        }
    }
    newBPNode->nextPointer=p->nextPointer;
    newBPNode->parentPointer=p->parentPointer;
    //更新节点p
    memcpy(p->keys,keys,n1*sizeof(int));
    memcpy(p->keyPointers,keyPointers,n1*sizeof(struct BPNode *));
    p->n=n1;
    p->nextPointer=newBPNode;
    //更新父节点
    struct BPNode * parent=p->parentPointer;
    if(parent==NULL)                                   //根节点
    {
        struct BPNode * root=( struct BPNode *)malloc(sizeof( struct BPNode));
        root->n =2;
        root->isLeaf=0;
        root->nextPointer=NULL;
        root->parentPointer=NULL;
        root->keys[0]=p->keys[p->n-1];
        root->keyPointers[0]=p;
        root->keys[1]=newBPNode->keys[newBPNode->n-1];
        root->keyPointers[1]=newBPNode;
        bpTree.rootPointer=root;
        if(keyPointer==NULL)
        {
            bpTree.minPointer=p;
        }
        p->parentPointer=root;
        p->nextPointer=newBPNode;
        newBPNode->parentPointer=root;
        newBPNode->nextPointer=NULL;
        return;                                    //分裂结束
    }
    else
    {
        //查找p节点指针在父节点keyPointers的位置
        int i=0;
        while(i<parent->n&&p!=parent->keyPointers[i])
        {
            i++;
        }
        parent->keys[i]=p->keys[p->n-1];         //更新p对应的关键字
        int maxNewKey=newBPNode->keys[newBPNode->n-1];
        if(parent->n<T)                          //p的父节点可以插入元素
        {
            //后移父节点关键字和孩子指针，以便插入新的key和keyPointer
            for( int j=parent->n-1;j>i;j--)
            {
                parent->keys[j+1]=parent->keys[j];
                parent->keyPointers[j+1]=parent->keyPointers[j];
            }
            //在父节点中插入maxNewKey和keyPointer
            parent->keys[i+1]=maxNewKey;
            parent->keyPointers[i+1]=newBPNode;
            parent->n++;
            parent->keyPointers[i]->nextPointer=newBPNode;
            newBPNode->parentPointer=parent;
            if(maxNewKey>parent->keys[parent->n-1])
            {
                //更新最大关键字
                parent=parent->parentPointer;
                while(parent!=NULL)
                {
                    parent->keys[parent->n-1]=maxNewKey;
                    parent=parent->parentPointer;
                }
            }
        }
        else                                   //p的父节点已满，则需要分裂
        {
            split(parent,maxNewKey,newBPNode);
        }
    }
}

/*
 查找指定关键字，若查找成功，则返回关键字所在的节点，否则返回NULL
 */
struct KeyIndex findKeyFromBPTree(struct BPTree * bpTree,int key)
{
    struct KeyIndex keyIndex;
    keyIndex.bpNode=NULL;
    keyIndex.index=-1;
    assert(bpTree!=NULL);
    struct BPNode * root=bpTree->rootPointer;
    assert(root!=NULL);
    struct BPNode * p=root;
    //若该关键字存在，则关键字一定在叶节点中
    while(!p->isLeaf)
    {
        int * keys=p->keys;
        int n=p->n;
        int i=0;
        //查找key所在的分支
        while(i<n&&key>keys[i])
        {
            i++;
        }
        //查询的关键字大于已有的关键字，则返回0
        if(i>=n)
        {
            return keyIndex;
        }
        else
        {
            p=p->keyPointers[i];
        }
        
    }
    //此时p指向叶节点，判断key是否存在
    int * keys=p->keys;
    int n=p->n;
    for(int i=0;i<n;i++)
    {
        if(key==keys[i])
        {
            keyIndex.bpNode=p;
            keyIndex.index=i;
            return keyIndex;
        }
    }
    return keyIndex;
}

/*
 向B+树中插入关键字，插入成功，返回1，否则，返回0
 当插入的关键字已存在时，返回0
 */
int insertKeyToBpTree(struct BPTree * bpTree,int key)
{
    assert(bpTree!=NULL);
    struct BPNode * root=bpTree->rootPointer;
    assert(root!=NULL);
    struct BPNode * p=root;
    //插入关键字时，一定是在叶节点插入
    while(!p->isLeaf)
    {
        int * keys=p->keys;
        int n=p->n;
        int i=0;
        //查找带插入key所在的分支
        while(i<n&&key>keys[i])
        {
            i++;
        }
        //待插入的关键字大于已有的关键字，则需要找到最大关键字所在的叶节点
        if(i>=n)
        {
            while(!p->isLeaf)
            {
                p=p->keyPointers[p->n-1];
            }
        }
        else
        {
            p=p->keyPointers[i];
        }
        
    }
    //此时p指向带插入的叶节点，判断p是否还可以插入
    if(p->n<T)
    {
        //查找待插入位置
        int index=findInsertingIndex(p->keys,p->n,key);
        //关键字已存在
         if(key==p->keys[index])
         {
           return 0;
         }
        //判断带插入的key是否为最大
        int isMax=0;
        if(index==p->n)
        {
            isMax=1;
        }
        int n=p->n;
        //后移元素，以便插入key
        for(int i=n-1;i>=index;i--)
        {
            p->keys[i+1]=p->keys[i];
        }
        //插入key
        p->keys[index]=key;
        p->n++;
        if(isMax)                          //新插入的节点为最大关键字，更新对应的父节点
        {
            updateParentKey(p, key);
        }
    }
    else                                    //叶节点已满,叶节点需要分裂
    {
        split(p,key,NULL);
    }
    
    return 1;
}

/*
 在指定节点p中，向前或向后移动keys和keyPointers
 */
void moveElements(struct BPNode * p,int start,int isFront)
{
    //向前移动元素
    if(isFront)
    {
        if(start<=0)
        {
            return ;
        }
        for(int i=start;i<p->n;i++)
        {
            p->keys[i-1]=p->keys[i];
            p->keyPointers[i-1]=p->keyPointers[i];
        }
    }
    else    //向后移动元素
    {
        if(start>=p->n||start<0)
        {
            return ;
        }
        for(int i=p->n-1;i>=start;i--)
        {
            p->keys[i+1]=p->keys[i];
            p->keyPointers[i+1]=p->keyPointers[i];
        }
    }
}

/*
 查找左兄弟节点
 */
struct BPNode * findLSiblingNode(struct BPNode * p)
{
    if(p==NULL||p->parentPointer==NULL)
    {
        return NULL;
    }
    struct BPNode * lSibling=NULL;
    struct BPNode * parent=p->parentPointer;
    //查找左兄弟
    int i=0;
    while(parent->keyPointers[i]!=p)
    {
        i++;
    }
    //节点p存在左兄弟,且左兄弟和p有共同的父节点
    if(i>0)
    {
        lSibling=parent->keyPointers[i-1];
    }
    else
    {//i==0,则p的左兄弟的父节点与p的父节点为左右兄弟节点
        struct BPNode * pLSibling=findLSiblingNode(p->parentPointer);
        if(pLSibling)
        {
            lSibling=pLSibling->keyPointers[pLSibling->n-1];
        }
    }
    return lSibling;
}

int deleteKeyFromBPNode(struct BPNode * p,int key,int index)
{
    if(p==NULL||p->n==0)
    {
        return 1;
    }
    if(index<0||index>=p->n)
    {
        return 0;
    }
    //从节点p中删除key
    moveElements(p,index+1,1);
    p->n--;
    //p为根节点rr
    if(p->parentPointer==NULL)
    {
        //B+数为空
        if(p->n==0)
        {
            
        }
        //根元素只有一个关键字，且存在子节点，则删除根节点,并使根指针指向源根的子节点
        if(p->n==1&&p->keyPointers[0]!=NULL)
        {
            bpTree.rootPointer=p->keyPointers[0];
            p->keyPointers[0]->parentPointer=NULL;
            free(p);
            p=NULL;
        }
        return 1;
    }
    //删除后节点p中的关键字个数符合要求，则直接删除
    if(p->n>=(T+1)/2)
    {
        //若删除的是节点p的最大关键字，则需要更新父节点关键字
        if(index==p->n)
        {
            updateParentKey(p,p->keys[p->n-1]);
        }
    }
    else  //删除后节点关键字个数不符合要求，则需要向左兄弟或右兄弟借，或跟左兄弟或右兄弟合并
    {
        int canBorrowFromRS=0;
        int canBorrowFromLS=0;
        //首先向右兄弟借，若可以借到,则插入删除关键字的节点p中，且借到的关键字在叶节点p中最大
        struct BPNode * rSibling=p->nextPointer;
        struct BPNode * lSibling=findLSiblingNode(p);
        if(rSibling!=NULL)
        {
            //右兄弟可以借
            if(rSibling->n-1>=(T+1)/2)
            {
                canBorrowFromRS=1;
                //插入借到的关键字,该关键字最大
                p->keys[p->n]=rSibling->keys[0];
                p->keyPointers[p->n]=rSibling->keyPointers[0];
                p->n++;
                //更改借来关键字指向节点的父节点
                if (p->isLeaf==0)
                {
                  rSibling->keyPointers[0]->parentPointer=p;
                }
                //更新p的父节点关键字
                updateParentKey(p,p->keys[p->n-1]);
                //从右兄弟中删除借到的关键字,也即前移元素
                moveElements(rSibling,1,1);
                rSibling->n--;
            }
        }
        //右兄弟不可以借，则尝试向左兄弟借
        if(canBorrowFromRS==0)
        {
            if(lSibling)
            {
                //左兄弟可以借，则借到的关键字在节点p中最小，需后移p中的元素，并且更新左兄弟的父节点关键字
                if(lSibling->n-1>=(T+1)/2)
                {
                    canBorrowFromLS=1;
                    //后移元素，以便插入借到的key
                    moveElements(p,0,0);
                    //插入借到的关键字,该关键字最小
                    p->keys[0]=lSibling->keys[lSibling->n-1];
                    p->keyPointers[0]=lSibling->keyPointers[lSibling->n-1];
                    p->n++;
                    //更改借来关键字指向节点的父节点
                    if (p->isLeaf==0)
                    {
                      lSibling->keyPointers[lSibling->n-1]->parentPointer=p;
                    }
                    //从左兄弟中删除借到的关键字，该关键字在左兄弟中为最大，只需关键字个数减1，并且需要更新父节点关键字
                    lSibling->n--;
                    //更新左兄弟的父节点关键字
                    updateParentKey(lSibling,lSibling->keys[lSibling->n-1]);
                    //若删除的是节点p的最大关键字，则需要更新父节点关键字
                    if(index==p->n-1)
                    {
                        updateParentKey(p,p->keys[p->n-1]);
                    }
                }
            }
        }
        //如果从左右兄弟都借不到，则可以将节点p合并到右兄弟节点中，若右兄弟节点为NULL，则合并到左兄弟节点中
        if(canBorrowFromRS==0&&canBorrowFromLS==0)
        {
            //跟右兄弟合并
            if(rSibling!=NULL)
            {
                int n1=p->n;
                int n2=rSibling->n;
                int keys[T];
                struct BPNode * keyPointers[T];
                //复制元素
                memset(keys,0,T*sizeof(int));
                memset(keyPointers,0,T*sizeof(struct BPNode *));
                memcpy(keys,p->keys,n1*sizeof(int));
                memcpy(keyPointers,p->keyPointers,n1*sizeof(struct BPNode *));
                for(int i=0;i<n2;i++)
                {
                    keys[n1+i]=rSibling->keys[i];
                    keyPointers[n1+i]=rSibling->keyPointers[i];
                }
                int n=n1+n2;
                memcpy(rSibling->keys,keys,n*sizeof(int));
                memcpy(rSibling->keyPointers,keyPointers,n*sizeof(struct BPNode *));
                rSibling->n=n;
                //更新p的左兄弟节点的下一个节点为p的下一个节点
                if(lSibling)
                {
                    lSibling->nextPointer=rSibling;
                }
                //删除p在父节点中的关键字及指针
                struct BPNode * parent=p->parentPointer;
                int i=0;
                while(parent->keyPointers[i]!=p)
                {
                    i++;
                }
                parent->keyPointers[i]=NULL;
                /*
                 若p为非叶节点更新p孩子节点的父节点为p的右兄弟
                 */
                if(p->isLeaf==0)
                {
                    for(int i=0;i<p->n;i++)
                    {
                        p->keyPointers[i]->parentPointer=rSibling;
                    }
                }
                else
                {
                    //p是否为最小关键字所在的节点
                    if(p==bpTree.minPointer)
                    {
                        bpTree.minPointer=rSibling;
                    }
                }
                free(p);
                p=NULL;
                deleteKeyFromBPNode(parent,parent->keys[i],i);
                
            }
            else //跟左兄弟合并
            {
                int n1=lSibling->n;
                int n2=p->n;
                //把p节点中元素拼接在左兄弟之后
                for(int i=0;i<n2;i++)
                {
                    lSibling->keys[n1+i]=p->keys[i];
                    lSibling->keyPointers[n1+i]=p->keyPointers[i];
                }
                lSibling->n=n1+n2;
                //更新p的左兄弟节点的下一个节点为p的下一个节点
                lSibling->nextPointer=rSibling;
                //删除p在父节点中的关键字及指针
                struct BPNode * parent=p->parentPointer;
                int i=0;
                while(parent->keyPointers[i]!=p)
                {
                    i++;
                }
                parent->keyPointers[i]=NULL;
                /*
                 若p为非叶节点更新p孩子节点的父节点为p的左兄弟
                 */
                if(p->isLeaf==0)
                {
                    for(int i=0;i<p->n;i++)
                    {
                        p->keyPointers[i]->parentPointer=lSibling;
                    }
                }
                free(p);
                p=NULL;
                //更新lSibling父节点中对应的关键字
                //获取lSibling的父节点
                struct BPNode * lSParent=lSibling->parentPointer;
                int j=0;
                while(lSParent->keyPointers[j]!=lSibling)
                {
                    j++;
                }
                lSParent->keys[j]=lSibling->keys[lSibling->n-1];
                deleteKeyFromBPNode(parent,parent->keys[i],i);
            }
        }
    }
    return 1;
}
/*
 从B+树中删除指定的关键字key
 若关键字不存在，则返回0；否则删除关键字，并返回1
 */
int deleteKeyFromBPTree(struct BPTree * bpTree,int key)
{
    struct KeyIndex keyIndex=findKeyFromBPTree(bpTree,key);
    //关键字不存在
    if(keyIndex.bpNode==NULL&&keyIndex.index==-1)
    {
        return 0;
    }
    return deleteKeyFromBPNode(keyIndex.bpNode,key,keyIndex.index);
}

/*
 释放B+树
 */
void freeBPTree(struct BPTree * bpTree)
{
    if(bpTree==NULL||bpTree->rootPointer==NULL)
    {
        return;
    }
    struct Queue q=createQueue();
    struct QueueNode *qNode=(struct QueueNode *) malloc(sizeof(struct QueueNode));
    qNode->bpNode=bpTree->rootPointer;
    qNode->next=NULL;
    insertToQueue(&q,qNode);
    while(!isQueueEmpty(&q))
    {
        qNode=getFrontFromQueue(&q);
        int n=qNode->bpNode->n;
        struct BPNode ** keyPointers=qNode->bpNode->keyPointers;
        for(int i=0;i<n;i++)
        {
            if(!qNode->bpNode->isLeaf)
            {
                struct QueueNode * newQNode=(struct QueueNode *) malloc(sizeof(struct QueueNode));
                newQNode->bpNode=keyPointers[i];
                newQNode->next=NULL;
                insertToQueue(&q,newQNode);
            }
        }
        free(qNode->bpNode);
        deleteFirstFromQueue(&q);
    }
}
/*
 display BPTree
 从minPointer开始遍历所有关键字
 */
void displayBPTree(struct BPTree * bpTree)
{
    if(bpTree==NULL||bpTree->minPointer==NULL)
        return;
    struct BPNode * p=bpTree->minPointer;
    while(p!=NULL)
    {
        int n=p->n;
        int * keys=p->keys;
        printf("[ ");
        for(int i=0;i<n;i++)
            printf("%d ",keys[i]);
        printf(" ]  ");
        p=p->nextPointer;
    }
    printf("\n");
}

/*
 广度遍历B+树
 */

void levelTraversal(struct BPTree * bpTree)
{
    if(bpTree==NULL||bpTree->rootPointer==NULL)
    {
        return;
    }
    struct Queue q=createQueue();
    struct QueueNode *qNode=(struct QueueNode *) malloc(sizeof(struct QueueNode));
    qNode->bpNode=bpTree->rootPointer;
    qNode->next=NULL;
    insertToQueue(&q,qNode);
    while(!isQueueEmpty(&q))
    {
        qNode=getFrontFromQueue(&q);
        int n=qNode->bpNode->n;
        int * keys=qNode->bpNode->keys;
        struct BPNode ** keyPointers=qNode->bpNode->keyPointers;
        printf("[");
        for(int i=0;i<n;i++)
        {
            printf(" %d",keys[i]);
            if(!qNode->bpNode->isLeaf)
            {
                struct QueueNode * newQNode=(struct QueueNode *) malloc(sizeof(struct QueueNode));
                newQNode->bpNode=keyPointers[i];
                newQNode->next=NULL;
                insertToQueue(&q,newQNode);
            }
        }
        printf("]  ");
        if(qNode->bpNode->nextPointer==NULL)
            printf("\n");
        deleteFirstFromQueue(&q);
    }
}

int main()
{
    /*
    FILE * fp=fopen("/Users/ssl/data.txt", "wb");
    fread(data,sizeof(int),n,fp);
    fwrite(data,sizeof(int),n,fp);
    fclose(fp);
     */
    //准备数据
    int n=150;
    int data[1024];
    srand((unsigned)time(NULL));
    printf("data: ");
    for(int i=0;i<n;i++)      
    {    
        data[i]=rand()%1000;
        printf("%d ",data[i]);
    }
    printf("\n");
    printf("B+ tree:\n");
    createBPTree();
    for(int i=n-1;i>=0;i--)
    {
        //printf("insert %d,the B+ tree：\n",data[i]);
        insertKeyToBpTree(&bpTree,data[i]);
        //levelTraversal(&bpTree);
    }
    levelTraversal(&bpTree);
    
    
    for(int i=0;i<n;i++)
    {
        //printf("delete %d,the B+ tree：\n",data[i]);
         deleteKeyFromBPTree(&bpTree,data[i]);
        //displayBPTree(&bpTree);
        //levelTraversal(&bpTree);
    }
    levelTraversal(&bpTree);
    
    freeBPTree(&bpTree);
    //system("pause");
    return 0;
}