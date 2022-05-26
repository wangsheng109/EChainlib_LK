
#include <stdarg.h>
#include <stdlib.h>

#include <mysql.h>
#include "MySQLDataBase.h"

CDatabase::CDatabase()
{
	query = 0;    //�����
	row   = 0;    //��¼��
	field = 0;    //�ֶ���Ϣ���ṹ�壩
}

CDatabase::~CDatabase()
{
	Disconnect();
}

int CDatabase::ConnectDB(Database_Param *p)
{
//	DBmutex.Lock();
	mysql_init(&mysql);
	if (!mysql_real_connect(&mysql,p->host,p->user,p->password,p->db_name,p->port,p->unix_socket,p->client_flag))
	{
		OutErrors();
//		DBmutex.Unlock();
		return 0;
	}
	mysql_query(&mysql,"SET NAMES UTF8");
//	DBmutex.Unlock();
	if (!mysql_set_character_set(&mysql, "utf8")) 
	{
	//    printf("New client character set: %s\n", mysql_character_set_name(&mysql));
		return 1;
	}
	else
	{
		printf("New client character set failed!\n");
		return 0;
	}
	
	return 1;
}
 
int CDatabase::Disconnect()
{
//	DBmutex.Lock();
	if (query != 0 )
		mysql_free_result(query);
	query = 0;
	mysql_close(&mysql);
//	DBmutex.Unlock();
	return 1;
}

int CDatabase::SelectDB(char * DBName)
{
	if (mysql_select_db(&mysql,DBName))
	  return 0; 
	else
	  return 1;
}

const char * CDatabase::OutErrors()
{
	return(mysql_error(&mysql));
}


int CDatabase::FindSave(char *str)
{
	if(mysql_query(&mysql,str))
	  return 0;
	query=mysql_store_result(&mysql);
	  return 1;
}

unsigned long CDatabase::GetRowNum()
{
   return (mysql_num_rows(query));
}


int CDatabase::MoveToNextRecord()
{
	if((row = mysql_fetch_row(query)) != 0)
	{
		return 1;
	}
	else
		return 0;
}

unsigned int CDatabase::GetColumnNum()
{
	return (mysql_num_fields(query));
}


void CDatabase::FreeRecord()
{
	if (query != 0 )
		mysql_free_result(query);
	query = 0;
}

int CDatabase::IsEnd()
{
	return mysql_eof(query);
}


char* CDatabase::GetColumnName(int ColumnNum)
{
	field=mysql_fetch_field_direct(query,ColumnNum);
	return field->name;
}

unsigned long CDatabase::GetColumnLength(int ColumnNum)
{
	field=mysql_fetch_field_direct(query,(unsigned int)ColumnNum);
	return field->length;
}
char CDatabase::GetValue_Char(int Column)
{
	if(Column > (int)mysql_num_fields(query) )
		return 0;
		
	return (char)row[Column][0];
}

char CDatabase::GetValue_Char(char* Column)
{
	char NameBuf[256]={0};
	int ColNum,FindNum,i;
	ColNum = (int)mysql_num_fields(query);
	for(i=0;i< ColNum;i++)
	{
		strcpy(NameBuf,GetColumnName(i));
		if(strcmp(Column,NameBuf) == 0)
		{
			FindNum = i;
			break;
		}
	}
	if(i >= ColNum)
		return 0;		
	return (char)row[FindNum][0];
}

char* CDatabase::GetValue_String(int Column)
{
	if(Column > (int)mysql_num_fields(query) )
		return 0;		
	return row[Column];
}

char* CDatabase::GetValue_String(char* Column)
{
	int ColNum;
	ColNum = CheckColumn(Column);
	if(ColNum >= 0 )
		return (char *)row[ColNum];
	else
		return 0;
}

int CDatabase::GetValue_Int(int Column,int& Value)
{
	if(Column > (int)mysql_num_fields(query) )
	{
		return 0;
	}
	Value = atoi(row[Column]);
		return 1;
}
	
int CDatabase::GetValue_Int(char* Column,int& Value)
{
	int ColNum;
	ColNum = CheckColumn(Column);
	if(ColNum >= 0 )
	{
		Value = atoi(row[ColNum]);
		return 1;
	}
	else
		return 0;
}
		
int CDatabase::GetValue_Long(int Column,long& Value)
{
	char *endptr;
	if(Column > (int)mysql_num_fields(query) )
		return 0;
	Value = strtol(row[Column],&endptr,10);
		return 1;
}
	
int CDatabase::GetValue_Long(char* Column,long& Value)
{
	char *endptr;
	int ColNum;
	ColNum = CheckColumn(Column);
	if(ColNum >= 0 )
	{
		Value = strtol(row[ColNum],&endptr,10);
		return 1;
	}
	else
		return 0;
}
	
int CDatabase::GetValue_Float(int Column,float& Value)
{
	if(Column > (int)mysql_num_fields(query) )
		return 0;
	Value = atof(row[Column]);
		return 1;
}
	
int CDatabase::GetValue_Float(char* Column,float& Value)
{
	int ColNum;
	ColNum = CheckColumn(Column);
	if(ColNum >= 0 )
	{
		Value = atof(row[ColNum]);
		return 1;
	}
	return 0;
}
	
int CDatabase::GetValue_Double(int Column,double& Value)
{
	if(Column > (int)mysql_num_fields(query) )
		return 0;
	Value = (double)atof(row[Column]);
		return 1;
}
	
int CDatabase::GetValue_Double(char* Column,double& Value)
{
	int ColNum;
	ColNum = CheckColumn(Column);
	if(ColNum >= 0 )
	{
		Value = (double)atof(row[ColNum]);
		return 1;
	}
	return 0;
}
	
int CDatabase::CheckColumn(char* Column)
{
	char NameBuf[256]={0};
	int ColNum,i;
	ColNum = (int)mysql_num_fields(query);
	for(i=0;i< ColNum;i++)
	{
		if(GetColumnName(i) == NULL)
			return -1;
		memset(NameBuf, 0, 256);
	//	strcpy(NameBuf,GetColumnName(i));
		memcpy(NameBuf,GetColumnName(i), 256);
		
		if(strcmp(Column,NameBuf) == 0)
		{
			return i;
		}
	}
	return -1;
}

int CDatabase::Exec(const char *SQLfmt, ...)
{
	char buff[4096] = {0};
	va_list vl;
	va_start(vl, SQLfmt);
	vsprintf((char *)buff, SQLfmt, vl);
	va_end(vl);
	if(mysql_query(&mysql,buff))
	{
		int no = mysql_errno(&mysql);
		return no;
	}else
	{
		return 1;;
	}

}

int CDatabase::Open_Table(const char *SQLfmt, ...)
{
	char buff[4096] = {0};
	va_list vl;

//	DBmutex.Lock();
	
	va_start(vl, SQLfmt);
	vsprintf((char *)buff, SQLfmt, vl);
	va_end(vl);
	if(mysql_query(&mysql,buff))
	{
	//	DBmutex.Unlock();
		return 0;
	}
	query=mysql_store_result(&mysql);
	if((row = mysql_fetch_row(query)) != 0)
	{
	//	DBmutex.Unlock();
		return 1;
	}else
	{
	//	DBmutex.Unlock();
		return 0;
	}
//	DBmutex.Unlock();
	return 1;
}

