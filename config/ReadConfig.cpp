#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/stat.h> 
#include "ReadConfig.h"
#include "common.h"

#define MAX_VALUE_BUFFER_SIZE 256

int		SConfigPara::m_DebugLog = 0;		//ϵͳ������־
	
string	SConfigPara::m_DBAddr;			//SQL������IP��ַ
int		SConfigPara::m_DBPort;			//SQL������PORT
string	SConfigPara::m_DBUser;			//SQL�������û���
string	SConfigPara::m_DBPassWd;		//SQL����������
string	SConfigPara::m_DBName;			//SQL���������ݿ���

int ReadIniConfig::OpenIniFile(const char *pFile) 
{ 
	struct stat statbuf; 
	stat(pFile,&statbuf); 
	iBufferLen = 0;  
	if((fp=fopen(pFile, "r+")) == NULL)
	{
		printf("fopen file %s failed!\n", pFile);
		return -1; 
	}
	if(fread( szBuffer,statbuf.st_size,1, fp) != 1) 
	{ 
		printf("fread file %s failed!\n", pFile);
		if ( fp != NULL) fclose( fp); 
		 fp = NULL; 
		return -2; 
	} 
	
	 iBufferLen = statbuf.st_size; 
	return 0; 
} 

int ReadIniConfig::CloseIniFile() 
{ 
	if ( fp != NULL) 
	{ 
		if (!fclose( fp)) 
			return 0; 
		else 
			return -1; 
	} 
	else 
		return 0; 
} 

//��ȡ�����ļ�ָ�����ַ���
int ReadIniConfig::GetIniString(const char *pSection,const char *pIdent, char *pResult) 
{ 
	int i=0; 
	int j=0; 
	int min; 
	int iKeyFound=-1; 
	
	if (!strlen(pSection) || !strlen(pIdent) || ( fp == NULL)) 
	{
		return -1; 
	}
	
	while (i< iBufferLen) 
	{ 
		while ((i< iBufferLen) && 
			(( szBuffer[i]==' ') || ( szBuffer[i]=='\t'))) 
			i++; 
		if (i>= iBufferLen) 
			return -1; 
		if ( szBuffer[i]=='#') 
		{ //ignore the lines beginning with '#' 
			while ((i< iBufferLen) && ( szBuffer[i] != '\n')) 
				i++; 
			if (i>= iBufferLen) 
				return -1; 
			//Jump to the next line 
			i++; 
		} 
		else 
		{ 
			if ( szBuffer[i]=='[') 
			{ 
				i++; 
				while ((i< iBufferLen) && 
					(( szBuffer[i]==' ') || ( szBuffer[i]=='\t'))) 
					i++; 
				if (i>= iBufferLen) 
					return -1; 
				if (strncmp( szBuffer+i, pSection, strlen(pSection))==0)
				{ 
				//	printf("pSection = %s,i = %d, szBuffer+i = %s\n", pSection, i, szBuffer+i);
					//Section may be found, let's see 
					i += strlen(pSection); 
					while ((i< iBufferLen) && 
						(( szBuffer[i]==' ') || ( szBuffer[i]=='\t'))) 
						i++; 
					if (i>= iBufferLen) 
						return -1; 
					if ( szBuffer[i]==']') 
						iKeyFound=0; 
					i++; 
					//matched ] or not, ignore the line 
					while ((i< iBufferLen) && ( szBuffer[i]!='\n')) 
						i++; 
					if (i>= iBufferLen) 
						return -1; 
					//Jump to the new line 
					i++; 
				}
				else
				{ //ignore the line and forward 
					while ((i< iBufferLen) && ( szBuffer[i]!='\n')) 
						i++; 
					if (i>= iBufferLen) return -1; 
					//Jump to the next line 
					i++; 
				} 
			} 
			else
			{ 
				if (iKeyFound != 0) { //Section has not found, ignore the line 
					while ((i< iBufferLen) && ( szBuffer[i] != '\n')) i++; 
					if (i>= iBufferLen) return -1; 
					//Jump to the new line 
					i++; 
				}
				else 
				{ //it may be the Identity to be found, judge it 
					if (strncmp( szBuffer+i, pIdent, strlen(pIdent))==0) { 
						i += strlen(pIdent); 
						
					//	printf("pIdent = %s,i = %d, szBuffer+i = %s\n", pIdent, i, szBuffer+i);
						
						if (i>= iBufferLen) return -1; 
						while ((i< iBufferLen) && 
							(( szBuffer[i]=='\t') || ( szBuffer[i]==' '))) i++; 
						if ( szBuffer[i] == '=') 
						{ //Value has found 
							i++; 
							while ((i< iBufferLen) && 
								(( szBuffer[i]=='\t') || ( szBuffer[i]==' '))) i++; 
							if (i>= iBufferLen) return -1; 
							j=i; 
							while ((j< iBufferLen) && 
								( szBuffer[j]!='\n')) j++; j--; 
							while (( szBuffer[j]==' ') || 
								( szBuffer[j]=='\t')) j--; 
							min = j-i+1; 
							strncpy(pResult,  szBuffer+i, min); 
							*(pResult+min) = '\0'; 
							return 0; 
						} else { //ignore the line 
							while ((i< iBufferLen) && ( szBuffer[i]!='\n')) i++; 
							if (i>= iBufferLen) return -1; 
							//Jump to the next line 
							i++; 
						} 

					} 
					else 
					{ //ignore the line 
						while ((i< iBufferLen) && ( szBuffer[i]!='\n')) i++; 
						if (i>= iBufferLen) return -1; 
						//Jump to the next line 
						i++; 
					} 
				} 
			} 
		} 
	} 
	return -1; 
} 

