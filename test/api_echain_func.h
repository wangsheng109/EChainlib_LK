#ifndef API_EChain_FUN_FOR_YINIU_H
#define API_EChain_FUN_FOR_YINIU_H

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <string.h>

void NBDLog(const char *fmt, ...);


/*
�������ܣ���ʼ������
ʹ��ǰ���������һ�θú���
�� �� ֵ��>0 �ɹ���  <0 ʧ��
*/
int EChain_InitSDK();

	
/*	
�������ܣ���ѯϵͳ������Ϣ
�� �� ֵ��<0,��ѯʧ��  >0 ��ѯ�ɹ�
�����������
���������
			jsonInfo��Ϊ��ѯ��������Json��Ϣ������ϵͳ����߶ȡ��ڵ������û�������������ƽ��ʱ�䣬����ʱ���
*/
int EChain_QueryBlockNewestInfo(std::string &jsonInfo);

	
/*	
�������ܣ������ʲ���ϵͳ�������ã������ϲ�����
�� �� ֵ��<0,�����ʲ�ʧ��  >0 �����ʲ��ɹ��� �½��µ��ʲ�ʱ����
���������
					cliendID:��ҵע��ID��
					issuerAddress:�ʲ����з�
					hexInfo:�ʲ�metadata��Ϣ
					assetAmount�������ʲ���������
					assetCode�����е�token��������
���������	
					transferHash:����Hash
*/
int EChain_IssueAssets(std::string cliendID, std::string issuerAddress, std::string hexInfo, std::string assetCode, int32_t assetAmount, std::string &transferHash);


/*	
�������ܣ�����ʵʱ�����˺ţ�
�� �� ֵ��<0,�����˺�ʧ��  >0 �����˺ųɹ�
�����������
����������´������˺������ַ
*/
int EChain_CreateAccount(std::string &newAddress);


/*	
�������ܣ������˺ţ�
�� �� ֵ��<0,�����˺�ʧ��  >0 �����˺ųɹ�
�����������
�����������
*/
int EChain_CreateAccountX();


/*	
�������ܣ���ǰ׼�����˺ţ�����100��
�� �� ֵ��<0,׼���˺�ʧ��  >0 ׼���˺ųɹ�
�����������
�����������
*/
int EChain_AccountPrepare();

	
/*
�������ܣ���ȡ����
�� �� ֵ��<0,��ȡʧ��  >0 ��ȡ�ɹ�
���������destAddress������ȡ���ҵ������ַ��assetCode����������, assetAmount,��������
�����������
*/
int EChain_ObtainAssetCode(std::string destAddress, std::string assetCode, int assetAmount);


	
/*	
�������ܣ��û�ע�Ტ�����������˺�
�� �� ֵ��result: �������ֵ
				  1:��ȷ
				  0:userInfo�����Ѿ�ע��
				  -1:���ݿ����
���������		  userID:  �û�ID��
				  userType:�û�����
				  userInfo:  �û������Ϣ
���������		  blockAddress: ���������������ַ
				  privateKey:�û�˽Կ
				  publicKey: �û���Կ
				  transHash: ���������ϣֵ		  
*/
int EChain_UserRegister(std::string userID, int userType, std::string userInfo, std::string &blockAddress, std::string &privateKey, std::string &publicKey, std::string &transHash);


	
/*	
�������ܣ��û�ʵ����֤
�� �� ֵ���������ֵ
				  1:��ȷ
				  0:�������ݴ���
				  -1:���ݿ����
���������
		blockAddress: �û�ע�������ַ
		userName: �û�����
		certificationInfo: �û���֤�����Ϣ��json��ʽ, ����
					{ "BaseInfo":{"CompanyName":"������������ѯ��Ϣ���޹�˾","CompanyAddress":"������������" ,"Telephone":"0755-32134897"," Contacter":"����","Certificate":"914492903888272e3f" }}
���������transactionHash: ���׼�¼��ϣֵ
*/
int EChain_UserCertification(std::string blockAddress, std::string userName, std::string certificationInfo, std::string &transactionHash);


		  
/*  
�������ܣ����ݽ���Hash��ѯ�ý�������û�ʵ����֤��Ϣ
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
���������transactionHash: ����Hash
���������certificationInfo:�û���֤��Ϣ
*/
int EChain_QueryCertificationInfo(std::string transactionHash, std::string &certificationInfo);



