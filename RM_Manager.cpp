#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"
#include "bitmap.h"
#include <string>


RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//初始化扫描
{
    if (rmFileScan->bOpen) return RM_FSOPEN;
    if (!fileHandle->bOpen) return RM_FHCLOSED;

    PF_FileHandle* pFileHandle = fileHandle->pFileHandle;

    rmFileScan->bOpen = true;
    rmFileScan->pRMFileHandle = fileHandle;
    rmFileScan->conNum = conNum;
    rmFileScan->conditions = conditions;
	rmFileScan->PageHandle.bOpen = false;

    PageNum pageCount = pFileHandle->pFileSubHeader->pageCount;
    if (pageCount == 1)
        return SUCCESS;

    PageNum i;
    for (i = 2; i <= pageCount; ++i) {
        if (BM::check(pFileHandle->pBitmap, i)) {   // 该页已分配
            RUN(GetThisPage(pFileHandle, i, &rmFileScan->PageHandle));
            rmFileScan->pn = i;
            rmFileScan->sn = 0;
            return SUCCESS;
        }
    }
    return SUCCESS;
}

RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{
    if (!rmFileScan->bOpen) return RM_FSCLOSED;
    if (!rmFileScan->pRMFileHandle->bOpen) return RM_FHCLOSED;
	if (!rmFileScan->PageHandle.bOpen) return RM_EOF;

    RM_FileHandle* fileHandle = rmFileScan->pRMFileHandle;
    RM_FileSubHeader* fileSubHeader = fileHandle->fileSubHeader;
    PF_FileHandle* pFileHandle = fileHandle->pFileHandle;
    
    PageNum pageCount = pFileHandle->pFileSubHeader->pageCount;
    int nAllocatedPages = pFileHandle->pFileSubHeader->nAllocatedPages;
    PageNum& pn = rmFileScan->pn;
    int& sn = rmFileScan->sn;

    // 
    while (pn <= pageCount) {
        char* pData;
        RUN(GetData(&rmFileScan->PageHandle, &pData));
        RM_Page rmPage(pData, fileSubHeader);

        for (; sn < fileSubHeader->recordsPerPage; ++sn) {
            if (BM::check(rmPage.bitmap, sn)) {   // 该槽有记录
                char* record = rmPage.get(sn);
                bool b = true;
                for (int i = 0; b && i < rmFileScan->conNum; ++i) {
                    Con &cond = rmFileScan->conditions[i];
                    void *pL, *pR;
                    if (cond.bLhsIsAttr)
                        pL = record + cond.LattrOffset;
                    else
                        pL = cond.Lvalue;
                    if (cond.bRhsIsAttr)
                        pR = record + cond.RattrOffset;
                    else
                        pR = cond.Rvalue;
                    if (cond.attrType == chars) {
                        std::string l((char*)pL);
                        std::string r((char*)pR);
                        switch (cond.compOp)
                        {
                        case EQual:
                            b = (l == r);
                            break;
                        case LEqual:
                            b = (l <= r);
                            break;
                        case NEqual:
                            b = (l != r);
                            break;
                        case LessT:
                            b = (l < r);
                            break;
                        case GEqual:
                            b = (l >= r);
                            break;
                        case GreatT:
                            b = (l > r);
                            break;
                        default:
                            break;
                        }
                    }
                    else if (cond.attrType == ints) {
                        int l = *(int*)pL;
                        int r = *(int*)pR;
                        switch (cond.compOp)
                        {
                        case EQual:
                            b = (l == r);
                            break;
                        case LEqual:
                            b = (l <= r);
                            break;
                        case NEqual:
                            b = (l != r);
                            break;
                        case LessT:
                            b = (l < r);
                            break;
                        case GEqual:
                            b = (l >= r);
                            break;
                        case GreatT:
                            b = (l > r);
                            break;
                        default:
                            break;
                        }
                    }
                    else if (cond.attrType == floats) {
                        float l = *(float*)pL;
                        float r = *(float*)pR;
                        switch (cond.compOp)
                        {
                        case EQual:
                            b = (l == r);
                            break;
                        case LEqual:
                            b = (l <= r);
                            break;
                        case NEqual:
                            b = (l != r);
                            break;
                        case LessT:
                            b = (l < r);
                            break;
                        case GEqual:
                            b = (l >= r);
                            break;
                        case GreatT:
                            b = (l > r);
                            break;
                        default:
                            break;
                        }
                    }
                }
                if (b) {
                    rec->rid.bValid = true;
                    rec->rid.pageNum = pn;
                    rec->rid.slotNum = sn;
                    // 内存分配
                    memcpy(rec->pData, record, fileSubHeader->recordSize);
                    rec->bValid = true;
                    ++rmFileScan->sn;
                    return SUCCESS;
                }
            }
        }
        RUN(UnpinPage(&rmFileScan->PageHandle));
        for (++pn; pn <= pageCount; ++pn) {
            if (BM::check(pFileHandle->pBitmap, pn)) {   // 该页已分配
                RUN(GetThisPage(pFileHandle, pn, &rmFileScan->PageHandle));
                sn = 0;
                break;
            }
        }
    }
    return RM_EOF;
}

