#ifndef RM_MANAGER_H_H
#define RM_MANAGER_H_H

#include "PF_Manager.h"
#include "str.h"

typedef int SlotNum;

struct RID {	
	PageNum pageNum;	//��¼����ҳ��ҳ��
	SlotNum slotNum;		//��¼�Ĳ�ۺ�
	bool bValid; 			//true��ʾΪһ����Ч��¼�ı�ʶ��
    RID() : bValid(false) {}
	void copy(RID *rid) {
		pageNum = rid->pageNum;
		slotNum = rid->slotNum;
		bValid = rid->bValid;
	}
};

typedef struct
{
	int bLhsIsAttr,bRhsIsAttr;//���������ԣ�1������ֵ��0��
	AttrType attrType;
	int LattrLength,RattrLength;
	int LattrOffset,RattrOffset;
	CompOp compOp;
	void *Lvalue,*Rvalue;
}Con;

typedef struct {
    int nRecords;			//��ǰ�ļ��а����ļ�¼��
    int recordSize;			//ÿ����¼�Ĵ�С
    int recordsPerPage;		//ÿ��ҳ�����װ�صļ�¼����
    int firstRecordOffset;	//ÿҳ��һ����¼���������еĿ�ʼλ��
} RM_FileSubHeader;

struct RM_FileHandle{//�ļ����
	bool bOpen;//����Ƿ�򿪣��Ƿ����ڱ�ʹ�ã�
	//��Ҫ�Զ������ڲ��ṹ
    PF_FileHandle* pFileHandle;         // ҳ�����ľ��
    PF_PageHandle* pPageHandle;         // ��һҳ�ľ��
    RM_FileSubHeader* fileSubHeader;    // ��¼������Ϣ����һҳ��
    char* bitmap;                       // ��¼����λͼ����һҳ��
    RM_FileHandle() : bOpen(false) {}
};

struct RM_FileScan {
	bool  bOpen;		//ɨ���Ƿ�� 
	RM_FileHandle  *pRMFileHandle;		//ɨ��ļ�¼�ļ����
	int  conNum;		//ɨ���漰���������� 
	Con  *conditions;	//ɨ���漰����������ָ��
    PF_PageHandle  PageHandle; //�����е�ҳ����
	PageNum  pn; 	//ɨ�輴�������ҳ���
	SlotNum  sn;		//ɨ�輴������Ĳ�ۺ�
    RM_FileScan() :bOpen(false) {}
};

struct RM_Page {
	int* nRecords;
	char* bitmap;
	char* rData;
    RM_FileSubHeader* fileSubHeader;
	RM_Page(char* pData, RM_FileSubHeader* fileSubHeader):
        fileSubHeader(fileSubHeader),
        nRecords((int*)pData),
        bitmap(pData+sizeof(int)),
        rData(pData+fileSubHeader->firstRecordOffset) {}
    char* get(int slotNum) {
        return rData + slotNum * fileSubHeader->recordSize;
    }
};

struct RM_Record{
	bool bValid;		 // False��ʾ��δ�������¼
	RID  rid; 		 // ��¼�ı�ʶ�� 
	char *pData; 		 //��¼���洢������ 
	RM_Record() : bValid(false), pData(new char[100]) {}
    RM_Record(const RM_FileHandle* fileHandle) : bValid(false) {
        pData = new char[fileHandle->fileSubHeader->recordSize];
		//pData = (char*)malloc(sizeof(fileHandle->fileSubHeader->recordSize));
    }
    ~RM_Record() {
        delete[] pData;
		//free(pData);
    }
};

RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec);

RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions);

RC CloseScan(RM_FileScan *rmFileScan);

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec);

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid);

RC InsertRec (RM_FileHandle *fileHandle, char *pData, RID *rid); 

RC GetRec (RM_FileHandle *fileHandle, RID *rid, RM_Record *rec); 

RC RM_CloseFile (RM_FileHandle *fileHandle);

RC RM_OpenFile (char *fileName, RM_FileHandle *fileHandle);

RC RM_CreateFile (char *fileName, int recordSize);

#endif