/*	
�������ܣ�������Լ�˺�
�� �� ֵ��result: �������ֵ
				  1:��ȷ
				  0:userInfo�����Ѿ�ע��
				  -1:���ݿ����
���������    userID:  �û�ID��
				  classID:  �û���������ʵ��ID��Ϣ
���������	  contractAddress: �û���������ʵ��ID��Ӧ���ɵĺ�Լ�˺�
				  privateKey:�û�˽Կ
				  publicKey: �û���Կ
				  transHash: ���������ϣֵ				  
*/
int EChain_ContractAddressCreate(std::string userID, std::string classID, std::string &contractAddress, std::string &privateKey, std::string &publicKey, std::string &transHash);


/*	
�������ܣ��û�ҵ������������֤
�� �� ֵ���������ֵ      
          1����ȷ
          0��json���ݸ�ʽ����
          -1���û�idδע��
          -2��֤ͨ������
          -3��֤ͨ��������
          -4��֤ͨ�ظ�֧��
          -5��֤ͨδ����
          -6��������˴���
          -7�����鹲ʶδͨ��
          -8���������ݿ����
          -9�������������
  	      
���������
          contractAddress:��Լ�ʺ�
          businessType: �û�����  
          dataHash: ����ժҪ
	  	  dataPath: ���ݴ��·����URL��ַ
	  	  descInfo: ����������Ϣ
���������
	 transactionHash: ����Hash ��Ϣ
	 result:���ؽ��
*/
int EChain_BusinessDataStorage(std::string contractAddress, int businessType, std::string dataHash, std::string dataPath, std::string descInfo, std::string &transactionHash);


/*	
�������ܣ��û�ҵ��������������
�� �� ֵ���������ֵ
          1����ȷ
          0��json���ݸ�ʽ����
          -1���û�idδע��
          -2��֤ͨ������
          -3��֤ͨ��������
          -4��֤ͨ�ظ�֧��
          -5��֤ͨδ����
          -6��������˴���
          -7�����鹲ʶδͨ��
          -8���������ݿ����
          -9�������������
  	      
���������
          contractAddress:��Լ�ʺ�
          updateType: ��������
          businessType: �û�����  
          preTranxHash: ���һ���������ݵĽ���Hashֵ
          dataHash: ����ժҪ
  	  dataPath: ���ݴ��·����URL��ַ
  	  descInfo: ����������Ϣ	  
���������
	 transactionHash: ����Hash ��Ϣ
	 result:���ؽ��
*/
int EChain_BusinessDataUpdate(std::string contractAddress, int updateType, int businessType, std::string preTranxHash, std::string dataHash, std::string dataPath, std::string descInfo, std::string &transactionHash);


/*	
�������ܣ���Լ�˺����������֤
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
	  1����ȷ
          0��json���ݸ�ʽ����
          -1���û�idδע��
          -2��֤ͨ������
          -3��֤ͨ��������
          -4��֤ͨ�ظ�֧��
          -5��֤ͨδ����
          -6��������˴���
          -7�����鹲ʶδͨ��
          -8���������ݿ����
          -9�������������
���������
          blockAddress:�û�ע�������ַ��Ϣ
          contractAddress: �û���Լ�˺�
  	  descInfo: ��Լ�˻�����������Ϣ
���������
	 result:���ش�����
*/
int EChain_ContractAddressDelete(std::string blockAddress, std::string contractAddress, std::string descInfo);



/*	
�������ܣ��û�������ݺ���������֤
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
1����ȷ
          0��json���ݸ�ʽ����
          -1���û�idδע��
          -2��֤ͨ������
          -3��֤ͨ��������
          -4��֤ͨ�ظ�֧��
          -5��֤ͨδ����
          -6��������˴���
          -7�����鹲ʶδͨ��
          -8���������ݿ����
          -9�������������
  	      
���������
          blockAddress:�û�ע�������ַ��Ϣ
          contractAddress:��Լ�ʺ�
          businessType: �û�����  
          transactionHash: ����Hash
          dataHash: ����ժҪ
���������
	 result:���ؽ��
*/
int EChain_UserIntegrityVerification(std::string blockAddress, std::string contractAddress, int businessType, std::string transactionHash, std::string dataHash);


/*	
�������ܣ��������ݵ��û����ݹ���
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
1����ȷ
          0��json���ݸ�ʽ����
          -1���û�idδע��
          -2��֤ͨ������
          -3��֤ͨ��������
          -4��֤ͨ�ظ�֧��
          -5��֤ͨδ����
          -6��������˴���
          -7�����鹲ʶδͨ��
          -8���������ݿ����
          -9�������������
  	      
���������
          blockAddress:�û�ע�������ַ��Ϣ
          contractAddress:��Լ�ʺ�
          businessType: �û�����  
          transactionHash: ����Hash
          dataHash: ����ժҪ
	   
���������
	 dataPath: ���ݴ��·����URL��ַ
	 result:���ؽ��

*/
int EChain_DataSharedContent(std::string blockAddress, std::string contractAddress, int businessType, std::string transactionHash, std::string dataHash, std::string &dataPath);


