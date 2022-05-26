#ifndef __READCONFIG_H__
#define __READCONFIG_H__

#include <string>
#include <list>


#define MAX_BUFFER_SIZE 6000 
using namespace std;


struct SConfigPara
{
	static int				m_DebugLog;		//ϵͳ������־
	
	static string			m_DBAddr;		//SQL������IP��ַ
	static int				m_DBPort;		//SQL������PORT
	static string			m_DBUser;		//SQL�������û���
	static string			m_DBPassWd;	//SQL����������
	static string			m_DBName;		//SQL���������ݿ���
};


#define MAX_BUFFER_SIZE 6000 
using namespace std;
class ReadIniConfig
{
public:
	ReadIniConfig(){}
	virtual ~ReadIniConfig(){}
	int OpenIniFile(const char *pFile);
	int CloseIniFile();
	int GetIniString(const char *pSection,const char *pIdent, char *pResult);
	int GetIniInteger(const char* pSection, const char* pIdent);
	int GetMulIniInteger(const char* pSection, const char* pIdent); 
	long GetIniLong(const char* pSection, const char* pIdent);
	double GetIniDouble(const char* pSection, const char* pIdent);
private:
   char szBuffer[MAX_BUFFER_SIZE]; 
   int iBufferLen; 
   FILE *fp;	
};

int	ReadIni();

#endif

