#ifndef PF_MANAGER_H_H
#define PF_MANAGER_H_H

#include <stdio.h>
/*#include <stdlib.h>*/
#include<io.h>
#include<fcntl.h>
#include<sys/types.h>

#include<sys/stat.h>
#include<string.h>
#include <time.h>
#include <malloc.h>
#include<windows.h>
#define RUN(func) {RC rc; if ((rc = (func)) != SUCCESS) return rc;}
#define PF_PAGE_SIZE ((1<<12)-4)
#define PF_FILESUBHDR_SIZE (sizeof(PF_FileSubHeader))
#define PF_BUFFER_SIZE 50//页面缓冲区的大小
#define PF_PAGESIZE (1<<12)
typedef unsigned int PageNum;

typedef struct{
	PageNum pageNum;
	char pData[PF_PAGE_SIZE];
}Page;

typedef struct{
	PageNum pageCount;
	int nAllocatedPages;
}PF_FileSubHeader;

typedef struct{
	bool bDirty;
	unsigned int pinCount;
	clock_t  accTime;
	char *fileName;
	int fileDesc;
	Page page;
}Frame;

struct PF_FileHandle {
	bool bopen;
	char *fileName;
	int fileDesc;
	Frame *pHdrFrame;
	Page *pHdrPage;
	char *pBitmap;
	PF_FileSubHeader *pFileSubHeader;
};

typedef struct{
	int nReads;
	int nWrites;
	Frame frame[PF_BUFFER_SIZE];
	bool allocated[PF_BUFFER_SIZE];
}BF_Manager;



struct PF_PageHandle{
	bool bOpen;
	Frame *pFrame;
};

#ifndef RC_HH
#define RC_HH
typedef enum{
	SUCCESS=0,
		PF_EXIST,
		PF_FILEERR,
		PF_INVALIDNAME,
		PF_WINDOWS,
		PF_FHCLOSED,
		PF_FHOPEN,
		PF_PHCLOSED,
		PF_PHOPEN,
		PF_NOBUF,
		PF_EOF,
		PF_INVALIDPAGENUM,
		PF_NOTINBUF,
		PF_PAGEPINNED,
		RM_FHCLOSED,        // FileHandle closed
		RM_FHOPENNED,       // FileHandle openned
		RM_INVALIDRECSIZE,  // recordsize<=0 || recordsize>=页大小
		RM_INVALIDRID,      // rid不合法
        RM_INVALIDREC,      // rec不合法
		RM_FSCLOSED,        // FileScan closed
		RM_NOMORERECINMEM,  // ??? no more rec in mem
		RM_FSOPEN,          // FileScan openned
        RM_EOF,
		IX_IHOPENNED,
		IX_IHCLOSED,
		IX_INVALIDKEY,
		IX_NOMEM,
		RM_NOMOREIDXINMEM,  // ??? no more index in mem
		IX_EOF,
		IX_SCANCLOSED,
		IX_ISCLOSED,
		IX_NOMOREIDXINMEM,
		IX_SCANOPENNED,
		FAIL,
		SQL_SYNTAX=-10
}RC;
#endif


const RC CreateFile(const char *fileName);
const RC openFile(char *fileName,PF_FileHandle *fileHandle);
const RC CloseFile(PF_FileHandle *fileHandle);

const RC GetThisPage(PF_FileHandle *fileHandle,PageNum pageNum,PF_PageHandle *pageHandle);
const RC AllocatePage(PF_FileHandle *fileHandle,PF_PageHandle *pageHandle);
const RC GetPageNum(PF_PageHandle *pageHandle,PageNum *pageNum);

const RC GetData(PF_PageHandle *pageHandle,char **pData);
const RC DisposePage(PF_FileHandle *fileHandle,PageNum pageNum);

const RC MarkDirty(PF_PageHandle *pageHandle);

const RC UnpinPage(PF_PageHandle *pageHandle);

#endif