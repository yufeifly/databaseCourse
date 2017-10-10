#include "stdafx.h"
#include "EditArea.h"
#include "HustBase.h"
#include "HustBaseDoc.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>

char dbPath[MAX_PATH];

void ExecuteAndMessage(char * sql, CEditArea* editArea) {//根据执行的语句类型在界面上显示执行结果。此函数需修改
	std::string s_sql = sql;
	if (s_sql.find("select") == 0) {
		SelResult res;
		Init_Result(&res);
		//rc = Query(sql,&res);
		//将查询结果处理一下，整理成下面这种形式
		//调用editArea->ShowSelResult(col_num,row_num,fields,rows);
		int col_num = 5;
		int row_num = 3;
		char ** fields = new char *[5];
		for (int i = 0; i<col_num; i++) {
			fields[i] = new char[20];
			memset(fields[i], 0, 20);
			fields[i][0] = 'f';
			fields[i][1] = i + '0';
		}
		char *** rows = new char**[row_num];
		for (int i = 0; i<row_num; i++) {
			rows[i] = new char*[col_num];
			for (int j = 0; j<col_num; j++) {
				rows[i][j] = new char[20];
				memset(rows[i][j], 0, 20);
				rows[i][j][0] = 'r';
				rows[i][j][1] = i + '0';
				rows[i][j][2] = '+';
				rows[i][j][3] = j + '0';
			}
		}
		editArea->ShowSelResult(col_num, row_num, fields, rows);
		for (int i = 0; i<5; i++) {
			delete[] fields[i];
		}
		delete[] fields;
		Destory_Result(&res);
		return;
	}
	RC rc = execute(sql);
	int row_num = 0;
	char**messages;
	switch (rc) {
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "操作成功";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "有语法错误";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "功能未实现";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	}
}

RC execute(char * sql) {
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
	rc = parse(sql, sql_str);//只有两种返回结果SUCCESS和SQL_SYNTAX

	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////判断SQL语句为select语句
			//break;

		case 2:
			//判断SQL语句为insert语句
			Insert(sql_str->sstr.ins.relName, sql_str->sstr.ins.nValues, sql_str->sstr.ins.values);
			break;

		case 3:
			//判断SQL语句为update语句
			//AfxMessageBox("update");
			Update(sql_str->sstr.upd.relName, sql_str->sstr.upd.attrName, &sql_str->sstr.upd.value,
				sql_str->sstr.upd.nConditions, sql_str->sstr.upd.conditions);
			break;

		case 4:
			//判断SQL语句为delete语句
			/*AfxMessageBox("delete");*/
			Delete(sql_str->sstr.del.relName , sql_str->sstr.del.nConditions, 
				sql_str->sstr.del.conditions);
			break;

		case 5:
			//判断SQL语句为createTable语句
			CreateTable(sql_str->sstr.cret.relName, sql_str->sstr.cret.attrCount, 
				sql_str->sstr.cret.attributes);
			break;

		case 6:
			//判断SQL语句为dropTable语句
			DropTable(sql_str->sstr.drt.relName);
			break;

		case 7:
			//判断SQL语句为createIndex语句
			break;

		case 8:
			//判断SQL语句为dropIndex语句
			break;

		case 9:
			//判断为help语句，可以给出帮助提示
			AfxMessageBox("Please contact Wang yichuan 123456!");
			break;

		case 10:
			//判断为exit语句，可以由此进行退出操作
			exit(0);
			break;
		}
	}
	else {
		AfxMessageBox(sql_str->sstr.errors);//弹出警告框，sql语句词法解析错误信息
		return rc;
	}
}