RC CloseScan(RM_FileScan *rmFileScan)
{
    if (!rmFileScan->bOpen) return RM_FSCLOSED;

    if (rmFileScan->PageHandle.bOpen)
        RUN(UnpinPage(&rmFileScan->PageHandle));

    rmFileScan->bOpen = false;

    return SUCCESS;
}

RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
    if (!fileHandle->bOpen) return RM_FHCLOSED;
    if (!rid->bValid)       return RM_INVALIDRID;

    PF_PageHandle pageHandle;
    RUN(GetThisPage(fileHandle->pFileHandle, rid->pageNum, &pageHandle));

    // 找到数据区
    char* pData;
    RUN(GetData(&pageHandle, &pData));

    RM_FileSubHeader* fileSubHeader = fileHandle->fileSubHeader;
    RM_Page rmPage(pData, fileSubHeader);

    if (!BM::check(rmPage.bitmap, rid->slotNum))    // 从该页位图中查看插槽是否有效
        return RM_INVALIDRID;

    // 将查询到的信息存入rec中
    rec->bValid = true;
    rec->rid.copy(rid);
    // 内存分配？
    memcpy(rec->pData, rmPage.get(rid->slotNum), fileSubHeader->recordSize);

    RUN(UnpinPage(&pageHandle));

	return SUCCESS;
}

RC InsertRec (RM_FileHandle *fileHandle,char *pData, RID *rid)
{
    if (!fileHandle->bOpen) return RM_FHCLOSED;
    RM_FileSubHeader* fileSubHeader = fileHandle->fileSubHeader;
    PF_FileHandle* pFileHandle = fileHandle->pFileHandle;   // 第0页
    PF_PageHandle* pPageHandle = fileHandle->pPageHandle;   // 第1页
    
    PageNum i;  // pageNum
    int j;  // slotNum
    int allocatedPages = pFileHandle->pFileSubHeader->nAllocatedPages - 2;
    PF_PageHandle pageHandle;
    char* data;    // 数据区
    bool newPage = true;
    
    for (i = 2; allocatedPages > 0 && i <= pFileHandle->pFileSubHeader->pageCount; ++i) {
        if (BM::check(pFileHandle->pBitmap, i)) {   // 该页已分配
            if (!BM::check(fileHandle->bitmap, i)) {    // 该页未满
                RUN(GetThisPage(pFileHandle, i, &pageHandle));
                newPage = false;
                break;
            }
            --allocatedPages;
        }
    }
    if (newPage) {
        RUN(AllocatePage(pFileHandle, &pageHandle));
        RUN(GetPageNum(&pageHandle, &i));
    }

    RUN(GetData(&pageHandle, &data));
    RM_Page rmPage(data, fileSubHeader);
    if (newPage) {      // 如果是新页
        *rmPage.nRecords = 0;
        BM::clear(rmPage.bitmap, fileSubHeader->recordsPerPage);
    }

    for (j = 0; j < fileSubHeader->recordsPerPage; ++j) {
        if (!BM::check(rmPage.bitmap, j)) {
            RUN(MarkDirty(&pageHandle));
            RUN(MarkDirty(pPageHandle));
            BM::set(rmPage.bitmap, j);
            ++*rmPage.nRecords;
            ++fileSubHeader->nRecords;
            memcpy(rmPage.get(j), pData, fileSubHeader->recordSize);
            break;
        }
    }

    if (*rmPage.nRecords >= fileSubHeader->recordsPerPage)  // 该页已满
        BM::set(fileHandle->bitmap, i);
    
    RUN(UnpinPage(&pageHandle));

    rid->bValid = true;
    rid->pageNum = i;
    rid->slotNum = j;
    
	return SUCCESS;
}