/*	
�������ܣ����ڽ�����û����ݹ���
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
1����ȷ
          0��json���ݸ�ʽ����
          -1���û�idδע��
          -2��֤ͨ������
          -3��֤ͨ��������
          -4��֤ͨ�ظ�֧��
          -5��֤ͨδ����
          -6��������˴���
          -7�����鹲ʶδͨ��
          -8���������ݿ����
          -9�������������
  	      
���������
          blockAddress:�û�ע�������ַ��Ϣ
          contractAddress:��Լ�ʺ�
          businessType: �û�����  
          transactionHash: ����Hash
          dataHash: ����ժҪ
	  conditionInfo: ����������Ϣ
	     
���������
	 dataResult: ���ݼ�����
	 result:���ؽ��
*/
int EChain_DataSharedResult(std::string blockAddress, std::string contractAddress, int businessType, std::string transactionHash, std::string dataHash, std::string conditionInfo, std::string &dataResult);


/*	
�������ܣ����ݴ�֤ȫ��¼��ѯ
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
���������
		blockAddress: ���ѯ�������ַ
		contractAddress:��Լ�ʺ�
		startTime:��ѯ��������ʼʱ��
		endTime:��ѯ����������ʱ��
		curPage: ��ǰ��ѯҳ
		numsPerPage: ÿҳ��¼����
		
���������businessDataInfo:�û���ʷ������Ϣ
*/
int EChain_QueryAddressRecord(std::string blockAddress, std::string contractAddress, std::string startTime, std::string endTime, int curPage, int numsPerPage, std::string &businessDataInfo);



/*	
�������ܣ����ڽ���Hash�����ݴ�֤��¼��ѯ
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
���������
			transactionHash: ����Hash
���������
			transactionInfo:���������Ϣ
			ledgerSeq:�������к�
			timeInfo:����ʱ����Ϣ
*/
int EChain_QueryTransactionHashRecord(std::string transactionHash, std::string &transactionInfo, int &ledgerSeq, std::string &timeInfo);



/*	
�������ܣ���ȡ�������ݳ�����Ϣ�����Ҵ���ʼ������ſ�ʼ�ķ�ҳ������Ϣ
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
���������
          curPage: ��ǰ��ѯҳ
		  numsPerPage: ÿҳ��¼����
���������blockInfo: ��������������Ϣ��json��ʽ, ��������;
					{"totalCount":60,"blockInfo":[{"ledgerSeq":1000,"blockAge":50,"blockhash":"17663770b8c6744fdd3768e5ff6b35a08b9d285197b3f716f3a2b6d57832b2cd","blockSize":111645},
												  {"ledgerSeq":999,"blockAge":56,"blockhash":"17663770b8c6744fdd3768e5ff6b35a08b9d285197b3f716f3a2b6d57832b2cd","blockSize":111655}
																			 ]}
*/
int EChain_QueryBlockBrowserInfo(int curPage, int numsPerPage, std::string &blockInfo);


/*	
�������ܣ����齻����Ϣ��ѯ
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
���������
		  blockSeq: ��ǰ��ѯ������߶�
���������TransactionInfo: ���齻�׼�¼��Ϣ��json��ʽ																		 ]}
*/
int EChain_QueryBlockTransaction(int blockSeq, std::string &transactionInfo);


/*	
�������ܣ�����������ͳ����Ϣ��ѯ
�� �� ֵ��<0,����ʧ��  >0 �����ɹ�
���������
		  ��
���������chainInfo: �����׼�¼��Ϣ��json��ʽ, ��������;
					{"blockHeight":12786,"blockNodes":8,"blockAccounts":678,"blockTransX":121645,"blockAmount":67800000000,"blockAverTime":60000,"blockTxPerDay":68}
*/
int EChain_QueryChainInfo(std::string &chainInfo);


/*	
�������ܣ���ȡ����������˺ź�nonce���
�� �� ֵ��ָ���ʺŵ�Nonce ֵ�������ʺŷ��� -1 
���������
					��
���������	
					genesAddress: ���������ʺ�
*/
int EChain_GetGenesisInfo(std::string &genesAddress);



/*	
�������ܣ���ȡָ���ʺŵ�Nonce ��Ϣ
�� �� ֵ��ָ���ʺŵ�Nonce ֵ�������ʺŷ��� -1 
���������
					blockAddress: �����ʺ�
���������	
					��
*/
int EChain_GetBlockAddressNonceInfo(std::string blockAddress);



#endif