RC CreateDB(char *dbpath, char *dbname) {
	//参数正确
	//建立两个系统表
	//dbPath = dbpath;
	CreateDirectory(dbpath, NULL);	//str为路径创建文件夹
	CString cfileNameTabs = dbpath;
	CString cfileNameCols = dbpath;
	cfileNameTabs += _T("\\SYSTABLES.xx");
	cfileNameCols += _T("\\SYSCOLUMNS.xx");
	char *fileNameTabs = cfileNameTabs.GetBuffer(cfileNameTabs.GetLength() + 1);
	char *fileNameCols = cfileNameCols.GetBuffer(cfileNameCols.GetLength() + 1);
	RM_CreateFile(fileNameTabs, sysTabsLength);
	RM_CreateFile(fileNameCols, sysColsLength);
	return SUCCESS;
}

//删除数据库
RC DropDB(char *dbname) {
	char fileP[MAX_PATH];
	sprintf(fileP, "rd %s /s /q", dbname);
	system(fileP);
	//AfxMessageBox(dbname);
	//SetCurrentDirectory("");
	RemoveDirectory(dbname);
	//CloseDB();
	return SUCCESS;
}

//打开数据库
RC OpenDB(char *dbname) {
	strcpy(dbPath, dbname);
	SetCurrentDirectory(dbname);
	/*CString cfileOpenTabs = dbPath;
	CString cfileOpenCols = dbPath;
	cfileOpenTabs += _T("\\SYSTABLES");
	cfileOpenCols += _T("\\SYSCOLUMNS");*/
	
	//RM_OpenFile();
	return SUCCESS;
}

// 关闭数据库
RC CloseDB() {
	//SetCurrentDirectory();
	//刷新树状显示
	CHustBaseDoc *pDoc;
	pDoc = CHustBaseDoc::GetDoc();
	CHustBaseApp::pathvalue = true;
	pDoc->m_pTreeView->PopulateTree();
	return SUCCESS;
}

//
bool CanButtonClick() {//需要重新实现
	//如果当前有数据库已经打开

	return true;
	//如果当前没有数据库打开
	//return false;
}

// ********************** My Functions ***********************

//创建表
RC CreateTable(char *relName, int attrCount, AttrInfo *attributes) {
	int recordSize = 0;
	for (int i = 0; i < attrCount; ++i) {
		recordSize += attributes[i].attrLength;
	}

	RM_CreateFile(relName, recordSize);

	RM_FileHandle fileHandle;
	RID rid;

	char pData[sysTabsLength];
	strcpy(pData, relName);
	*(int*)(pData + nameSize) = attrCount;
	RM_OpenFile("SYSTABLES.xx", &fileHandle);
	InsertRec(&fileHandle, pData, &rid);
	RUN(RM_CloseFile(&fileHandle));

	RM_OpenFile("SYSCOLUMNS.xx", &fileHandle);
	for (int offset = 0, i = 0; i < attrCount; ++i) {
		char pData[sysColsLength];
		char* d = pData;
		strcpy(d, relName);
		d += nameSize;

		strcpy(d, attributes[i].attrName);
		d += nameSize;

		*(int*)d = attributes[i].attrType;
		d += sizeof(int);

		*(int*)d = attributes[i].attrLength;
		d += sizeof(int);

		*(int*)d = offset;
		d += sizeof(int);

		*d = '0';
		d += sizeof(char);

		InsertRec(&fileHandle, pData, &rid);
		offset += attributes[i].attrLength;
	}
	RM_CloseFile(&fileHandle);

	CHustBaseDoc *pDoc;
	pDoc = CHustBaseDoc::GetDoc();
	CHustBaseApp::pathvalue = true;
	pDoc->m_pTreeView->PopulateTree();
	return SUCCESS;
}

