#include"protocol.h"
#include<stdlib.h>
#include<QDebug>

struct PDU *mkPDU(uint uintMsgLen)
{
    uint uintPDULen = sizeof(struct PDU)+uintMsgLen;    //
    struct PDU *pPDU = (struct PDU*)malloc(uintPDULen);
    if(pPDU == NULL) exit(EXIT_FAILURE);
    memset(pPDU, 0, uintPDULen);

    pPDU->uintPDULen = uintPDULen;
    //qDebug() << pPDU->uintPDULen;
    pPDU->uintMsgLen = uintMsgLen;

    //free(pPDU);
    //pPDU = NULL;

    return pPDU;
}
