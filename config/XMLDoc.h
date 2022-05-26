#include <stdio.h>
#include <string>

#include "XMLNode.h"

#define XML_HEADLINE "<?xml version=\"1.0\" encoding=\"GBK\"?>\n"

#ifndef __XMLDOC__
#define __XMLDOC__

using namespace std;

//����XML�ĵ�
XMLDoc* CreateXMLDoc(const char* v_FileName,const char* v_RootNodeName);

class XMLDoc
{
public:
	XMLDoc(const char* v_FileName);
	XMLDoc();
    // ���������Զ�ɾ�����ڵ㣬ÿ���ڵ�����������ֻ��Զ�ɾ���Լ����ӽڵ�
	~XMLDoc();
public:
	//��XML�ĵ�
	bool LoadXMLDoc();
	//
	bool LoadXML(const char *v_str);

	//ȡ�ø��ڵ�
	XMLNode* GetRootNode(const char* v_Name);
	//�����ļ�
	bool SaveFile();
private:
	XMLNode* m_RootNode;
	string m_Buffer;
	string m_FileName;
	bool   m_Useable;
};

#endif