//插入记录
RC Insert(char *relName, int nValues, Value * values) {
	RM_FileHandle fileHandle;
	RID rid;

	RUN(RM_OpenFile("SYSCOLUMNS.xx", &fileHandle));
	RM_Record rec(&fileHandle);
	char* pData = new char[fileHandle.fileSubHeader->recordSize];
	RM_FileScan scan;
	Con condition;
	condition.bLhsIsAttr = 1;
	condition.bRhsIsAttr = 0;
	condition.attrType = chars;
	condition.LattrLength = condition.RattrLength = nameSize;
	condition.LattrOffset = 0;
	condition.compOp = EQual;
	condition.Rvalue = relName;
	RUN(OpenScan(&scan, &fileHandle, 1, &condition));
	while (GetNextRec(&scan, &rec) == SUCCESS) {
		Value* value = values + (--nValues);
		int type = *(int*)(rec.pData + 2 * nameSize);
		int length = *(int*)(rec.pData + 2 * nameSize + sizeof(int));
		int offset = *(int*)(rec.pData + 2 * nameSize + 2 * sizeof(int));
		if (type == chars) {
			memcpy(pData + offset, value->data, length);
		}
		else if (type == ints) {
			if (value->type == ints)
				*(int*)(pData + offset) = *(int*)value->data;
			else if (value->type == floats)
				*(int*)(pData + offset) = *(float*)value->data;
		}
		else if (type == floats) {
			if (value->type == ints)
				*(float*)(pData + offset) = *(int*)value->data;
			else if (value->type == floats)
				*(float*)(pData + offset) = *(float*)value->data;
		}
	}
	CloseScan(&scan);
	RM_CloseFile(&fileHandle);

	RUN(RM_OpenFile(relName, &fileHandle));
	InsertRec(&fileHandle, pData, &rid);
	RUN(RM_CloseFile(&fileHandle));

	delete[] pData;
	// 刷新table显示
	CHustBaseDoc *pDoc;
	pDoc = CHustBaseDoc::GetDoc();
	CHustBaseApp::pathvalue = true;
	pDoc->m_pListView->displayTabInfo(relName);//tabname是该表的表名

	return SUCCESS;
}

// 删除记录
RC Delete(char *relName, int nConditions, Condition *conditions) {
	RM_FileHandle fileHandle;
	RID rid;
	RM_FileScan scan;
	
	RUN(RM_OpenFile("SYSCOLUMNS.xx", &fileHandle));
	RM_Record rec(&fileHandle);

	Con condition[2];
	Con cond[8];
	for (int i = 0; i < nConditions; ++i) {
		condition[0].bLhsIsAttr = 1;//左边是属性
		condition[0].bRhsIsAttr = 0;
		condition[0].attrType = chars;
		condition[0].LattrLength = condition[0].RattrLength = nameSize; // 属性长度
		condition[0].LattrOffset = 0;	//属性偏移量
		condition[0].compOp = EQual;
		condition[0].Rvalue = relName;
		//
		condition[1].bLhsIsAttr = 1;//左边是属性
		condition[1].bRhsIsAttr = 0;
		condition[1].attrType = chars;
		condition[1].LattrLength = condition[1].RattrLength = nameSize; // 属性长度
		condition[1].LattrOffset = 0 + nameSize;	//属性偏移量
		condition[1].compOp = EQual;
		if (conditions[i].bLhsIsAttr == 1) 
			condition[1].Rvalue = conditions[i].lhsAttr.attrName;
		else if (conditions[i].bRhsIsAttr == 1) 
			condition[1].Rvalue = conditions[i].rhsAttr.attrName;
	//
		RUN(OpenScan(&scan, &fileHandle, 2, condition));
		if(GetNextRec(&scan, &rec) == SUCCESS){
			if (conditions[i].bLhsIsAttr == 1) {
				cond[i].bLhsIsAttr = 1;
				cond[i].bRhsIsAttr = 0;
			}
			else if (conditions[i].bRhsIsAttr == 1){
				cond[i].bLhsIsAttr = 0;
				cond[i].bRhsIsAttr = 1;
			}
			cond[i].attrType = (AttrType)*(int*)(rec.pData+2*nameSize);
			cond[i].LattrLength = cond[i].RattrLength = *(int*)(rec.pData + 2 * nameSize + sizeof(int));
			cond[i].LattrOffset = cond[i].RattrOffset = *(int*)(rec.pData + 2 * nameSize + 2*sizeof(int));
			cond[i].compOp = conditions[i].op;
			cond[i].Rvalue = conditions[i].rhsValue.data;
			cond[i].Lvalue = conditions[i].lhsValue.data;
		}
		RUN(CloseScan(&scan));
	}
	RUN(RM_CloseFile(&fileHandle));
	//
	RUN(RM_OpenFile(relName, &fileHandle));
	RM_Record rec2(&fileHandle);

	RUN(OpenScan(&scan, &fileHandle, nConditions, cond));

	while (GetNextRec(&scan, &rec2) == SUCCESS) {
		DeleteRec(&fileHandle, &rec2.rid);
	}
	RUN(CloseScan(&scan));
	RUN(RM_CloseFile(&fileHandle));

	// 刷新table显示
	CHustBaseDoc *pDoc;
	pDoc = CHustBaseDoc::GetDoc();
	CHustBaseApp::pathvalue = true;
	pDoc->m_pListView->displayTabInfo(relName);//tabname是该表的表名

	return SUCCESS;
}

