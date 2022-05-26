#ifndef __MYSQL_EXEC_API_H__
#define __MYSQL_EXEC_API_H__

#ifdef WIN32
	#include <windows.h>
#else
	#define HANDLE void*
	#define BOOL   int	
#endif

typedef struct tagDatabase_Param//���ݿ����
{
	char host[128];				//������
	char user[128];				//�û���
	char password[128];			//����
	char db_name[128];			//���ݿ���
	char table_name[128];		//����
	unsigned int port;			//�˿ڣ�һ��Ϊ0
	const char *unix_socket;	 //�׽��֣�һ��ΪNULL
	unsigned int client_flag;	 //һ��Ϊ0
	char filed_file_path[128];
	char filed_file_time[128];
}Database_Param,*LPDatabase_Param;




/*************************************************/
HANDLE			mysql_connect(Database_Param *p);			//������mysql ������������
int				mysql_disconnect(HANDLE &hObj);				//�Ͽ���mysql ������������

int			mysql_use_db(HANDLE hObj,char* dbname);	//ѡ��mysql ���ݿ�

//mysql_open_tbl(m_db,"select * from mp4test;");			//�����ݿ��е�һ����
int				mysql_open_tbl(HANDLE hObj,const char* sqlfmt, ...);
int				mysql_close_db(HANDLE hObj);	//�ر����ݿ��е�һ����
int				mysql_next(HANDLE hObj);					//ȡ����һ����¼
int			mysql_iseof(HANDLE hObj);					//�Ƿ��˼�¼�Ľ�β

//mysql_exec_sql(m_db,"sql ���");							//ִ��sql ���
int				mysql_exec_sql(HANDLE hObj,const char* sqlfmt, ...);

//**************************************
unsigned int	mysql_get_cols(HANDLE hObj);				//ȡ��������
unsigned long	mysql_get_rows(HANDLE hObj);				//ȡ��������
char*			mysql_get_col_name(HANDLE hObj,int idx);	//ȡ�е�����
unsigned long	mysql_get_col_len(HANDLE hObj,int idx);		//ȡ�еĳ���

//����id�������������ڻ�ʱ��			date:20071118		time:143026
int				mysql_get_datetime_id(HANDLE hObj,int idx,int& year,int& month, int& day);
//int				mysql_get_datetme(HANDLE hObj,char* col_name,long& date,long& time);
int				mysql_get_datetme(HANDLE hObj,char* col_name, int& year,int& month, int& day);

char			mysql_get_char_id(HANDLE hObj,int idx);		//������idȡһ���ַ�
char			mysql_get_char(HANDLE hObj,char* col_name);	//��������ȡһ���ַ�

int			mysql_get_int_id(HANDLE hObj,int idx,int& value);	//������idȡһ������
int			mysql_get_int(HANDLE hObj,char* col_name,int& value);		//��������ȡһ������

int			mysql_get_long_id(HANDLE hObj,int idx,long& value);
int			mysql_get_long(HANDLE hObj,char* col_name,long& value);

int			mysql_get_float_id(HANDLE hObj,int idx,float& value);
int			mysql_get_float(HANDLE hObj,char* col_name,float& value);

int			mysql_get_double_id(HANDLE hObj,int idx,double& value);
int			mysql_get_double(HANDLE hObj,char* col_name,double& value);

char*			mysql_get_string_id(HANDLE hObj,int idx);						//������idȡ���ַ���
char*			mysql_get_string(HANDLE hObj,char* col_name);					//��������ȡ���ַ���

#endif  //__MYSQL_EXEC_API_H__
