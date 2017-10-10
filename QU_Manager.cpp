#include "StdAfx.h"
#include "QU_Manager.h"
#include "RM_Manager.h"

void Init_Result(SelResult * res){
	res->next_res = NULL;
}

void Destory_Result(SelResult * res){
	for(int i = 0;i<res->row_num;i++){
		for(int j = 0;j<res->col_num;j++){
			delete[] res->res[i][j];
		}
		delete[] res->res[i];
	}
	if(res->next_res != NULL){
		Destory_Result(res->next_res);
	}
}

//��ѯ����
/*RC Query(char * sql, SelResult * res) {
	//���Ƚ���sql
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
	rc = parse(sql, sql_str);//ֻ�����ַ��ؽ��SUCCESS��SQL_SYNTAX
	if (rc == SUCCESS) {
		char *fileName = sql_str->sstr.cret.relName;
		RM_FileHandle *fileHandle = new RM_FileHandle();
		RM_OpenFile(fileName, fileHandle);//	���ļ�

		// ������Ҫд��һЩ����
		RM_FileScan *rmFileScan = new  RM_FileScan();
		int conNum = sql_str->sstr.sel.nConditions;
		
		Condition *tmpCondition = sql_str->sstr.sel.conditions;
		Con *conditions = new Con[conNum];
		for (int i = 0; i < conNum; ++i) {
			if (tmpCondition[i].bLhsIsAttr == 1) {//
				conditions[i].bLhsIsAttr = 1;
				conditions[i].bRhsIsAttr = 0;
				conditions[i].attrType = tmpCondition[i].lhsValue.type;
				conditions[i].Lvalue = tmpCondition[i].lhsValue.data;
				conditions[i].Rvalue = NULL;
				conditions[i].compOp = tmpCondition[i].op;
				conditions[i].LattrLength = ;
				conditions[i].RattrLength = 0;
			}
		}

		OpenScan(rmFileScan,fileHandle, conNum, conditions);


		RM_CloseFile(fileHandle);//		�ر��ļ�
		//RM_OpenFile(char *fileName, RM_FileHandle *fileHandle);
	} else {
		AfxMessageBox(sql_str->sstr.errors);//���������sql���ʷ�����������Ϣ
		return rc;
	}
}*/