RC DeleteRec(RM_FileHandle *fileHandle,const RID *rid)
{
    if (!fileHandle->bOpen) return RM_FHCLOSED;
    RM_FileSubHeader* fileSubHeader = fileHandle->fileSubHeader;
    PF_FileHandle* pFileHandle = fileHandle->pFileHandle;   // 第0页
    PF_PageHandle* pPageHandle = fileHandle->pPageHandle;   // 第1页

    if (!rid->bValid)       return RM_INVALIDRID;   // bValid不合法
    if (!BM::check(pFileHandle->pBitmap, rid->pageNum)) return RM_INVALIDRID; // pageNum不合法
    if (rid->slotNum > fileSubHeader->recordsPerPage) return RM_INVALIDRID; // slotNum不合法

    PF_PageHandle pageHandle;   // 记录所在页
    RUN(GetThisPage(pFileHandle, rid->pageNum, &pageHandle));

    // 找到数据区
    char* pData;
    RUN(GetData(&pageHandle, &pData));

    RM_Page rmPage(pData, fileSubHeader);
    if (!BM::check(rmPage.bitmap, rid->slotNum))    // 从该页位图中查看插槽是否有效
        return RM_INVALIDRID;
    
    RUN(MarkDirty(&pageHandle));
    RUN(MarkDirty(pPageHandle));
    if (*rmPage.nRecords == fileSubHeader->recordsPerPage)   // 如果是满页，将变成非满页
        BM::reset(fileHandle->bitmap, rid->pageNum);
    BM::reset(rmPage.bitmap, rid->slotNum);
    --*rmPage.nRecords;
    --fileSubHeader->nRecords;
    if (*rmPage.nRecords == 0)   // 如果该页没有记录，将丢弃该页
        RUN(DisposePage(pFileHandle, rid->pageNum));

    RUN(UnpinPage(&pageHandle));
	return SUCCESS;
}

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
    if (!fileHandle->bOpen) return RM_FHCLOSED;
    RM_FileSubHeader* fileSubHeader = fileHandle->fileSubHeader;
    PF_FileHandle* pFileHandle = fileHandle->pFileHandle;   // 第0页
    PF_PageHandle* pPageHandle = fileHandle->pPageHandle;   // 第1页

    if (!rec->bValid) return RM_INVALIDREC;
    const RID& rid = rec->rid;
    if (!rid.bValid) return RM_INVALIDRID;

    PF_PageHandle pageHandle;   // 记录所在页
    RUN(GetThisPage(pFileHandle, rid.pageNum, &pageHandle));

    // 找到数据区
    char* pData;
    RUN(GetData(&pageHandle, &pData));

    RM_Page rmPage(pData, fileSubHeader);
    if (!BM::check(rmPage.bitmap, rid.slotNum))    // 从该页位图中查看插槽是否有效
        return RM_INVALIDRID;

    RUN(MarkDirty(&pageHandle));
    memcpy(rmPage.get(rid.slotNum), rec->pData, fileSubHeader->recordSize);
    
    RUN(UnpinPage(&pageHandle));

	return SUCCESS;
}

RC RM_CreateFile (char *fileName, int recordSize)
{
    // recordSize大小要合适
    if (recordSize <= 0) return RM_INVALIDRECSIZE;
    int recordsPerPage = (PF_PAGE_SIZE - sizeof(int)) / (recordSize * 8 + 1) * 8;
    if (recordsPerPage == 0) return RM_INVALIDRECSIZE;

    RUN(CreateFile(fileName));

    PF_FileHandle fileHandle;
    RUN(openFile(fileName, &fileHandle));

    PF_PageHandle pageHandle;
    RUN(AllocatePage(&fileHandle, &pageHandle));

    RUN(MarkDirty(&pageHandle));

    char* pData;
    RUN(GetData(&pageHandle, &pData));
    RM_FileSubHeader* fileSubHeader = (RM_FileSubHeader*)pData;
    fileSubHeader->nRecords = 0;
    fileSubHeader->recordSize = recordSize;
    fileSubHeader->recordsPerPage = recordsPerPage;
    fileSubHeader->firstRecordOffset = sizeof(int) + recordsPerPage / 8;

    RUN(UnpinPage(&pageHandle));
    RUN(CloseFile(&fileHandle));

	return SUCCESS;
}

RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
    if (fileHandle->bOpen == true) return RM_FHOPENNED;

    // 打开文件
    PF_FileHandle* pfFileHandle = new PF_FileHandle;
    RUN(openFile(fileName, pfFileHandle));

    // 找到记录信息控制页
    PF_PageHandle* pageHandle = new PF_PageHandle;
    RUN(GetThisPage(pfFileHandle, 1, pageHandle));

    // 找到控制信息
    char* pData;
    RUN(GetData(pageHandle, &pData));

    fileHandle->bOpen = true;
    fileHandle->pFileHandle = pfFileHandle;
    fileHandle->pPageHandle = pageHandle;
    fileHandle->fileSubHeader = (RM_FileSubHeader*)pData;
    fileHandle->bitmap = pData + sizeof(RM_FileSubHeader);

	return SUCCESS;
}

RC RM_CloseFile(RM_FileHandle *fileHandle)
{
    if (fileHandle->bOpen == false) return RM_FHCLOSED;

    RUN(UnpinPage(fileHandle->pPageHandle));
    RUN(CloseFile(fileHandle->pFileHandle));

    fileHandle->bOpen = false;
    delete fileHandle->pPageHandle;
    delete fileHandle->pFileHandle;

	return SUCCESS;
}