//更新记录
RC Update(char *relName, char *attrName, Value *value, int nConditions, Condition *conditions) {
	RM_FileHandle fileHandle;
	RID rid;
	RM_FileScan scan;

	int offset,type, length;

	RUN(RM_OpenFile("SYSCOLUMNS.xx", &fileHandle));
	RM_Record rec(&fileHandle);
	Con condition[2];
	Con cond[8];
	for (int i = 0; i < nConditions; ++i) {
		condition[0].bLhsIsAttr = 1;//左边是属性
		condition[0].bRhsIsAttr = 0;
		condition[0].attrType = chars;
		condition[0].LattrLength = condition[0].RattrLength = nameSize; // 属性长度
		condition[0].LattrOffset = 0;	//属性偏移量
		condition[0].compOp = EQual;
		condition[0].Rvalue = relName;
		//
		condition[1].bLhsIsAttr = 1;//左边是属性
		condition[1].bRhsIsAttr = 0;
		condition[1].attrType = chars;
		condition[1].LattrLength = condition[1].RattrLength = nameSize; // 属性长度
		condition[1].LattrOffset = 0 + nameSize;	//属性偏移量
		condition[1].compOp = EQual;
		if (conditions[i].bLhsIsAttr == 1) 
			condition[1].Rvalue = conditions[i].lhsAttr.attrName;
		else if (conditions[i].bRhsIsAttr == 1) 
			condition[1].Rvalue = conditions[i].rhsAttr.attrName;
		//
		RUN(OpenScan(&scan, &fileHandle, 2, condition));
		if (GetNextRec(&scan, &rec) == SUCCESS) {
			if (conditions[i].bLhsIsAttr == 1) {
				cond[i].bLhsIsAttr = 1;
				cond[i].bRhsIsAttr = 0;
			}
			else if (conditions[i].bRhsIsAttr == 1) {
				cond[i].bLhsIsAttr = 0;
				cond[i].bRhsIsAttr = 1;
			}
			cond[i].attrType = (AttrType)*(int*)(rec.pData + 2 * nameSize);
			cond[i].LattrLength = cond[i].RattrLength = *(int*)(rec.pData + 2 * nameSize + sizeof(int));
			cond[i].LattrOffset = cond[i].RattrOffset = *(int*)(rec.pData + 2 * nameSize + 2 * sizeof(int));
			cond[i].compOp = conditions[i].op;
			cond[i].Rvalue = conditions[i].rhsValue.data;
			cond[i].Lvalue = conditions[i].lhsValue.data;
		}
		RUN(CloseScan(&scan));
	}
	//再次查询系统表，获取attrName的相关信息
	condition[1].bLhsIsAttr = 1;//左边是属性
	condition[1].bRhsIsAttr = 0;
	condition[1].attrType = chars;
	condition[1].LattrLength = condition[1].RattrLength = nameSize; // 属性长度
	condition[1].LattrOffset = 0 + nameSize;	//属性偏移量
	condition[1].compOp = EQual;
	condition[1].Rvalue = attrName;

	RUN(OpenScan(&scan, &fileHandle, 2, condition));
	if (GetNextRec(&scan, &rec) == SUCCESS) {
		type = *(int*)(rec.pData + 2 * nameSize);
		length = *(int*)(rec.pData + 2 * nameSize + sizeof(int));
		offset = *(int*)(rec.pData + 2 * nameSize + 2 * sizeof(int));
	}
	RUN(CloseScan(&scan));
	RUN(RM_CloseFile(&fileHandle)); //关闭系统表
	// 打开记录表
	RUN(RM_OpenFile(relName, &fileHandle));
	RM_Record rec2(&fileHandle);

	RUN(OpenScan(&scan, &fileHandle, nConditions, cond));

	while (GetNextRec(&scan, &rec2) == SUCCESS) {
		//这里需要修改rec2的pdata
		if (type == chars) {
			memcpy(rec2.pData + offset, value->data, length);
		}
		else if (type == ints) {
			if (value->type == ints)
				*(int*)(rec2.pData + offset) = *(int*)value->data;
			else if (value->type == floats)
				*(int*)(rec2.pData + offset) = *(float*)value->data;
		}
		else if (type == floats) {
			if (value->type == ints)
				*(float*)(rec2.pData + offset) = *(int*)value->data;
			else if (value->type == floats)
				*(float*)(rec2.pData + offset) = *(float*)value->data;
		}
		//调用更新记录函数
		UpdateRec(&fileHandle, &rec2);
	}
	RUN(CloseScan(&scan));
	RUN(RM_CloseFile(&fileHandle));

	// 刷新table显示
	CHustBaseDoc *pDoc;
	pDoc = CHustBaseDoc::GetDoc();
	CHustBaseApp::pathvalue = true;
	pDoc->m_pListView->displayTabInfo(relName);//tabname是该表的表名

	return SUCCESS;
}

