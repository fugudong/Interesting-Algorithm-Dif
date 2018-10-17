#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUD_DATA_TYPE float
#define MAXQSIZE 2880//960ms*3
#define SAMPLE_PER_MS 48//48 kHz
#define AUDIO_BLOCK_SIZE 160


typedef struct{
    AUD_DATA_TYPE *data;
}QElemType; //1 ms 3 Elem

typedef struct{
    QElemType *base;
    int front;
    int rear;
    int elem_data_size;
    int audio_block_size;
    int elem_cnt_per_ms;
}SqQueue;


 int InitQueue(SqQueue *Q, int sample, int audio_block_size)
 {
    int i;
    QElemType *temp_elem;

   Q->base=(QElemType *)malloc(MAXQSIZE*sizeof(QElemType));
   if(!Q->base)
     return 0;
   Q->front=Q->rear=0;
   Q->elem_data_size = MaxDivision(sample, audio_block_size);
   Q->audio_block_size = audio_block_size;
   Q->elem_cnt_per_ms  = sample/Q->elem_data_size;

   temp_elem = Q->base;
   for(i=0; i<MAXQSIZE; i++){
        temp_elem->data = malloc(Q->elem_data_size*sizeof(AUD_DATA_TYPE));
        temp_elem++;
   }
   return 1;
 }

 int DestroyQueue(SqQueue *Q)
 {
     int i;
     QElemType *temp_elem;

     if(Q->base){
       temp_elem = Q->base;
       for(i=0; i<MAXQSIZE; i++){
            free(temp_elem->data);
            temp_elem++;
       }
       free(Q->base);
    }
   Q->base=NULL;
   Q->front=Q->rear=0;
   return 1;
 }

 int ClearQueue(SqQueue *Q)
 {
   Q->front=Q->rear=0;
   return 1;
 }

 int QueueEmpty(SqQueue Q)
 {
   if(Q.front==Q.rear)
     return 1;
   else
     return 0;
 }

 int QueueFull(SqQueue Q)
 {
   if(Q.front==(Q.rear+1)%MAXQSIZE)
     return 1;
   else
     return 0;
 }

 int QueueLength(SqQueue Q)
 {
   return(Q.rear-Q.front+MAXQSIZE)%MAXQSIZE;
 }

 int GetHead(SqQueue Q, QElemType *e)
 {
   if(Q.front==Q.rear)
     return 0;
   *e=*(Q.base+Q.front);
   return 1;
 }

 int EnQueue(SqQueue *Q, AUD_DATA_TYPE *in_data)
 {
     QElemType elem;
   if(Q->front==(Q->rear+1)%MAXQSIZE)
        return 0;

   elem = *(Q->base+Q->rear);
   memcpy(elem.data, in_data, Q->elem_data_size*sizeof(AUD_DATA_TYPE));
   Q->rear = (Q->rear+1)%MAXQSIZE;
   return 1;
 }

 int DeQueue(SqQueue *Q, AUD_DATA_TYPE *out_data)
 {
   QElemType elem;
   if(Q->front==Q->rear)
     return 0;
   elem = *(Q->base+Q->front);
   memcpy(out_data, elem.data, Q->elem_data_size*sizeof(AUD_DATA_TYPE));
   Q->front=(Q->front+1)%MAXQSIZE;
   return 1;
 }

int ModifyQueueFront(SqQueue *Q,int cnt, int dir)
 {
    if(dir){//delay small
        //if((Q->front +cnt +MAXQSIZE)%MAXQSIZE >= Q->rear )
        //    return 0;
        Q->front = (Q->front + cnt + MAXQSIZE)%MAXQSIZE;
    }
    else{//delay large
        //if((Q->front -cnt +MAXQSIZE)%MAXQSIZE <= (Q->rear+1) )
        //    return 0;
        Q->front = (Q->front - cnt + MAXQSIZE)%MAXQSIZE;
    }
   return 1;
 }

 int QueueTraverse(SqQueue *Q, void(*vi)(QElemType))
 {
   int i;
   i=Q->front;
   while(i!=Q->rear)
   {
     vi(*(Q->base+i));
     i++;
   }
   printf("\n");
   return 1;
 }

 void delay_audio_proc(SqQueue *q_aud, AUD_DATA_TYPE *in, AUD_DATA_TYPE *out, int delay_ms)
 {
    int i, dir, delt_ms;
    static int flag = 1, pre_delay = 0;

    for(i=0; i<q_aud->audio_block_size/q_aud->elem_data_size; i++){
        EnQueue(q_aud, in);
        in += q_aud->elem_data_size;
    }
    printf("After EnQueueLength %d\n",QueueLength(*q_aud));
    printf("Q front %d rear %d\n",q_aud->front, q_aud->rear);

    if(pre_delay != delay_ms){
        flag = 0;
        dir = pre_delay > delay_ms ? 1 :0;
        delt_ms = abs(pre_delay - delay_ms);
        printf("delay ms %d delt_ms %d dir %d\n", delay_ms, delt_ms, dir);
        pre_delay = delay_ms;
    }
    if(flag == 0){
        if(ModifyQueueFront(q_aud, delt_ms*q_aud->elem_cnt_per_ms, dir)){
            flag = 1;
            printf("After Modify front QueueLength %d\n",QueueLength(*q_aud));
            printf("Q front %d rear %d\n",q_aud->front, q_aud->rear);
        }
    }

    if(flag){
        for(i=0; i<q_aud->audio_block_size/q_aud->elem_data_size; i++){
            DeQueue(q_aud, out);
            out += q_aud->elem_data_size;
        }
        printf("After DeQueueLength %d\n",QueueLength(*q_aud));
        printf("Q front %d rear %d\n\n\n",q_aud->front, q_aud->rear);
    }

 }

int main()
{
    static int delay_ms = 4;
    int cnt = 0;
    SqQueue q_aud;
    AUD_DATA_TYPE in[AUDIO_BLOCK_SIZE], out[AUDIO_BLOCK_SIZE];

    InitQueue(&q_aud, SAMPLE_PER_MS, AUDIO_BLOCK_SIZE);


    for(cnt=0; cnt<40000; cnt++){
        if(cnt%50 == 0){
            delay_ms++;
        }
        delay_audio_proc(&q_aud, in, out, delay_ms);
    }
    printf("Hello world!\n");
    return 0;
}