//��ȡ�����ļ��е�������
int ReadIniConfig::GetIniInteger(const char* pSection, const char* pIdent) 
{ 
	char szTempBuffer[MAX_VALUE_BUFFER_SIZE] = {0}; 
	if ( GetIniString(pSection, pIdent, szTempBuffer) == 0 )
	{ 
		if (strlen(szTempBuffer)>2)
		{ 
			if ( (szTempBuffer[0]=='0') && 
				( (szTempBuffer[1]=='x') || (szTempBuffer[1]=='X')) 
				)
				return (int)(strtol(szTempBuffer, (char **)NULL, 16)); 
		} 
		
		return atoi(szTempBuffer); 
	} 
	else 
	{
		printf("GetIniString failed!\n");
		return 0; 
	}
} 


//��ȡ�����ļ��еĳ�������
long ReadIniConfig::GetIniLong(const char* pSection, const char* pIdent) 
{ 
	char szTempBuffer[MAX_VALUE_BUFFER_SIZE] = {0}; 
	if ( GetIniString(pSection, pIdent, szTempBuffer) == 0 ) { 
		if (strlen(szTempBuffer)>2) { 
			if ( (szTempBuffer[0]=='0') && 
				( (szTempBuffer[1]=='x') || (szTempBuffer[1]=='X')) 
				) return (strtol(szTempBuffer, (char **)NULL, 16)); 
		} 
		return atol(szTempBuffer); 
	} else return 0; 
} 

double ReadIniConfig::GetIniDouble(const char* pSection, const char* pIdent) 
{ 
	char szTempBuffer[MAX_VALUE_BUFFER_SIZE]; 
	if ( GetIniString(pSection, pIdent, szTempBuffer) == 0 ) { 
		return atof(szTempBuffer); 
	} else return 0; 
} 

int	ReadIni()
{	
	//��ȡ�����ļ�
	char temp[1024] = {0};

	ReadIniConfig * pMyconfig = new ReadIniConfig();
	
#ifdef WIN32
	pMyconfig->OpenIniFile("EChainConf.ini");
#elif _LINUX_
	pMyconfig->OpenIniFile("EChainConf.ini");
#endif

	SConfigPara::m_DebugLog = pMyconfig->GetIniInteger("Config","DebugLog");
	if(SConfigPara::m_DebugLog < 0)
	{
		printf("m_DebugLog < 0 error!\n");
		return -1;
	}
	
	pMyconfig->GetIniString("MySQL","DBAddr",temp);
	SConfigPara::m_DBAddr = temp;
	if(SConfigPara::m_DBAddr.size() <= 0)
	{
		printf("m_DBAddr error!\n");
		return -1;
	}
	SConfigPara::m_DBPort = pMyconfig->GetIniInteger("MySQL","DBPort");
	if(SConfigPara::m_DBPort < 0)
	{
		printf("m_DBPort < 0 error!\n");
		return -1;
	}
	
	pMyconfig->GetIniString("MySQL","DBUser",temp);
	SConfigPara::m_DBUser = temp;
	if(SConfigPara::m_DBUser.size() <= 0)
	{
		printf("m_DBUser error!\n");
		return -1;
	}
	pMyconfig->GetIniString("MySQL","DBPassWd",temp);
	SConfigPara::m_DBPassWd = temp;
	if(SConfigPara::m_DBPassWd.size() <= 0)
	{
		printf("m_DBPassWd error!\n");
		return -1;
	}
	pMyconfig->GetIniString("MySQL","DBName",temp);
	SConfigPara::m_DBName = temp;
	if(SConfigPara::m_DBName.size() <= 0)
	{
		printf("m_DBName error!\n");
		return -1;
	}
	
//	NBDLog("m_DBAddr = %s, m_DBPort = %d, m_DBUser = %s, m_DBPassWd = %s!\n", SConfigPara::m_DBAddr.c_str(), SConfigPara::m_DBPort, SConfigPara::m_DBUser.c_str(), SConfigPara::m_DBPassWd.c_str());
	pMyconfig->CloseIniFile();
	delete pMyconfig;

	return 1;
}


