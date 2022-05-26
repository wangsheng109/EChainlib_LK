#ifndef MYSQLDATABASE_H
#define MYSQLDATABASE_H 

#include   <stdio.h>
#include   <string.h>
#include   <mysql.h>
#ifdef WIN32
	#include <windows.h>
	#include   <winsock.h>
#endif

#include "MySQLExecAPI.h"

class CDatabase  
{
	public:
		unsigned int GetColumnNum();				//�õ�����
		unsigned long GetRowNum();				//�õ�����
		char* GetColumnName(int ColumnNum);		//�õ�����
		unsigned long GetColumnLength(int ColumnNum);
		int SelectDB(char* DBName);		//ѡ�����ݿ�
		int ConnectDB(Database_Param *p);		//�������ݿ�
		int Disconnect();
		int GetAllFromTable(char* TableName);		//ѡ�����м�¼
		int Exec(const char* SQLfmt, ...);
		int Open_Table(const char* SQLfmt, ...);		
		char GetValue_Char(int Column);
		char GetValue_Char(char* Column);
		int GetValue_Int(int Column,int& Value);
		int GetValue_Int(char* Column,int& Value);
		int GetValue_UnsignedInt(int Column,unsigned int& Value);
		int GetValue_UnsignedInt(char* Column,unsigned int& Value);
		int GetValue_Long(int Column,long& Value);
		int GetValue_Long(char* Column,long& Value);
		int GetValue_Float(int Column,float& Value);
		int GetValue_Float(char* Column,float& Value);
		int GetValue_Double(int Column,double& Value);
		int GetValue_Double(char* Column,double& Value);
		char* GetValue_String(int Column);
		char* GetValue_String(char* Column);		
		int MoveToNextRecord();
		void FreeRecord();						//�ͷŽ����			
		int IsEnd();							//�Ƿ����
		const char * OutErrors();				//���������Ϣ		
		CDatabase();
		virtual ~CDatabase();

	private:
		MYSQL mysql;            //���ݿ����Ӿ��
		MYSQL_RES *query;       //�����
		MYSQL_ROW row;          //��¼��
		MYSQL_FIELD *field;     //�ֶ���Ϣ���ṹ�壩
		int FindSave(char *str);    //���Ҳ���������
		int CheckColumn(char* Column);
};

#endif //MYSQLDATABASE_H
