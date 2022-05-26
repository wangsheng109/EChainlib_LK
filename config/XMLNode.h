//#include <assert.h>
#include <memory.h>
#include <string>
#include <vector>
#include "common.h"

#define XML_NOTE_BEGIN_TAG "<!-- "
#define XML_NOTE_END_TAG " -->"
#define XML_NOTE_BEGIN_LEN 5
#define XML_NOTE_END_LEN 4

#ifndef __XMLNODE__
#define __XMLNODE__

using namespace std;

string Repeatchar(char v_char,Int32 v_num);

template <class T>
class XMLNodes:public vector<T>
{
public:
	T GetNodeByName(string v_NodeName)
	{
		Int32 count = vector<T>::size();
		for (Int32 i =0;i<count;i++)
		{
			T pNode = vector<T>::at(i);
			if (pNode->GetName() == v_NodeName)
				return pNode;
		}
		return NULL;
	}

	T GetNodeByIndex(int v_Index)
	{
		Int32 count = vector<T>::size();
		if(v_Index >= count)
		{
			return NULL;
		}
		T pNode = vector<T>::at(v_Index);
	//	LOG("iCount = %d, name = %s\n", count, pNode->GetName().c_str());
	
		return pNode;
	}
};

struct Property
{
	string PropertyName;
	string PropertyValue;
};

class XMLDoc;

class XMLNode
{
public:
	friend class XMLDoc;
protected:
	XMLNode(string v_NodeText,Int32 v_Depth);
	XMLNode(string v_Note,string v_Name,string v_Content,Int32 v_Depth);
	~XMLNode();
public:
	//ȡ�ýڵ�����
	string GetContent();
	//���ýڵ�����
	bool SetContent(string v_Content);
	//ȡ�ýڵ�����
	string GetName();
	//���ýڵ�����
	bool SetName(string v_Name);
	//ȡ�ýڵ�ע��
	string GetNote();
	//���ýڵ�ע��
	bool SetNote(string v_Name);
	//ȡ�ýڵ�����
	string GetProperty(string v_PropertyName);
	//���ӽڵ�����
	bool AddProperty(string v_PropertyName,string v_PropertyValue);
	//�޸Ľڵ�����
	bool ModifyProperty(string v_PropertyName,string v_PropertyValue);
	//ɾ���ڵ�����
	bool RemoveProperty(string v_PropertyName);
	//ȡ�ýڵ��ı�
	string GetNodeText();
public:
	//�����ӽڵ�
	XMLNode* AddChildNode(string v_Note,string v_Name,string v_Content);
	//ɾ���ӽڵ�(������)
	bool RemoveChildNode(string v_Name);
	//ɾ���ӽڵ�(������)
	bool RemoveChildNode(Int32 index);
	//ɾ���ӽڵ�(��ָ��)
	bool RemoveChildNode(XMLNode* v_Node);
	//ɾ�������ӽڵ�
	bool RemoveAllChildNode();
	//ȡ�������ӽڵ�
	XMLNodes<XMLNode*> GetChildren();
	//ȡ���ӽڵ�(������)
	XMLNode* GetChild(Int32 index);
	//ȡ���ӽڵ�(������)
	XMLNode* GetChild(string v_NodeName);
protected:
	XMLNodes<XMLNode*> m_Nodes;
	vector<Property> m_Property;
	string m_Note;
	string m_NodeName;
	string m_Content;
	Int32  m_NodeType;
	Int32  m_Depth;
};

#endif

