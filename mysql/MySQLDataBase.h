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
		unsigned int GetColumnNum();				//得到列数
		unsigned long GetRowNum();				//得到行数
		char* GetColumnName(int ColumnNum);		//得到列名
		unsigned long GetColumnLength(int ColumnNum);
		int SelectDB(char* DBName);		//选择数据库
		int ConnectDB(Database_Param *p);		//连接数据库
		int Disconnect();
		int GetAllFromTable(char* TableName);		//选择所有记录
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
		void FreeRecord();						//释放结果集			
		int IsEnd();							//是否最后
		const char * OutErrors();				//输出错误信息		
		CDatabase();
		virtual ~CDatabase();

	private:
		MYSQL mysql;            //数据库连接句柄
		MYSQL_RES *query;       //结果集
		MYSQL_ROW row;          //记录集
		MYSQL_FIELD *field;     //字段信息（结构体）
		int FindSave(char *str);    //查找并保存结果集
		int CheckColumn(char* Column);
};

#endif //MYSQLDATABASE_H