//删除表
RC DropTable(char *relName) {
	RM_FileHandle fileHandle;
	RID rid;
	RM_FileScan scan;

	Con condition;
	condition.bLhsIsAttr = 1;//左边是属性
	condition.bRhsIsAttr = 0;
	condition.attrType = chars;
	condition.LattrLength = condition.RattrLength = nameSize; // 属性长度
	condition.LattrOffset = 0;	//属性偏移量
	condition.compOp = EQual;
	condition.Rvalue = relName;

	//删除系统表SYSTABLES相关内容
	RUN(RM_OpenFile("SYSTABLES.xx", &fileHandle));
	RM_Record rec(&fileHandle);
	RUN(OpenScan(&scan, &fileHandle, 1, &condition));
	if (GetNextRec(&scan, &rec) == SUCCESS) {
		DeleteRec(&fileHandle, &rec.rid);
	}
	RUN(CloseScan(&scan));
	RUN(RM_CloseFile(&fileHandle));
	//删除系统表SYSCOLUMNS的相关内容
	RUN(RM_OpenFile("SYSCOLUMNS.xx", &fileHandle));
	RM_Record rec2(&fileHandle);
	RUN(OpenScan(&scan, &fileHandle, 1, &condition));
	while (GetNextRec(&scan, &rec2) == SUCCESS) {
		DeleteRec(&fileHandle, &rec2.rid);
	}
	RUN(CloseScan(&scan));
	RUN(RM_CloseFile(&fileHandle));
	//删除表文件
	char fileP[MAX_PATH];
	sprintf(fileP, "del %s\\%s",dbPath,relName);
	//AfxMessageBox(fileP);
	system(fileP);
	//刷新
	CHustBaseDoc *pDoc;
	pDoc = CHustBaseDoc::GetDoc();
	CHustBaseApp::pathvalue = true;
	pDoc->m_pTreeView->PopulateTree();

	return SUCCESS;
}
