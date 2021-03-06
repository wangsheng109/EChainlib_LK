#ifndef API_XCHAIN_FUNC
#define API_XCHAIN_FUNC

#include <gtest/gtest.h>
#include "common.h"
#include "common/http_client.h"
#include "common/private_key.h"
#include "common/web_socket_server.h"
#include "MySQLExecAPI.h"
#include "ReadConfig.h"
#include "OSMutex.h"
#include "api_echain_func.h"
#include <mysql.h>
#include <unistd.h>

#include <utils/crypto.h>
#include <utils/strings.h>
#include <json/json.h>

#include <openssl/md5.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/if_ether.h>

#include <iconv.h>

char logfile_name[64] = {0};
OSMutex logmutex;
FILE *logfile;
time_t ticks;

time_t g_curTime = 0;
time_t g_cmpTime = 0;

OSMutex g_dbmutex;

#define	BLOCKHEADER	686

MYSQL	g_mysql;
MYSQL_RES*         g_res;
MYSQL_ROW          g_row;

std::string g_genesisAddress;
std::string g_destAddress ="ea01c16299cd75dd6d106e17f00ae8b71bf4ab11a3d234";
	
std::string g_assetCode = "LKToken";
int g_genesisNonce;

#define ADDNEW		1
#define UPDATE		2
#define DELETE		3

typedef struct tagBlockMoreItem
{	
 	int iBlockHeight;	
	char szBlockHash[88];
	int iBlockSize;		
	char szDateTime[32];
}BlockMoreItem,*LPBlockMoreItem;


typedef struct tagBusinessDataItem
{	 
	int RecordType;
	int BusinessType;		
	char DataHsh[88];
	char TransactionHash[88];	
	char DateTime[32];
}BusinessDataItem,*LPBusinessDataItem;


typedef struct tagTokenInfoItem
{	 
	char TokenName[64];
	int TokenAmount;
	int TokenBalance;
	int TokenLimited;
}TokenInfoItem,*LPTokenInfoItem;


int getFileLength(const char *pathname)
{	
	int	fd;	
	struct  stat    buf;	
	if ((fd = open(pathname, O_RDONLY)) < 0)	
	{		
		printf("open %s failed!\n", pathname);
		return -2;	
	}        
	if (fstat(fd, &buf) < 0)        
	{		
		printf("fstat error!\n");	
		close(fd);		
		return -1;        
	}	
	close(fd);	
	if (S_ISREG(buf.st_mode))		
		return(buf.st_size);		
	return -1;
}

int ChangeLogFile()
{
	char timebuf[64] = {0};
	time_t t;
    struct tm tm_t;

	logmutex.Lock();
	
	t = time(NULL);
	if (localtime_r(&t, &tm_t) != NULL)
	{
		sprintf(timebuf,"EChain_%04d-%02d-%02d",tm_t.tm_year+1900,tm_t.tm_mon+1,tm_t.tm_mday);
	}
	
	//??????????"2012-07-12"??????, ??????????????????????????
	if(strncmp(logfile_name, timebuf, 17) != 0)
	{
		fclose(logfile);
		logfile = NULL;
		sprintf(logfile_name, "%s-%02d-%02d.log", timebuf, tm_t.tm_hour, tm_t.tm_min);
		logfile = fopen(logfile_name, "w+");
		if(!logfile)
		{
			printf("Can't open log file!\n");
			logmutex.Unlock();
			return -1; 
		}
		logmutex.Unlock();
		return 2;
	}

	int iSize = getFileLength(logfile_name);
	if(iSize < 0)
	{
		printf("getFileLength failed!\n");
		logmutex.Unlock();
		return -2;
	}
	//????????????????????????????
	else if(iSize >= 20 * 1024 * 1024)
	{
		fclose(logfile);
		logfile = NULL;
		sprintf(logfile_name, "%s-%02d-%02d.log", timebuf, tm_t.tm_hour, tm_t.tm_min);
		logfile = fopen(logfile_name, "w+");
		if(!logfile)
		{
			printf("Can't open log file!\n");
			logmutex.Unlock();
			return -1; 
		}
		logmutex.Unlock();
		return 2;
	}
	logmutex.Unlock();
	return 1;
}

void NBDLog(const char *fmt, ...)
{
	
	va_list vl;
	va_start(vl, fmt);
	logmutex.Lock();
	
	ticks = time(NULL);
	fprintf(logfile, "%.24s ", ctime(&ticks));
	
	vfprintf(logfile, fmt, vl);
	
	fflush(logfile);
	logmutex.Unlock();
	va_end(vl);

	g_curTime = time(NULL);

	if((g_curTime - g_cmpTime) >= 180)
	{
		g_cmpTime = g_curTime;
		ChangeLogFile();
	}
}

int StrToHex(char *strData, char *hexData)
{
	int i = 0;
  
  if(strData == NULL || hexData == NULL){
    return -1;
  }

  if(strlen(strData) == 0){
    return -2;
  }
  while (1) {
      if (0==strData[i]) break;
      sprintf(hexData+i*2,"%02X",(unsigned char)strData[i]);
      i++;
  }
  setlocale(LC_ALL,"chs");
//  printf("strData=%s\n hexData=%s\n",strData,hexData);
  return 0;
}

int HexToStr(char *hexData, char *strData)
{
  int i = 0;
  int iTemp;
  if(hexData == NULL || strData == NULL){
    return -1;
  }

  if(strlen(hexData) %2 == 1){
    return -2;
  }

  while (1) {
      if (1!=sscanf(hexData+i*2,"%2x",&iTemp)) break;
      strData[i]=(char)iTemp;
      i++;
  }
  strData[i]='\0';
//  printf("hexData=%s\n strData=%s\n",hexData,strData);
  return 0;
}

int utf8Togb2312(char *sourcebuf, size_t sourcelen,char *destbuf,size_t destlen)
{
  iconv_t cd;
  if( (cd = iconv_open("gb2312","utf-8")) ==0 )
    return -1;
  memset(destbuf,0,destlen);
  char **source = &sourcebuf;
  char **dest = &destbuf;

  if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
    return -1;
  iconv_close(cd);
  return 0;
  
}

int gb2312toutf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{
  iconv_t cd;
  if( (cd = iconv_open("utf-8","gb2312")) ==0 )
    return -1;
  memset(destbuf,0,destlen);
  char **source = &sourcebuf;
  char **dest = &destbuf;

  if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
    return -1;
  iconv_close(cd);
  return 0;
  
}


char LICENSE_NAME[512]		= "license.dat";

#define		TRUE	1
#define		FALSE	0

//????????m
int GetMacAddr(unsigned char* v_buf)
{
	int ret,i;
	int sock;
	int r = 0;
	struct ifreq ifreq;
	struct ifreq ifreqs[20];
	struct ifconf ifconf;

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0)
	{
		perror("socket");
		return (-1);
	}

	memset(&ifconf, 0, sizeof(ifconf));
	ifconf.ifc_buf = (char*) (ifreqs);
	ifconf.ifc_len = sizeof(ifreqs);

	ret = ioctl(sock, SIOCGIFCONF, &ifconf, sizeof(ifconf));
	if (ret < 0)
	{
		perror("ioctl(SIOGIFCONF)");
		return (-1);
	}

	int n = ifconf.ifc_len / sizeof(struct ifreq);
	for (i = 0; i < n; i++)
	{
		if (ifreqs[i].ifr_addr.sa_family != PF_INET)
		{
			continue;
		}
		memset(&ifreq, 0, sizeof(ifreq));
		strcpy(ifreq.ifr_name, ifreqs[i].ifr_name);
		ret = ioctl(sock, SIOCGIFHWADDR, &ifreq, sizeof(ifreq));
		if (ret < 0 || ifreq.ifr_hwaddr.sa_family != ARPHRD_ETHER)
		{
			continue;
		}
		memcpy(v_buf + r*6, ifreq.ifr_hwaddr.sa_data, 6);
		r++;
	}
	close(sock);
	return r;
}

char *MD5End2(MD5_CTX *ctx, char *buf)
{
	int i;
	unsigned char digest[16];
	static const char hex[]="0123456789abcdef";

	if (!buf)
		return 0;
	MD5_Final(digest, ctx);
	for (i = 0; i < 16; i++) {
		buf[i+i] = hex[digest[i] >> 4];
		buf[i+i+1] = hex[digest[i] & 0x0f];
	}
	buf[i+i] = '\0';
	return buf;
}

int VerifyCertificate(char *szCertFileName)
{
	int n = 0, i;
	unsigned char mac_buf[48] = {0};
	unsigned char license_buf[32] = {0};
	FILE *fp;
	
	fp = fopen(szCertFileName, "rb");
	if (fp == NULL)
	{
		printf("Cannot open licence file %s\n", szCertFileName);
		return 0;
	}
	i = fread(license_buf, 1, 32, fp);
	if (i < 32)
	{
		printf("The content of licence file is error!\n");
		return 0;
	}
	fclose(fp);
	
	n = GetMacAddr(mac_buf);
	for (i = 0; i < n; i++)
	{
		//get md5 buf
		const char ver_buf[] = "Miracles happen every day.";
		char md5_buf[40] = {0};
		MD5_CTX ctx;

		MD5_Init(&ctx);
		MD5_Update(&ctx, mac_buf + i * 6, 6);
		MD5_Update(&ctx, (const unsigned char*)ver_buf, strlen(ver_buf));
		MD5_Update(&ctx, mac_buf + i * 6, 6);
		MD5End2(&ctx, md5_buf);

		int cmp = memcmp(md5_buf, license_buf, 32);
		
		//printf("[%s]", md5_buf);
		if (cmp == 0)
		{
			return 1;
		}
	}

	printf("Licence check failed!\n");
	return 0;
}



int InitDataBasesConnection()
{
	mysql_init(&g_mysql);//??????MYSQL????????????????
	//????????????
	char szValue[1] = {'1'};
	mysql_options(&g_mysql, MYSQL_OPT_RECONNECT, szValue);

	
	if (!mysql_real_connect(&g_mysql, SConfigPara::m_DBAddr.c_str(),SConfigPara::m_DBUser.c_str(),SConfigPara::m_DBPassWd.c_str(),SConfigPara::m_DBName.c_str(),SConfigPara::m_DBPort,NULL,0))
	{
		NBDLog("DBAddr is %s, DBUser is %s, DBPasswd is %s, DBPort is %d!\n", SConfigPara::m_DBAddr.c_str(),SConfigPara::m_DBUser.c_str(), SConfigPara::m_DBPassWd.c_str(), SConfigPara::m_DBPort);
		NBDLog("Can't connect the mysql, error is:%s", mysql_error(&g_mysql));
		return -1;
	}
	else
	{
		NBDLog("connect database success!\n");
	}

	mysql_query(&g_mysql,"SET NAMES UTF8");
//	DBmutex.Unlock();
	if (!mysql_set_character_set(&g_mysql, "utf8")) 
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

int DeInitDataBasesConnect()
{
	if (g_res != 0 )
	{
		mysql_free_result(g_res);
		g_res = 0;
	}
	
	mysql_close(&g_mysql);

	return 1;
}

int ParseGenesInfoFromJson(ECHAINS::HttpClient::RecvMessage &revMessage, std::string &accountAddr, int32_t &nonceNum)
{
	Json::Value rep;
	Json::Reader reader;
	std::string address;
	int32_t iNonceNum;
		
//	NBDLog("%s!\n", rec_accouts.context.c_str());	
	if (!reader.parse(revMessage.context.c_str(), rep))  // reader??Json????????????rep??rep??????Json????????????	
	{
		NBDLog("reader.parse failed in ParseGenesInfoFromJson!\n");	
		return -1;
	}
	
	if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
	{
		if (rep.isMember("result") && rep["result"].isMember("address"))
		{
			address = rep["result"]["address"].asString();
		}
		else
		{
			NBDLog("Can't find private_key in ParseGenesInfoFromJson!\n");
			return -1;
		}
		if (rep.isMember("result") && rep["result"].isMember("nonce"))
		{
			iNonceNum = rep["result"]["nonce"].asInt();
		}
		else
		{
			nonceNum = 0;
		}
	}
	else
	{
		NBDLog("error_code is %d in ParseGenesInfoFromJson!\n", rep["error_code"].asInt());
		return -1;
	}
	accountAddr = address;
	nonceNum = iNonceNum;
	return 1;
}

Json::Value GetCreateAccountJson(echain::PrivateKey & srcPrv, std::string & destAddr, int32_t & nonce){
	
	Json::Value transaction_json;
		
//	transaction_json["expr_condition"] = "";
//	transaction_json["metadata"] = "";
	transaction_json["source_address"] = srcPrv.GetBase16Address();
//	transaction_json["source_address"] = g_genesisAddress;
	transaction_json["nonce"] = nonce;
	
//	transaction_json["operationtype"] = 1;
//	transaction_json["orderVol"] = 67;
//	transaction_json["orderSum"] = 136;
	
	Json::Value operation;
	Json::Value createAccount;
	Json::Value privInfo;
	Json::Value thresholds;
	Json::Value metaDatas;
		
	thresholds["tx_threshold"] = 7;	
	privInfo["master_weight"]	= 10;
//	privInfo["signers"]	= "[]";	
	privInfo["thresholds"]	= Json::Value(thresholds);
		
	metaDatas["key"]	= "HELLO";
	metaDatas["value"]	= "XChain";	
		
	createAccount["dest_address"] = destAddr;
	createAccount["metadatas"].append(metaDatas);
	createAccount["priv"] = Json::Value(privInfo);
		
	operation["create_account"] = Json::Value(createAccount);
	operation["type"] = 1;
	transaction_json["operations"].append(operation);

	return transaction_json;
}

Json::Value GetCreateAccountJsonX(echain::PrivateKey & srcPrv, std::string & destAddr, std::string & hexInfo, int32_t & nonce){
	
	Json::Value transaction_json;
		
//	transaction_json["expr_condition"] = "";
	transaction_json["metadata"] = hexInfo;
	transaction_json["source_address"] = srcPrv.GetBase16Address();
	transaction_json["nonce"] = nonce;
	
	
	
	Json::Value operation;
	Json::Value createAccount;
	Json::Value privInfo;
	Json::Value thresholds;
	Json::Value metaDatas;
		
	thresholds["tx_threshold"] = 7;	
	privInfo["master_weight"]	= 10;
//	privInfo["signers"]	= "[]";	
	privInfo["thresholds"]	= Json::Value(thresholds);
		
		
	metaDatas["key"]	= "hello";
	metaDatas["value"]	= "GIB";	
		
	createAccount["dest_address"] = destAddr;
	createAccount["metadatas"].append(metaDatas);
	createAccount["priv"] = Json::Value(privInfo);
		
	operation["create_account"] = Json::Value(createAccount);
	operation["type"] = 1;
	transaction_json["operations"].append(operation);

	return transaction_json;
}



Json::Value GetPaymentJson(std::string srcAddress, std::string destAddress, int32_t nonce, std::string code, int32_t amount)
{
	Json::Value transaction_json;
		
//	transaction_json["expr_condition"] = "";
//	transaction_json["metadata"] = hexInfo;
	transaction_json["source_address"] = srcAddress;
	transaction_json["nonce"] = nonce;
	
	Json::Value operation;
	Json::Value payment;
	Json::Value asset;
	Json::Value property;
	Json::Value input;
		
	property["issuer"] = g_genesisAddress;
	property["code"]	= code;
	asset["amount"]	= amount;	
//	asset["amount"]	= 1.599999;
	//????????????????1
//	asset["amount"]	= 1;	
	asset["property"]	= Json::Value(property);
		
	input["bar"]	= "foo";
	
	payment["dest_address"] = destAddress;
	payment["asset"] = Json::Value(asset);
//	payment["input"] = Json::Value(input);
	
	operation["payment"] = Json::Value(payment);
	operation["type"] = 3;
	transaction_json["operations"].append(operation);

	return transaction_json;
}

Json::Value GetPaymentJsonX(std::string srcAddress, std::string destAddress, std::string issuerAddress, std::string hexInfo, int32_t nonce, std::string code, int32_t amount)
{
	Json::Value transaction_json;
		
//	transaction_json["expr_condition"] = "";
	transaction_json["metadata"] = hexInfo;
	transaction_json["source_address"] = srcAddress;
	transaction_json["nonce"] = nonce;
	
	Json::Value operation;
	Json::Value payment;
	Json::Value asset;
	Json::Value property;
	Json::Value input;
		
	property["issuer"] = issuerAddress;
	property["code"]	= code;
	asset["amount"]	= amount;	
	//????????????????1
//	asset["amount"]	= 1;	
	asset["property"]	= Json::Value(property);
		
	input["bar"]	= "foo";
	
	payment["dest_address"] = destAddress;
	payment["asset"] = Json::Value(asset);
//	payment["input"] = Json::Value(input);
	
	operation["payment"] = Json::Value(payment);
	operation["type"] = 3;
	transaction_json["operations"].append(operation);

	return transaction_json;
}

Json::Value SubmintTransaction(echain::PrivateKey &prv,std::string sign_data, std::string transaction_blob){
	Json::Value item;

	Json::Value signature;
	signature["sign_data"] = sign_data;
	signature["public_key"] =prv.GetBase16PublicKey();

	item["signatures"].append(signature);
	item["transaction_blob"] = transaction_blob;

	Json::Value items;
	items["items"].append(item);
	return items;
}

Json::Value GetIssueAssetJson(std::string srcAddress, std::string hexInfo, int32_t nonce, int32_t assetAmount, std::string assetCode)
{
	Json::Value transaction_json;
		
	transaction_json["metadata"] = hexInfo;
	transaction_json["expr_condition"] = "";
	transaction_json["source_address"] = srcAddress;
	transaction_json["nonce"] = nonce;
	Json::Value operation;
#if 0
	operation["type"] = 2;
	operation["asset_type"] = 1;
	operation["asset_issuer"] = srcAddress;
	operation["asset_code"] = assetCode;
	operation["asset_amount"] = assetAmount;
	transaction_json["operations"].append(operation);
#else
//operation["issuer"] = prv.GetBase16Address();
	Json::Value issueasset;
//issueasset["asset_type"] = 1;
	issueasset["code"] = assetCode;
	issueasset["amount"] = assetAmount;
//	issueasset["amount"] = 1000.59999999;
	
	operation["issue_asset"] = Json::Value(issueasset);
	operation["type"] = 2;
	transaction_json["operations"].append(operation);
#endif	

	return transaction_json;
}	


int ParseNonceNumFromJson(ECHAINS::HttpClient::RecvMessage &revMessage)
{
	int32_t iNonceNum = 0;
	Json::Value rep;
	Json::Reader reader;
		
	
	if (!reader.parse(revMessage.context.c_str(), rep))  // reader??Json????????????rep??rep??????Json????????????	
	{
		NBDLog("reader.parse failed in ParseNonceNumFromJson!\n");	
		return -1;
	}
	
	if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
	{
		if (rep.isMember("result") && rep["result"].isMember("nonce"))
		{
			iNonceNum = rep["result"]["nonce"].asInt();
		}
		else
		{
			iNonceNum = 0;
		}
	}
	else
	{
		NBDLog("error_code is %d in ParseNonceNumFromJson!\n", rep["error_code"].asInt());
		//??????????
		if(rep["error_code"].asInt()==4)
		{
			iNonceNum = 0;
		}
		else
		{
			return -1;
		}
	}
	return iNonceNum;
}

//????????????
int ParseModulesStatusInfoFromJson(ECHAINS::HttpClient::RecvMessage &revMessage, std::string &jsonInfo)
{
	Json::Value rep;
	Json::Reader reader;
	
	int iBlocks = 0;
	int iAccounts = 0;
	int iTxNums = 0;
	int iNodes = 0;
	int iBlockAvegTime = 0;
	int iTxsPerTime = 0;
		
//	NBDLog("%s!\n", rec_accouts.context.c_str());	
	if (!reader.parse(revMessage.context.c_str(), rep))  // reader??Json????????????rep??rep??????Json????????????	
	{
		NBDLog("reader.parse failed in ParseModulesStatusInfoFromJson!\n");	
		return -1;
	}
	
	if (rep.isMember("ledger_manager"))
	{
		iBlocks = rep["ledger_manager"]["ledger_sequence"].asInt();
		iAccounts = rep["ledger_manager"]["account_count"].asInt();
		iTxNums = rep["ledger_manager"]["tx_count"].asInt();
		
		iBlockAvegTime = rand()%(64000-56000)+56000;
		//iTxsPerTime = iBlocks*60/iTxNums;
		iTxsPerTime = iTxNums*3/iBlocks;  //??????????8????????24??????1/3
		if(iTxsPerTime <= 1)
			iTxsPerTime = rand()%(6-1)+1;
	}
	
	if (rep.isMember("peer_manager") && !rep["peer_manager"]["consensus_network"].isNull() && !rep["peer_manager"]["consensus_network"]["peers"].isNull())
	{
			Json::Value val_peers = rep["peer_manager"]["consensus_network"]["peers"];   
			iNodes = val_peers.size() + 1;
	}
	else
	{
		iNodes = 4;
	}
	
	Json::Value block_json;
		
	block_json["blockHeight"] = iBlocks;
	block_json["blockNodes"] = iNodes;
	block_json["blockAccounts"] = iAccounts;
	block_json["blockTransX"] = iTxNums;
	block_json["blockAverTime"] = iBlockAvegTime;
	block_json["blockTxPerMin"] = iTxsPerTime;
	
	jsonInfo = block_json.toStyledString();
//	printf("jsonInfo = %s!\n", jsonInfo.c_str());

	return 1;
}

int ParseAccountInfoFromJson(ECHAINS::HttpClient::RecvMessage &revMessage, std::string &accountAddr, std::string &prvKey, std::string &pubKey)
{
	Json::Value rep;
	Json::Reader reader;
	std::string address;
	std::string privateKey;
	std::string publicKey;
		
//	NBDLog("%s!\n", rec_accouts.context.c_str());	
	if (!reader.parse(revMessage.context.c_str(), rep))  // reader??Json????????????rep??rep??????Json????????????	
	{
		NBDLog("reader.parse failed in ParseAccountInfoFromJson!\n");	
		return -1;
	}
	
	if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
	{
		if (rep.isMember("result") && rep["result"].isMember("address"))
		{
			address = rep["result"]["address"].asString();
		}
		if (rep.isMember("result") && rep["result"].isMember("private_key"))
		{
			privateKey = rep["result"]["private_key"].asString();
		}
		if (rep.isMember("result") && rep["result"].isMember("public_key"))
		{
			publicKey = rep["result"]["public_key"].asString();
		}
		else
		{
			NBDLog("Can't find account info in ParseAccountInfoFromJson!\n");
			return -1;
		}
	}
	else
	{
		NBDLog("error_code is %d in ParseAccountInfoFromJson!\n", rep["error_code"].asInt());
		return -1;
	}
	accountAddr = address;
	prvKey = privateKey;
	pubKey = publicKey;
	return 1;
}


int ParseCreditInfoFromHexString(std::string metaData, std::string &creditInfo)
{
	Json::Value rep;
	Json::Reader reader;
	char szBuff[128] = {0};
		
	int iLen = metaData.size();
	char *pCreditData  = (char *)malloc(sizeof(char)*(iLen/2 + 1));
	if(pCreditData == NULL)		
	{			
		NBDLog("malloc pCreditData failed in ParseCreditInfoFromHexString!\n");			
		return -1;		
	}	
	if(HexToStr((char *)metaData.c_str(), pCreditData) < 0)
	{
		NBDLog("HexToStr failed in ParseCreditInfoFromHexString!\n");
		return -1;
	}
//	NBDLog("HexToStr, StrData is %s!\n", pStrData);
	
	creditInfo = pCreditData;
	if(pCreditData != NULL)
	{
		free(pCreditData); 
		pCreditData = NULL;
	}
	
	return 1;
}

//????????????????
int ParseBlockHeightFromJson(ECHAINS::HttpClient::RecvMessage &revMessage, int &blockHeight)
{
	Json::Value rep;
	Json::Reader reader;
	
	int iBlocks = 0;

	if (!reader.parse(revMessage.context.c_str(), rep))  
	{
		NBDLog("reader.parse failed in ParseBlockHeightFromJson!\n");	
		return -1;
	}
	
	if (rep.isMember("ledger_manager"))
	{
		iBlocks = rep["ledger_manager"]["ledger_sequence"].asInt();
	}
	else
	{
		NBDLog("reader.parse ledger_sequence failed in ParseBlockHeightFromJson!\n");	
		return -1;
	}
	blockHeight = iBlocks;
	return 1;
}

int ParseMetaInfoFromTransactionHash(std::string transactionHash, std::string &transactionInfo)
{
	ECHAINS::HttpClient http;
	ECHAINS::HttpClient::RecvMessage rec;
	Json::Value rep;
	Json::Reader reader;
	std::string metaDataInfo;
	double dCloseTime;
	char szCloseTime[64] = {0};
	
	http.Initialize("127.0.0.1:25000");	
	
	rec= http.http_request(ECHAINS::HttpClient::HTTP_GET, "getTransactionHistory?hash=" + transactionHash, "");
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in EChain_QueryTransactionHashInfo!\n");	
		return -1;
	}
	
	if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
	{
		if (rep.isMember("result") && rep["result"].isMember("transactions"))
		{
			Json::Value val_transactions = rep["result"]["transactions"];   
			int transNum = val_transactions.size();
			for(int j = 0; j < transNum; j++)
			{
				if(val_transactions[j].isMember("transaction") && val_transactions[j]["transaction"].isMember("metadata"))
				{
					std::string szHexString = val_transactions[j]["transaction"]["metadata"].asString();
					NBDLog("szHexString  = %s!\n", szHexString.c_str());	
					if(ParseCreditInfoFromHexString(szHexString, metaDataInfo) < 0)
					{
						NBDLog("ParseCreditInfoFromHexString failed!\n");
						return -1;
					}
					NBDLog("metaDataInfo  = %s!\n", metaDataInfo.c_str());	
					transactionInfo = metaDataInfo;
					
					return 1;
				}
			}
		}	
		else
		{	
			NBDLog("transactions is null, rec is \n%s\n", rec.context.c_str());	
			transactionInfo = " ";
		}
	}
	else
	{
		NBDLog("error_code is %d in ParseMetaInfoFromTransactionHash!\n", rep["error_code"].asInt());	
		return -6;
	}
	
	
//	NBDLog("blockHash = %s, blockHeight = %d!\n", blockHash.c_str(), blockHeight);
	
	return 1;
}


int getPrivateKeyFromUserRegisterDB(std::string srcAddress, std::string &prvKey)
{
	char SQLCmd[1024] = {0};
	char szPrvKey[128] = {0};
	g_dbmutex.Lock();
	//select a.place_id SysID, a.place_name SysName,  a.channel_id ChannelID, b.passwd Passwd from sys_places a,sys_place_users b where a.place_id = '%s' and a.place_id = b.place_id and b.user_name = '%s' and b.is_active = '1';"
	sprintf(SQLCmd, "select privatekey from user_register where blockaddress='%s';", srcAddress.c_str());
	
	mysql_ping(&g_mysql);
	int iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		g_row = mysql_fetch_row(g_res);
		
		if (g_row != NULL)
		{
			strcpy(szPrvKey, g_row[0]);
			prvKey = szPrvKey;
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return 1;
		}
		else
		{
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return -1;
		}
	}
}

int getPrivateKeyFromContractAccountDB(std::string contractAddress, std::string &prvKey)
{
	char SQLCmd[1024] = {0};
	char szPrvKey[128] = {0};
	g_dbmutex.Lock();
	//select a.place_id SysID, a.place_name SysName,  a.channel_id ChannelID, b.passwd Passwd from sys_places a,sys_place_users b where a.place_id = '%s' and a.place_id = b.place_id and b.user_name = '%s' and b.is_active = '1';"
	sprintf(SQLCmd, "select privatekey from contract_account where contractaddress='%s';", contractAddress.c_str());
	
	mysql_ping(&g_mysql);
	int iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		g_row = mysql_fetch_row(g_res);
		
		if (g_row != NULL)
		{
			strcpy(szPrvKey, g_row[0]);
			prvKey = szPrvKey;
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return 1;
		}
		else
		{
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return -1;
		}
	}
}


int getCurTimeByString(char *v_curTimeInfo)
{	
	time_t cur_time;
	struct	tm	ptr;
	
	cur_time = time(NULL);
	if (localtime_r(&cur_time, &ptr) != NULL)
	{
		sprintf(v_curTimeInfo, "%04d-%02d-%02d %02d:%02d:%02d", ptr.tm_year+1900,
			ptr.tm_mon+1, ptr.tm_mday, ptr.tm_hour, ptr.tm_min, ptr.tm_sec);
		return 1;
	}
	return -1;
}

int transTimeToString(double transTime, char *v_curTimeInfo)
{	
	time_t cur_time;
	struct	tm	ptr;
	
	cur_time = transTime/1000/1000;
	time_t ttime = time(NULL);
//	printf("cur_time = %d, ttime = %d!\n", cur_time, ttime);
	if (localtime_r(&cur_time, &ptr) != NULL)
	{
		sprintf(v_curTimeInfo, "%04d-%02d-%02d %02d:%02d:%02d", ptr.tm_year+1900,
			ptr.tm_mon+1, ptr.tm_mday, ptr.tm_hour, ptr.tm_min, ptr.tm_sec);
		return 1;
	}
	return -1;
}

int processUserRegisterInfo(std::string userID, int userType, std::string userInfo, std::string &blockAddress, std::string &blockPrvKey, std::string &blockPubKey, std::string &transHash)
{
	char szBarTime[64] = {0};
	char SQLCmd[1024] = {0};
	char szAddress[128] = {0};
	char szPrvKey[128] = {0};
	char szPubKey[128] = {0};
	char szTxHash[128] = {0};
	
	getCurTimeByString(szBarTime);
	
	g_dbmutex.Lock();
	//select a.place_id SysID, a.place_name SysName,  a.channel_id ChannelID, b.passwd Passwd from sys_places a,sys_place_users b where a.place_id = '%s' and a.place_id = b.place_id and b.user_name = '%s' and b.is_active = '1';"
	sprintf(SQLCmd, "select blockaddress, privatekey, publickey, transactionhash from user_register where userid='%s';", userID.c_str());
	
	mysql_ping(&g_mysql);
	int iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		g_row = mysql_fetch_row(g_res);
		if (g_row != NULL)
		{
			//????????????????????????????
			NBDLog("there is a conflict userid %s in user_register table, please recheck it!\n", userID.c_str());
			
			
			strcpy(szAddress, g_row[0]);
			blockAddress = szAddress;
			strcpy(szPrvKey, g_row[1]);
			blockPrvKey = szPrvKey;
			strcpy(szPubKey, g_row[2]);
			blockPubKey = szPubKey;
			strcpy(szTxHash, g_row[3]);
			transHash = szTxHash;
			
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return 0;
		}
		else	//??????????????????
		{
			mysql_free_result(g_res);
			memset(SQLCmd, 0, 1024);
			sprintf(SQLCmd, "select blockaddress, privatekey, publickey, transactionhash from preaccount_info limit 1;");
			
			iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
			if(iResult)
			{
				printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
				NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
				g_dbmutex.Unlock();
				return -1;
			}
			else 
			{
				g_res = mysql_store_result(&g_mysql);//????????????????????
				g_row = mysql_fetch_row(g_res);
				if (g_row != NULL)
				{
					strcpy(szAddress, g_row[0]);
					blockAddress = szAddress;
					strcpy(szPrvKey, g_row[1]);
					blockPrvKey = szPrvKey;
					strcpy(szPubKey, g_row[2]);
					blockPubKey = szPubKey;
					strcpy(szTxHash, g_row[3]);
					transHash = szTxHash;
					
				//	NBDLog("blockAddress = %s, blockPrvKey = %s!\n", blockAddress.c_str(), blockPrvKey.c_str());
					memset(SQLCmd, 0, 1024);
					sprintf(SQLCmd, "delete from preaccount_info where blockaddress='%s';", blockAddress.c_str());
					iResult = mysql_query(&g_mysql,SQLCmd);
					if (iResult)
					{
						printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						g_dbmutex.Unlock();
						return -1;
					}
				//	NBDLog("SQLCmd = %s, blockPrvKey = %s!\n", SQLCmd, blockPrvKey.c_str());
					memset(SQLCmd, 0, 1024);
					sprintf(SQLCmd, "insert into user_register values(null, '%s', '%d','%s', '%s', '%s', '%s', '%s', '%s');", userID.c_str(), userType, userInfo.c_str(), blockAddress.c_str(), blockPrvKey.c_str(), blockPubKey.c_str(), transHash.c_str(), szBarTime);
					
					mysql_ping(&g_mysql);
					iResult = mysql_query(&g_mysql,SQLCmd);
					if (iResult)
					{
						printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						g_dbmutex.Unlock();
						return -1;
					}
			
					mysql_free_result(g_res);
					g_dbmutex.Unlock();
					return 1;
				}
				else
				{
					mysql_free_result(g_res);
					g_dbmutex.Unlock();
					return -1;
				}
				
			}
		}
	}
	
	return 1;
}

int processContractAccountInfo(std::string userID, std::string classID, std::string &contractAddress, std::string &blockPrvKey, std::string &blockPubKey, std::string &transHash)
{
	char szBarTime[64] = {0};
	char SQLCmd[1024] = {0};
	char szAddress[128] = {0};
	char szPrvKey[128] = {0};
	char szPubKey[128] = {0};
	char szTxHash[128] = {0};
	
	getCurTimeByString(szBarTime);
	
	g_dbmutex.Lock();
	//select a.place_id SysID, a.place_name SysName,  a.channel_id ChannelID, b.passwd Passwd from sys_places a,sys_place_users b where a.place_id = '%s' and a.place_id = b.place_id and b.user_name = '%s' and b.is_active = '1';"
	sprintf(SQLCmd, "select contractaddress, privatekey, publickey, transactionhash from contract_account where userid='%s' and classid = '%s';", userID.c_str(), classID.c_str());
	
	mysql_ping(&g_mysql);
	int iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		g_row = mysql_fetch_row(g_res);
		if (g_row != NULL)
		{
			//????????????????????????????
			NBDLog("there is a conflict userid %s and classid %s in contract_account table, please recheck it!\n", userID.c_str(), classID.c_str());
			
			
			strcpy(szAddress, g_row[0]);
			contractAddress = szAddress;
			strcpy(szPrvKey, g_row[1]);
			blockPrvKey = szPrvKey;
			strcpy(szPubKey, g_row[2]);
			blockPubKey = szPubKey;
			strcpy(szTxHash, g_row[3]);
			transHash = szTxHash;
			
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return 0;
		}
		else	//??????????????????
		{
			mysql_free_result(g_res);
			memset(SQLCmd, 0, 1024);
			sprintf(SQLCmd, "select blockaddress, privatekey, publickey, transactionhash from preaccount_info limit 1;");
			
			iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
			if(iResult)
			{
				printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
				NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
				g_dbmutex.Unlock();
				return -1;
			}
			else 
			{
				g_res = mysql_store_result(&g_mysql);//????????????????????
				g_row = mysql_fetch_row(g_res);
				if (g_row != NULL)
				{
					strcpy(szAddress, g_row[0]);
					contractAddress = szAddress;
					strcpy(szPrvKey, g_row[1]);
					blockPrvKey = szPrvKey;
					strcpy(szPubKey, g_row[2]);
					blockPubKey = szPubKey;
					strcpy(szTxHash, g_row[3]);
					transHash = szTxHash;
					
				//	NBDLog("blockAddress = %s, blockPrvKey = %s!\n", blockAddress.c_str(), blockPrvKey.c_str());
					memset(SQLCmd, 0, 1024);
					sprintf(SQLCmd, "delete from preaccount_info where blockaddress='%s';", contractAddress.c_str());
					iResult = mysql_query(&g_mysql,SQLCmd);
					if (iResult)
					{
						printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						g_dbmutex.Unlock();
						return -1;
					}
				//	NBDLog("SQLCmd = %s, blockPrvKey = %s!\n", SQLCmd, blockPrvKey.c_str());
					memset(SQLCmd, 0, 1024);
					sprintf(SQLCmd, "insert into contract_account values(null, '%s', '%s', '%s', '%s', '%s', '%s', '%s');", userID.c_str(), classID.c_str(), contractAddress.c_str(), blockPrvKey.c_str(), blockPubKey.c_str(), transHash.c_str(), szBarTime);
					
					mysql_ping(&g_mysql);
					iResult = mysql_query(&g_mysql,SQLCmd);
					if (iResult)
					{
						printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
						g_dbmutex.Unlock();
						return -1;
					}
			
					mysql_free_result(g_res);
					g_dbmutex.Unlock();
					return 1;
				}
				else
				{
					mysql_free_result(g_res);
					g_dbmutex.Unlock();
					return -1;
				}
				
			}
		}
	}
	
	return 1;
}


int insertAccountInfo2DB(std::string srcAddress, std::string prvKey, std::string pubKey)
{
	char szBarTime[64] = {0};
	char SQLCmd[1024] = {0};
	
	getCurTimeByString(szBarTime);
	
	g_dbmutex.Lock();
	//select a.place_id SysID, a.place_name SysName,  a.channel_id ChannelID, b.passwd Passwd from sys_places a,sys_place_users b where a.place_id = '%s' and a.place_id = b.place_id and b.user_name = '%s' and b.is_active = '1';"
	sprintf(SQLCmd, "select address from account_info where address='%s';", srcAddress.c_str());
	
	mysql_ping(&g_mysql);
	int iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		g_row = mysql_fetch_row(g_res);
		if (g_row != NULL)
		{
			//????????????????????????????
			NBDLog("there is a conflict address in account_info table, please recheck it!\n");
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return 0;
		}
		mysql_free_result(g_res);
		memset(SQLCmd, 0, 1024);
		sprintf(SQLCmd, "insert into account_info values(null, '%s', '%s', '%s', '%s');", srcAddress.c_str(), prvKey.c_str(), pubKey.c_str(), szBarTime);
		
	//	mysql_ping(&g_mysql);
		iResult = mysql_query(&g_mysql,SQLCmd);
		if (iResult)
		{
			printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
			NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
			g_dbmutex.Unlock();
			return -1;
		}
		
	}
	g_dbmutex.Unlock();
	
	return 1;
}

int insertPreAccountInfo2DB(std::string srcAddress, std::string privateKey, std::string publicKey, std::string transHash)
{
	char szBarTime[64] = {0};
	char SQLCmd[1024] = {0};
	
	getCurTimeByString(szBarTime);
	
	g_dbmutex.Lock();
	
	sprintf(SQLCmd, "select blockaddress from preaccount_info where blockaddress='%s';", srcAddress.c_str());
	
	mysql_ping(&g_mysql);
	int iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		g_row = mysql_fetch_row(g_res);
		if (g_row != NULL)
		{
			//????????????????????????????
			NBDLog("there is a conflict address in preaccount_info table, please recheck it!\n");
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return 0;
		}
		mysql_free_result(g_res);
		memset(SQLCmd, 0, 1024);
		sprintf(SQLCmd, "insert into preaccount_info values(null, '%s', '%s', '%s', '%s', '%s');", srcAddress.c_str(), privateKey.c_str(), publicKey.c_str(), transHash.c_str(), szBarTime);
		
	//	mysql_ping(&g_mysql);
		iResult = mysql_query(&g_mysql,SQLCmd);
		if (iResult)
		{
			printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
			NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
			g_dbmutex.Unlock();
			return -1;
		}
		
	}
	g_dbmutex.Unlock();
	
	return 1;
}


int insertCertificationInfo2DB(std::string blockAddress, std::string userName, std::string transactionHash)
{
	char szBarTime[64] = {0};
	char SQLCmd[1024] = {0};
	
	getCurTimeByString(szBarTime);
	
	g_dbmutex.Lock();
	mysql_ping(&g_mysql);
	//????????????
	sprintf(SQLCmd, "insert into user_certification values(null, '%s', '%s', '%s', '%s');", blockAddress.c_str(), userName.c_str(), transactionHash.c_str(), szBarTime);
	
	int iResult = mysql_query(&g_mysql, SQLCmd);
	if (iResult)
	{
		printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
		NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	g_dbmutex.Unlock();
	return 1;
}

int insertBusinessDataInfo2DB(std::string contractAddress, int updateType, int businessType, std::string dataHash, std::string transactionHash)
{
	char szBarTime[64] = {0};
	char SQLCmd[1024] = {0};
	
	getCurTimeByString(szBarTime);
	
	g_dbmutex.Lock();
	mysql_ping(&g_mysql);
	//????????????
	sprintf(SQLCmd, "insert into business_data values(null, '%s', '%d', '%d', '%s', '%s', '%s');", 
			contractAddress.c_str(), updateType, businessType, dataHash.c_str(), transactionHash.c_str(), szBarTime);
	
	int iResult = mysql_query(&g_mysql, SQLCmd);
	if (iResult)
	{
		printf("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
		NBDLog("mysql_query() fail!sqlcmd is %s\n",SQLCmd);
		g_dbmutex.Unlock();
		return -1;
	}
	g_dbmutex.Unlock();
	return 1;
}

int EChain_InitSDK()
{
	if(ReadIni() < 0)
	{
		printf("ReadIni failed!\n");
		return -1;
	}
	
//	printf("SConfigPara::m_DebugLog = %d!\n", SConfigPara::m_DebugLog);
	if(SConfigPara::m_DebugLog == 1)
	{
		int year, month, day, hour, minute;
		time_t cur_time;
		struct	tm	ptr;
		cur_time = time(NULL);
		if (localtime_r(&cur_time, &ptr) != NULL)
		{
			sprintf(logfile_name, "EChain_%04d-%02d-%02d-%02d-%02d.log", ptr.tm_year+1900,
				ptr.tm_mon+1, ptr.tm_mday, ptr.tm_hour, ptr.tm_min);
		}
		else
		{
			strcpy(logfile_name, "XChain.log");
		}
		
	//	LOG("create log file %s!\n", logfile_name);
		logfile = fopen(logfile_name, "w+");
	//	logfile = stderr;//fopen("log.txt", "w");
		if(!logfile)
		{
			printf("Can't open log file!\n");
			return -1;
		}
		
	}
	else
	{
		logfile = stderr;
		return 1;
	}
	
	if (!VerifyCertificate(LICENSE_NAME))
	{
		return -3;
	}
	if(InitDataBasesConnection() < 0)
	{
		printf("InitDataBasesConnection failed!\n");
		return -1;
	}
	
	g_curTime = time(NULL);
	g_cmpTime = time(NULL);
	
	g_genesisNonce = EChain_GetGenesisInfo(g_genesisAddress);
	if(g_genesisNonce < 0)
	{
		printf("EChain_GetGenesisInfo failed!\n");
		return -1;
	}
	
	NBDLog("EChain_InitSDK success, the genesis addr is %s, the genesis nonce is %d!\n", g_genesisAddress.c_str(), g_genesisNonce);
	
	return 1;
}


//????????
int EChain_CreateAccount(std::string &newAddress)
{
	ECHAINS::HttpClient http;
	int iRet = -1;
	std::string genesPrvKey;
	std::string destAddress;
	std::string privateKey;
	std::string publicKey;
	std::string txHash;
	
	http.Initialize("127.0.0.1:25000");
Again:	
	//????????????????????
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "createAccount", "");
	
//	NBDLog("createAccount response is \n%s\n", rev_accouts.context.c_str());	
	if(ParseAccountInfoFromJson(rev_accouts, destAddress, privateKey, publicKey) < 0)
	{
		NBDLog("ParseAccountInfoFromJson error!\n");
		return -1;
	}
	
	//??????????????????????
	if(getPrivateKeyFromUserRegisterDB(g_genesisAddress, genesPrvKey) < 0)
	{
		NBDLog("getPrivateKeyFromUserRegisterDB failed!\n");
		return -1;
	}
//	NBDLog("EChain_CreateAccount genesPrvKey = %s!\n", genesPrvKey.c_str());
	echain::PrivateKey genesPrv(genesPrvKey);

	g_genesisNonce = EChain_GetGenesisInfo(g_genesisAddress);
Loop:
  Json::Value resq_tx = GetCreateAccountJson(genesPrv, destAddress, g_genesisNonce);
  	
//  NBDLog("CreateAccount request is \n%s\n", resq_tx.toStyledString().c_str());	
  
	ECHAINS::HttpClient::RecvMessage rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "getTransactionBlob",resq_tx.toStyledString());

//	NBDLog("getTransactionBlob response is \n%s\n", rec.context.c_str());		
	Json::Value rep;
	Json::Reader reader;
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed!\n");	
		return -1;
	}
	std::string blob = rep["result"]["transaction_blob"].asString();
	std::string blobde;
	utils::decode_b16(blob, blobde);
	txHash = rep["result"]["hash"].asString();
		
//	NBDLog("blob = %s,\nblobde = %s!\n", blob.c_str(), blobde.c_str());
	rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "submitTransaction",
		SubmintTransaction(genesPrv, utils::encode_b16(genesPrv.Sign(blobde)), blob).toStyledString());

//	NBDLog("EChain_CreateAccount submitTransaction response is \n%s\n", rec.context.c_str());		

	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{	
		NBDLog("reader.parse failed!\n");	
		return -1;
	}
	int iResult = rep["results"][Json::UInt(0)]["error_code"].asInt();
	if(iResult != 0)
	{
		NBDLog("CreateAccount submitTransaction failed, result = %d!\n", iResult);
		if(iResult == 3) //nonce????????
		{
			g_genesisNonce++;
			goto Loop;
		}
		return -1;
 	}
 	//??????????????????????
	iRet = insertAccountInfo2DB(destAddress, privateKey, publicKey);
	if(iRet == 0)  //????????????????????????????????
		goto Again;
//	insertNnoceInfo2DB(genesAddress, iNonceNum);
	newAddress = destAddress;
//	printf("Create new account success, the new account is %s!\nHash is %s!\n", newAddress.c_str(), txHash.c_str());
	return 1;
};

//??????????????????
//????????<0,????????  >0 ????????
//??????????destAddress????????????????????????assetCode??????????, assetAmount,????????
//????????????
int EChain_ObtainAssetCode(std::string destAddress, std::string assetCode, int assetAmount)
{
	ECHAINS::HttpClient http;
	
	std::string srcPrvKey;
	std::string txHash;
		
	http.Initialize("127.0.0.1:25000");
	
	if(getPrivateKeyFromUserRegisterDB(g_genesisAddress, srcPrvKey) < 0)
	{
		NBDLog("getPrivateKeyFromUserRegisterDB failed!\n");
		return -1;
	}
	
//	NBDLog("priviteKey = %s!\n", priviteKey.c_str());
	echain::PrivateKey srcPrv(srcPrvKey);
	g_genesisNonce = EChain_GetGenesisInfo(g_genesisAddress);
Loop:
  Json::Value resq_tx = GetPaymentJson(g_genesisAddress, destAddress, g_genesisNonce, assetCode, assetAmount);
  NBDLog("GetPaymentJson request is \n%s\n", resq_tx.toStyledString().c_str());	
  Json::Value rep;
	Json::Reader reader;	
	ECHAINS::HttpClient::RecvMessage rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "getTransactionBlob",resq_tx.toStyledString());
//	NBDLog("getTransactionBlob response is \n%s\n", rec.context.c_str());		
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed!\n");	
		return -1;
	}
	
	std::string blob = rep["result"]["transaction_blob"].asString();
	std::string blobde;
	utils::decode_b16(blob, blobde);
		
	txHash = rep["result"]["hash"].asString();
	
//	NBDLog("blob = %s,\nblobde = %s!\n", blob.c_str(), blobde.c_str());
	rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "submitTransaction",
		SubmintTransaction(srcPrv, utils::encode_b16(srcPrv.Sign(blobde)), blob).toStyledString());
	
	NBDLog("EChain_ObtainAssetCode submitTransaction response is \n%s\n", rec.context.c_str());		
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{	
		NBDLog("reader.parse failed!\n");
		
		return -1;
	}
	int iResult = rep["results"][Json::UInt(0)]["error_code"].asInt();
	NBDLog("iResult = %d!\n", iResult);
	if(iResult != 0)
	{
		NBDLog("Payment submitTransaction failed, result = %d!\n", iResult);
		if(iResult == 3) //nonce????????
		{
			g_genesisNonce++;
			goto Loop;
		}
		
		return -1;
 	}
// 	txHash = rep["results"][Json::UInt(0)]["hash"].asString();
 	
 	//????????
// 	insertNnoceInfo2DB(srcAddress, iNonceNum);
 	
 //	NBDLog("The account %s gained %d token, token name is %s, Hash value is %s!\n", destAddress.c_str(), assetAmount, assetCode.c_str(), txHash.c_str());
 	
	return 1;
};

//????????
int EChain_CreateAccountX()
{
	ECHAINS::HttpClient http;
	int32_t iNonceNum = 0;
	int32_t iNonceDB = 0;
	int iRet = -1;
	std::string genesAddress;
	std::string genesPrvKey;
	std::string destAddress;
	std::string privateKey;
	std::string publicKey;
	std::string txHash;
	std::string hexInfo;	
	char *pHexData;

	http.Initialize("127.0.0.1:25000");
Again:	
	//????????????????????
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "createAccount", "");
	
	//NBDLog("createAccount response is \n%s\n", rev_accouts.context.c_str());	
	if(ParseAccountInfoFromJson(rev_accouts, destAddress, privateKey, publicKey) < 0)
	{
		NBDLog("ParseAccountInfoFromJson error!\n");
		return -1;
	}
	
	if(getPrivateKeyFromUserRegisterDB(g_genesisAddress, genesPrvKey) < 0)
	{
		NBDLog("getPrivateKeyFromUserRegisterDB failed!\n");
		return -1;
	}
	//NBDLog("EChain_CreateAccount genesPrvKey = %s!\n", genesPrvKey.c_str());
	echain::PrivateKey genesPrv(genesPrvKey);
	g_genesisNonce = EChain_GetGenesisInfo(g_genesisAddress);
Loop:
  Json::Value resq_tx = GetCreateAccountJson(genesPrv, destAddress, g_genesisNonce);
  NBDLog("CreateAccount request is \n%s\n", resq_tx.toStyledString().c_str());	
  
	ECHAINS::HttpClient::RecvMessage rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "getTransactionBlob",resq_tx.toStyledString());

	//NBDLog("getTransactionBlob response is \n%s\n", rec.context.c_str());		
	Json::Value rep;
	Json::Reader reader;
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed!\n");	
		return -1;
	}
	std::string blob = rep["result"]["transaction_blob"].asString();
	std::string blobde;
	utils::decode_b16(blob, blobde);
	txHash = rep["result"]["hash"].asString();
	
		
	//NBDLog("blob = %s,\nblobde = %s!\n", blob.c_str(), blobde.c_str());
	rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "submitTransaction",
		SubmintTransaction(genesPrv, utils::encode_b16(genesPrv.Sign(blobde)), blob).toStyledString());

	NBDLog("EChain_CreateAccount submitTransaction response is \n%s\n", rec.context.c_str());		

	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{	
		NBDLog("reader.parse failed!\n");	
		return -1;
	}
	
 	int iResult = rep["results"][Json::UInt(0)]["error_code"].asInt();
	NBDLog("iResult %d\n", iResult);
	if(iResult != 0)
	{
	//	NBDLog("CreateAccount submitTransaction failed, result = %d!\n", iResult);
		if(iResult == 3) //nonce????????
		{
			g_genesisNonce++;
			goto Loop;
		}
		else if(iResult == 99)  
		{
			g_genesisNonce = EChain_GetGenesisInfo(g_genesisAddress);
			if(g_genesisNonce < 0)
			{
				NBDLog("EChain_GetGenesisInfo failed!\n");
				return -1;
			}
			
			goto Loop;
		}
		
		return -1;
 	}

	if(EChain_ObtainAssetCode(destAddress, g_assetCode, 100) < 0)	
	{		
		NBDLog("EChain_ObtainAssetCode failed in EChain_CreateAccountX!\n");	
		return -1;	
	}
	
	//????????????????pre??????
	iRet = insertPreAccountInfo2DB(destAddress, privateKey, publicKey, txHash);
	if(iRet <= 0)  //????????????????????????????????
		goto Again;

	
//	NBDLog("Create new account success, the new account is %s!\nHash is %s!\n", blockAddress.c_str(), transactionHash.c_str());
	
	return 1;
}


int EChain_AccountPrepare()
{
	char SQLCmd[1024] = {0};
	char szAddress[64] = {0};
	int iCount = 0;
	int i = 0;
	
	g_dbmutex.Lock();
	//select a.place_id SysID, a.place_name SysName,  a.channel_id ChannelID, b.passwd Passwd from sys_places a,sys_place_users b where a.place_id = '%s' and a.place_id = b.place_id and b.user_name = '%s' and b.is_active = '1';"
	sprintf(SQLCmd, "select count(*) from preaccount_info;");
	
	mysql_ping(&g_mysql);
	int iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		
		return -1;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		g_row = mysql_fetch_row(g_res);
	//	NBDLog("33333333333  in EChain_AccountPrepare\n");
		if (g_row != NULL)
		{
			iCount = atoi(g_row[0]);
		}
		mysql_free_result(g_res);
		g_dbmutex.Unlock();
		
	}

	for(i = iCount; i < 200; i++)
	{
		if(EChain_CreateAccountX() < 0)
		{
			NBDLog("EChain_CreateAccountX failed in EChain_AccountPrepare!\n");
			return -1;
		}
	//	NBDLog("5555555 i = %d in EChain_AccountPrepare\n", i);
	usleep(1000*1000*60);
	}
	
	return 1;
}


/*	
??????????????????????????????????
?? ?? ????result: ??????????
		  	      1:????
		  	      0:userInfo????????????
		  	      -1:??????????
??????????        userID:  ????ID??
				  userType:????????
				  userInfo:  ????????????
??????????		  blockAddress: ??????????????????
				  privateKey:????????
                  publicKey: ????????
				  transHash: ??????????????
				  
*/
int EChain_UserRegister(std::string userID, int userType, std::string userInfo, std::string &blockAddress, std::string &privateKey, std::string &publicKey, std::string &transHash)
{
	int iRet = -1;
	std::string strBlockAddress;
	std::string strPrivateKey;
	std::string strPublicKey;
	std::string strTransHash;
	std::string strRecordHash;

	iRet = processUserRegisterInfo(userID, userType, userInfo, strBlockAddress, strPrivateKey, strPublicKey, strTransHash);
	if(iRet < 0) 
	{
		NBDLog("processUserRegisterInfo failed in EChain_UserRegister!\n");
		return -1;
	}
	else if(iRet == 0) 
	{
		NBDLog("conflict userID %s in user_register table!\n", userID.c_str());
		blockAddress = strBlockAddress;
		privateKey = strPrivateKey;
		publicKey = strPublicKey;
		transHash = strTransHash;
		return 0;
	}
	else
	{
		blockAddress = strBlockAddress;
		privateKey = strPrivateKey;
		publicKey = strPublicKey;
		transHash = strTransHash;
		return 1;
	}
	
}


/*	
??????????????????????
?? ?? ??????????????
		  	      1:????
		  	      0:????????????
		  	      -1:??????????
??????????
		blockAddress: ????????????????
		userName: ????????
		certificationInfo: ??????????????????json????, ????
					{ "BaseInfo":{"CompanyName":"??????????????????????????","CompanyAddress":"????????????" ,"Telephone":"0755-32134897"," Contacter":"????","Certificate":"914492903888272e3f" }}
				  	
??????????transactionHash: ??????????????
		  
*/
int EChain_UserCertification(std::string blockAddress, std::string userName, std::string certificationInfo, std::string &transactionHash)
{
	ECHAINS::HttpClient http;
	int32_t iNonceNum = 0;
	
	std::string hexInfo;	
	char *pHexData = NULL;
	std::string blockPrvKey;
	
	http.Initialize("127.0.0.1:25000");

	if(getPrivateKeyFromUserRegisterDB(blockAddress, blockPrvKey) < 0)	
	{		
		NBDLog("getPrivateKeyFromUserRegisterDB failed!\n");		
		return -1;	
	}
	echain::PrivateKey blockPrv(blockPrvKey);
ReStart:	
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getAccount?address=" + blockAddress, "");
	
//	NBDLog("getAccount response is \n%s\n", rev_accouts.context.c_str());	
	iNonceNum = ParseNonceNumFromJson(rev_accouts);
	if(iNonceNum < 0)
	{
		NBDLog("ParseNonceNumFromJson error!\n");
		return -6;
	}
	
	int iLen = certificationInfo.size();
	if(iLen <= 1)
	{
		NBDLog("certificationInfo is null in EChain_UserCertification!\n");
		certificationInfo = "{\"certificationInfo\":null}";
		iLen = certificationInfo.size();
	}
	
	pHexData = (char *)malloc(sizeof(char)*(iLen*2 + 1));
	if(pHexData == NULL)		
	{			
		NBDLog("malloc pHexData failed!\n");			
		return 0;		
	}	
	
	if(StrToHex((char *)certificationInfo.c_str(), pHexData) < 0)
	{
		NBDLog("StrToHex failed, pAddData is %s!\n", certificationInfo.c_str());
		return 0;
	}
	hexInfo = pHexData;
	
		
Loop:
	iNonceNum = iNonceNum + 1;
		
  Json::Value resq_tx = GetPaymentJsonX(blockAddress, g_destAddress, g_destAddress, hexInfo, iNonceNum, g_assetCode, 1);
  
  //NBDLog("GetPaymentJson request is \n%s\n", resq_tx.toStyledString().c_str());	
  
  Json::Value rep;
	Json::Reader reader;	
	ECHAINS::HttpClient::RecvMessage rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "getTransactionBlob",resq_tx.toStyledString());
	NBDLog("getTransactionBlob response is \n%s\n", rec.context.c_str());		
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed!\n");	
		return -6;
	}
	
	std::string blob = rep["result"]["transaction_blob"].asString();
	std::string blobde;
	utils::decode_b16(blob, blobde);
	
	transactionHash= rep["result"]["hash"].asString();
	
//	NBDLog("blob = %s,\nblobde = %s!\n", blob.c_str(), blobde.c_str());
	rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "submitTransaction",SubmintTransaction(blockPrv, utils::encode_b16(blockPrv.Sign(blobde)), blob).toStyledString());
	
	NBDLog("EChain_TransferPayment submitTransaction response is \n%s\n", rec.context.c_str());	
//	printf("EChain_TransferPayment submitTransaction response is \n%s\n", rec.context.c_str());	
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{	
		NBDLog("reader.parse failed!\n");	
		return -6;
	}
	int iResult = rep["results"][Json::UInt(0)]["error_code"].asInt();
	if(iResult != 0)
	{
	//	NBDLog("Payment submitTransaction failed, result = %d!\n", iResult);
		if(iResult == 3) //nonce????????
			goto Loop;
		else if(iResult == 99) //nonce????????
		{
			if(pHexData != NULL)
			{
				free(pHexData); 
				pHexData = NULL;
			}
			
			goto ReStart;
		}
		else if((iResult == 100)||(iResult == 104)) //????????
		{
			if(pHexData != NULL)
			{
				free(pHexData); 
				pHexData = NULL;
			}
			
			if(EChain_ObtainAssetCode(blockAddress, g_assetCode, 100) < 0)	
			{		
				NBDLog("EChain_ObtainAssetCode failed in EChain_TransferRecord!\n");	
				return -1;	
			}
			goto ReStart;
		}
		if(pHexData != NULL)
		{
			free(pHexData); 
			pHexData = NULL;
		}
		NBDLog("EChain_UserCertification failed, error_code is %d!\n", iResult);
		return -7;
 	}
 	//?????????????? 	
 	insertCertificationInfo2DB(blockAddress, userName, transactionHash);
	if(pHexData != NULL)
	{
		free(pHexData); 
		pHexData = NULL;
	}
	
	return 1;
}

/*	
??????????????????Hash??????????????????????????????
?? ?? ????<0,????????  >0 ????????
??????????transactionHash: ????Hash
??????????certificationInfo:????????????
*/
int EChain_QueryCertificationInfo(std::string transactionHash, std::string &certificationInfo)
{
	ECHAINS::HttpClient http;
	ECHAINS::HttpClient::RecvMessage rec;
	Json::Value rep;
	Json::Reader reader;
	std::string metaDataInfo;

	
	http.Initialize("127.0.0.1:25000");	
	
	rec= http.http_request(ECHAINS::HttpClient::HTTP_GET, "getTransactionHistory?hash=" + transactionHash, "");
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in EChain_QueryCreditInfo!\n");	
		return -1;
	}
	
	if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
	{
		if (rep.isMember("result") && rep["result"].isMember("transactions"))
		{
			Json::Value val_transactions = rep["result"]["transactions"];   
			int transNum = val_transactions.size();
			for(int j = 0; j < transNum; j++)
			{
				if(val_transactions[j].isMember("transaction") && val_transactions[j]["transaction"].isMember("metadata"))
				{
					std::string szHexString = val_transactions[j]["transaction"]["metadata"].asString();

					if(ParseCreditInfoFromHexString(szHexString, metaDataInfo) < 0)
					{
						NBDLog("ParseCreditInfoFromHexString failed!\n");
						return -1;
					}
					certificationInfo = metaDataInfo;
					return 1;
				}
			}
		}	
	}
	else
	{
		NBDLog("error_code is %d in EChain_QueryCreditInfo!\n", rep["error_code"].asInt());	
		return -6;
	}
	
//	NBDLog("blockHash = %s, blockHeight = %d!\n", blockHash.c_str(), blockHeight);
	return 1;
}

/*	
??????????????????????
?? ?? ????result: ??????????
				  1:????
				  0:userInfo????????????
				  -1:??????????
??????????    userID:  ????ID??
				  classID:  ????????????????ID????
??????????	  contractAddress: ????????????????ID??????????????????
				  privateKey:????????
				  publicKey: ????????
				  transHash: ??????????????
				  
*/
int EChain_ContractAddressCreate(std::string userID, std::string classID, std::string &contractAddress, std::string &privateKey, std::string &publicKey, std::string &transHash)
{
	int iRet = -1;
	std::string strContractAddress;
	std::string strPrivateKey;
	std::string strPublicKey;
	std::string strTransHash;
	std::string strRecordHash;

	iRet = processContractAccountInfo(userID, classID, strContractAddress, strPrivateKey, strPublicKey, strTransHash);
	if(iRet < 0) 
	{
		NBDLog("processContractAccountInfo failed in EChain_ContractAddressCreate!\n");
		return -1;
	}
	else if(iRet == 0) 
	{
		NBDLog("conflict userID %s classID %S in contract_account table!\n", userID.c_str(), classID.c_str());
		contractAddress = strContractAddress;
		privateKey = strPrivateKey;
		publicKey = strPublicKey;
		transHash = strTransHash;
		return 0;
	}
	else
	{
		contractAddress = strContractAddress;
		privateKey = strPrivateKey;
		publicKey = strPublicKey;
		transHash = strTransHash;
		return 1;
	}
}


/*	
??????????????????????????????
?? ?? ??????????????
  	      
1??????
          0??json????????????
          -1??????id??????
          -2????????????
          -3??????????????
          -4??????????????
          -5????????????
          -6??????????????
          -7????????????????
          -8????????????????
          -9??????????????
  	      
??????????
          contractAddress:????????
          businessType: ????????  
          dataHash: ????????
  	  dataPath: ??????????????URL????
  	  descInfo: ????????????
	      
??????????
	 transactionHash: ????Hash ????
	 result:????????
*/
int EChain_BusinessDataStorage(std::string contractAddress, int businessType, std::string dataHash, std::string dataPath, std::string descInfo, std::string &transactionHash)
{
	ECHAINS::HttpClient http;
	int32_t iNonceNum = 0;
	
	std::string hexInfo;	
	char *pHexData;
	std::string strTxHash;
	char szBarTime[64] = {0};
	std::string businessDataInfo;
		
	Json::Value blockJson;
	Json::Reader reader;
	Json::Value descJson;
	
	int iRet = 0;

	std::string blockPrvKey;
	
	http.Initialize("127.0.0.1:25000");

	if(getPrivateKeyFromContractAccountDB(contractAddress, blockPrvKey) < 0)	
	{		
		NBDLog("getPrivateKeyFromContractAccountDB failed!\n");		
		return -1;	
	}
	echain::PrivateKey blockPrv(blockPrvKey);
	
ReStart:	
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getAccount?address=" + contractAddress, "");
	
//	NBDLog("getAccount response is \n%s\n", rev_accouts.context.c_str());	
	iNonceNum = ParseNonceNumFromJson(rev_accouts);
	if(iNonceNum < 0)
	{
		NBDLog("ParseNonceNumFromJson error!\n");
		return -6;
	}
	if (!reader.parse(descInfo.c_str(), descJson)) 
	{
		NBDLog("reader.parse failed in EChain_BusinessDataStorage!\n");	
		return -1;
	}
	
	getCurTimeByString(szBarTime);
	blockJson["dataType"] = businessType;
	blockJson["operationType"] = ADDNEW;
	blockJson["dataHash"] = dataHash;
	blockJson["preHash"] = " ";
	blockJson["dataPath"] = dataPath;
	//blockJson["descInfo"] = descInfo;
	blockJson["descInfo"] = descJson;
	blockJson["version"] = 1;
	blockJson["preHash"] = " ";
	blockJson["time"] = szBarTime;
	
	businessDataInfo = blockJson.toStyledString();

	NBDLog("businessDataJsonInfo is %s in EChain_BusinessDataStorage!\n", businessDataInfo.c_str());
	
	int iLen = businessDataInfo.size();
	if(iLen <= 1)
	{
		NBDLog("businessDataJsonInfo is null in EChain_BusinessDataStorage!\n");
		businessDataInfo = "{\"businessDataInfo\":null}";
		iLen = businessDataInfo.size();
	}
	
	pHexData = (char *)malloc(sizeof(char)*(iLen*2 + 1));
	if(pHexData == NULL)		
	{			
		NBDLog("malloc pHexData failed!\n");			
		return 0;		
	}	
	
	if(StrToHex((char *)businessDataInfo.c_str(), pHexData) < 0)
	{
		NBDLog("StrToHex failed, businessDataInfo is %s!\n", businessDataInfo.c_str());
		return 0;
	}
	hexInfo = pHexData;	
		
Loop:
	iNonceNum = iNonceNum + 1;
		
  	Json::Value resq_tx = GetPaymentJsonX(contractAddress, g_destAddress, g_destAddress, hexInfo, iNonceNum, g_assetCode, 1);
  
  //printf("GetPaymentJson request is \n%s\n", resq_tx.toStyledString().c_str());	
  
  	Json::Value rep;
	
	ECHAINS::HttpClient::RecvMessage rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "getTransactionBlob",resq_tx.toStyledString());
//	printf("getTransactionBlob response is \n%s\n", rec.context.c_str());		
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed!\n");	
		if(pHexData != NULL)
		{
			free(pHexData); 
			pHexData = NULL;
		}
		return -6;
	}
	
	std::string blob = rep["result"]["transaction_blob"].asString();
	std::string blobde;
	utils::decode_b16(blob, blobde);

	strTxHash = rep["result"]["hash"].asString();
	
//	NBDLog("blob = %s,\nblobde = %s!\n", blob.c_str(), blobde.c_str());
	rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "submitTransaction",SubmintTransaction(blockPrv, utils::encode_b16(blockPrv.Sign(blobde)), blob).toStyledString());
	
//	NBDLog("EChain_TransferPayment submitTransaction response is \n%s\n", rec.context.c_str());	

	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{	
		NBDLog("reader.parse failed!\n");	
		if(pHexData != NULL)
		{
			free(pHexData); 
			pHexData = NULL;
		}
		return -6;
	}
	int iResult = rep["results"][Json::UInt(0)]["error_code"].asInt();
	if(iResult != 0)
	{
	//	NBDLog("Payment submitTransaction failed, result = %d!\n", iResult);
		if(iResult == 3) //nonce????????
			goto Loop;
		else if(iResult == 99) //nonce????????
		{
			if(pHexData != NULL)
			{
				free(pHexData); 
				pHexData = NULL;
			}
			
			goto ReStart;
		}
		else if((iResult == 100)||(iResult == 104)) //????????
		{
			if(pHexData != NULL)
			{
				free(pHexData); 
				pHexData = NULL;
			}
			
			if(EChain_ObtainAssetCode(contractAddress, g_assetCode, 100) < 0)	
			{		
				NBDLog("EChain_ObtainAssetCode failed in EChain_BusinessDataStorage!\n");	
				if(pHexData != NULL)
				{
					free(pHexData); 
					pHexData = NULL;
				}
				return -1;	
			}
			goto ReStart;
		}
		if(pHexData != NULL)
		{
			free(pHexData); 
			pHexData = NULL;
		}
		NBDLog("EChain_BusinessDataStorage failed, error_code is %d!\n", iResult);
		return -7;
 	}
	transactionHash = strTxHash;
	insertBusinessDataInfo2DB(contractAddress, ADDNEW, businessType, dataHash, transactionHash);
	if(pHexData != NULL)
	{
		free(pHexData); 
		pHexData = NULL;
	}
	
	
	return 1;
}



/*	
??????????????????????????????
?? ?? ??????????????
  	      
1??????
          0??json????????????
          -1??????id??????
          -2????????????
          -3??????????????
          -4??????????????
          -5????????????
          -6??????????????
          -7????????????????
          -8????????????????
          -9??????????????
  	      
??????????
          contractAddress:????????
          updateType: ????????
          businessType: ????????  
          preTranxHash: ??????????????????????Hash??
          dataHash: ????????
  	  dataPath: ??????????????URL????
  	  descInfo: ????????????
	     
??????????
	 transactionHash: ????Hash ????
	 result:????????		  
*/
int EChain_BusinessDataUpdate(std::string contractAddress, int updateType, int businessType, std::string preTranxHash, std::string dataHash, std::string dataPath, std::string descInfo, std::string &transactionHash)
{
	ECHAINS::HttpClient http;
	int32_t iNonceNum = 0;
	int32_t iVersion = 0;
	Json::Reader reader;
	
	std::string hexInfo;	
	char *pHexData;
	std::string transactionInfo;
	std::string strTxHash;
	char szBarTime[64] = {0};
	std::string businessDataInfo;
	Json::Value blockJson;
	Json::Value descJson;
	int iRet = 0;

	getCurTimeByString(szBarTime);
	if(ParseMetaInfoFromTransactionHash(preTranxHash, transactionInfo) < 0)
	{
		NBDLog("ParseMetaInfoFromTransactionHash failed!\n");	
		return -1;
	}

	if (!reader.parse(descInfo.c_str(), descJson)) 
	{
		NBDLog("reader.parse failed in EChain_BusinessDataUpdate!\n");	
		return -1;
	}

	Json::Value meteJson;
	if (!reader.parse(transactionInfo.c_str(), meteJson))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in parse transactionInfo!\n");	
		return -1;
	}
	iVersion = meteJson["version"].asInt();	

	blockJson["dataType"] = businessType;
	blockJson["operationType"] = updateType;
	blockJson["dataHash"] = dataHash;
	blockJson["preHash"] = preTranxHash;
	blockJson["dataPath"] = dataPath;
	//blockJson["descInfo"] = descInfo;
	blockJson["descInfo"] = descJson;
	blockJson["version"] = iVersion + 1;
	blockJson["preHash"] = preTranxHash;
	blockJson["time"] = szBarTime;

	std::string blockPrvKey;
	
	http.Initialize("127.0.0.1:25000");

	if(getPrivateKeyFromContractAccountDB(contractAddress, blockPrvKey) < 0)	
	{		
		NBDLog("getPrivateKeyFromContractAccountDB failed!\n");		
		return -1;	
	}
	echain::PrivateKey blockPrv(blockPrvKey);
ReStart:	
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getAccount?address=" + contractAddress, "");
	
//	NBDLog("getAccount response is \n%s\n", rev_accouts.context.c_str());	
	iNonceNum = ParseNonceNumFromJson(rev_accouts);
	if(iNonceNum < 0)
	{
		NBDLog("ParseNonceNumFromJson error!\n");
		return -6;
	}

	
	
	businessDataInfo = blockJson.toStyledString();
	
	int iLen = businessDataInfo.size();
	if(iLen <= 1)
	{
		NBDLog("businessDataJsonInfo is null in EChain_BusinessDataStorage!\n");
		businessDataInfo = "{\"businessDataInfo\":null}";
		iLen = businessDataInfo.size();
	}
	
	pHexData = (char *)malloc(sizeof(char)*(iLen*2 + 1));
	if(pHexData == NULL)		
	{			
		NBDLog("malloc pHexData failed!\n");			
		return 0;		
	}	
	
	if(StrToHex((char *)businessDataInfo.c_str(), pHexData) < 0)
	{
		NBDLog("StrToHex failed, businessDataInfo is %s!\n", businessDataInfo.c_str());
		return 0;
	}
	hexInfo = pHexData;	
		
Loop:
	iNonceNum = iNonceNum + 1;
		
  	Json::Value resq_tx = GetPaymentJsonX(contractAddress, g_destAddress, g_destAddress, hexInfo, iNonceNum, g_assetCode, 1);
  
  //printf("GetPaymentJson request is \n%s\n", resq_tx.toStyledString().c_str());	
  
  	Json::Value rep;
		
	ECHAINS::HttpClient::RecvMessage rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "getTransactionBlob",resq_tx.toStyledString());
//	printf("getTransactionBlob response is \n%s\n", rec.context.c_str());		
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed!\n");	
		if(pHexData != NULL)
		{
			free(pHexData); 
			pHexData = NULL;
		}
		return -6;
	}
	
	std::string blob = rep["result"]["transaction_blob"].asString();
	std::string blobde;
	utils::decode_b16(blob, blobde);

	strTxHash = rep["result"]["hash"].asString();
	
//	NBDLog("blob = %s,\nblobde = %s!\n", blob.c_str(), blobde.c_str());
	rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "submitTransaction",SubmintTransaction(blockPrv, utils::encode_b16(blockPrv.Sign(blobde)), blob).toStyledString());
	
//	NBDLog("EChain_TransferPayment submitTransaction response is \n%s\n", rec.context.c_str());	

	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{	
		NBDLog("reader.parse failed!\n");	
		if(pHexData != NULL)
		{
			free(pHexData); 
			pHexData = NULL;
		}
		return -6;
	}
	int iResult = rep["results"][Json::UInt(0)]["error_code"].asInt();
	if(iResult != 0)
	{
	//	NBDLog("Payment submitTransaction failed, result = %d!\n", iResult);
		if(iResult == 3) //nonce????????
			goto Loop;
		else if(iResult == 99) //nonce????????
		{
			if(pHexData != NULL)
			{
				free(pHexData); 
				pHexData = NULL;
			}
			
			goto ReStart;
		}
		else if((iResult == 100)||(iResult == 104)) //????????
		{
			if(pHexData != NULL)
			{
				free(pHexData); 
				pHexData = NULL;
			}
			
			if(EChain_ObtainAssetCode(contractAddress, g_assetCode, 100) < 0)	
			{		
				NBDLog("EChain_ObtainAssetCode failed in EChain_BusinessDataStorage!\n");	
				if(pHexData != NULL)
				{
					free(pHexData); 
					pHexData = NULL;
				}
				return -1;	
			}
			goto ReStart;
		}
		if(pHexData != NULL)
		{
			free(pHexData); 
			pHexData = NULL;
		}
		NBDLog("EChain_BusinessDataStorage failed, error_code is %d!\n", iResult);
		return -7;
 	}
	if(pHexData != NULL)
	{
		free(pHexData); 
		pHexData = NULL;
	}
	transactionHash = strTxHash;
	insertBusinessDataInfo2DB(contractAddress, updateType, businessType, dataHash, transactionHash);
	
	return 1;
}

/*	
??????????????????????????????
?? ?? ????<0,????????  >0 ????????
1??????
          0??json????????????
          -1??????id??????
          -2????????????
          -3??????????????
          -4??????????????
          -5????????????
          -6??????????????
          -7????????????????
          -8????????????????
          -9??????????????
  	      
??????????
          blockAddress:????????????????????
          contractAddress: ????????????
  	  descInfo: ????????????????????
??????????
	 result:????????????

*/
int EChain_ContractAddressDelete(std::string blockAddress, std::string contractAddress, std::string descInfo)
{
	char SQLCmd[1024] = {0};
	int iResult = 0;
	BusinessDataItem **ppBusinessDataInfo = NULL;
	int i = 0;
	int iRecordItems = 0;
	int iCounts = 0;
	std::string strDataPath;
	std::string strTranxHash;
	
	g_dbmutex.Lock();
	//select a.place_id SysID, a.place_name SysName,  a.channel_id ChannelID, b.passwd Passwd from sys_places a,sys_place_users b where a.place_id = '%s' and a.place_id = b.place_id and b.user_name = '%s' and b.is_active = '1';"
	sprintf(SQLCmd, "select contractaddress, businesstype, datahash, transactionhash from business_data where contractaddress='%s';", contractAddress.c_str());
	
	mysql_ping(&g_mysql);
	iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		
		return -8;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		iRecordItems = (int)mysql_num_rows(g_res);
		NBDLog("iRecordItems = %d!\n", iRecordItems);
		if(iRecordItems == 0)
		{
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			return -1;
		}

		ppBusinessDataInfo = new BusinessDataItem*[iRecordItems];
		for(i = 0; i < iRecordItems; i++)
		{
			ppBusinessDataInfo[i] = new BusinessDataItem;
			if (ppBusinessDataInfo[i] == NULL)
			{
				NBDLog("malloc ppBusinessDataInfo failed!\n");
				mysql_free_result(g_res);
				g_dbmutex.Unlock();
				return -1;
			}
			memset(ppBusinessDataInfo[i], 0, sizeof(BusinessDataItem));
		}
		while (g_row = mysql_fetch_row(g_res))
		{
			
				if (g_row[1] != NULL)
				{
					ppBusinessDataInfo[iCounts]->BusinessType = atoi(g_row[1]);
				}
				if (g_row[2] != NULL)
				{
					strcpy(ppBusinessDataInfo[iCounts]->DataHsh, g_row[2]);
				}
				if (g_row[3] != NULL)
				{
					strcpy(ppBusinessDataInfo[iCounts]->TransactionHash, g_row[3]);
				}				
				iCounts++;
		}	
		mysql_free_result(g_res);
		g_dbmutex.Unlock();
	}

	for(i = 0; i < iRecordItems; i++)
	{
		if(EChain_BusinessDataUpdate(contractAddress, DELETE, ppBusinessDataInfo[i]->BusinessType, ppBusinessDataInfo[i]->TransactionHash, ppBusinessDataInfo[i]->DataHsh, strDataPath, descInfo, strTranxHash) < 0)
		{
			NBDLog("EChain_BusinessDataUpdate failed in !\n");
			for(i = 0; i< iRecordItems; i++)
			{
				delete ppBusinessDataInfo[i];
				ppBusinessDataInfo[i] = NULL;
			}
			delete ppBusinessDataInfo;
			ppBusinessDataInfo = NULL;
			return -1;
		}
	}
	for(i = 0; i< iRecordItems; i++)
	{
		delete ppBusinessDataInfo[i];
		ppBusinessDataInfo[i] = NULL;
	}
	delete ppBusinessDataInfo;
	ppBusinessDataInfo = NULL;
	return 1;
}


/*	
??????????????????????????????????
?? ?? ????<0,????????  >0 ????????
1??????
          0??json????????????
          -1??????id??????
          -2????????????
          -3??????????????
          -4??????????????
          -5????????????
          -6??????????????
          -7????????????????
          -8????????????????
          -9??????????????
  	      
??????????
          blockAddress:????????????????????
          contractAddress:????????
          businessType: ????????  
          transactionHash: ????Hash
          dataHash: ????????
  	  dataPath: ??????????????URL????
  	  descInfo: ????????????
	     
??????????
	 result:????????????

*/
int EChain_UserIntegrityVerification(std::string blockAddress, std::string contractAddress, int businessType, std::string transactionHash, std::string dataHash)
{
	int32_t ibusinessType = 0;
	std::string strDataHash;
	std::string strDataPath;
	int32_t iVersion = 0;
	std::string transactionInfo;
	Json::Reader reader;

	if(ParseMetaInfoFromTransactionHash(transactionHash, transactionInfo) < 0)
	{
		NBDLog("ParseMetaInfoFromTransactionHash failed in EChain_UserIntegrityVerification!\n");	
		return -1;
	}

	Json::Value meteJson;
	if (!reader.parse(transactionInfo.c_str(), meteJson))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in parse transactionInfo!\n");	
		return -1;
	}
	ibusinessType = meteJson["dataType"].asInt();	
	strDataHash = meteJson["dataHash"].asString();	
	strDataPath = meteJson["dataPath"].asString();	

	NBDLog("strDataHash = %s, strDataPath = %s!\n", strDataHash.c_str(), strDataPath.c_str());
	if(strDataHash != dataHash)
	{
		NBDLog("dataHash error in EChain_UserIntegrityVerification!\n");	
		return -1;
	}	

	if(ibusinessType != businessType)
	{
		NBDLog("businessType error in EChain_UserIntegrityVerification!, businessType = %d, ibusinessType = %d!\n", businessType, ibusinessType);	
		return -2;
	}	
	
//	NBDLog("blockHash = %s, blockHeight = %d!\n", blockHash.c_str(), blockHeight);
	
	return 1;
}

/*	
????????????????????????????????
?? ?? ????<0,????????  >0 ????????
1??????
          0??json????????????
          -1??????id??????
          -2????????????
          -3??????????????
          -4??????????????
          -5????????????
          -6??????????????
          -7????????????????
          -8????????????????
          -9??????????????
  	      
??????????
          blockAddress:????????????????????
          contractAddress:????????
          businessType: ????????  
          transactionHash: ????Hash
          dataHash: ????????
??????????
	 dataPath: ??????????????URL????
	 result:????????

*/
int EChain_DataSharedContent(std::string blockAddress, std::string contractAddress, int businessType, std::string transactionHash, std::string dataHash, std::string &dataPath)
{
	int32_t ibusinessType = 0;
	std::string strDataHash;
	std::string strDataPath;
	int32_t iVersion = 0;
	Json::Reader reader;
	std::string transactionInfo;
	NBDLog("transactionHash = %s, dataHash = %s!\n", transactionHash.c_str(), dataHash.c_str());	
	if(ParseMetaInfoFromTransactionHash(transactionHash, transactionInfo) < 0)
	{
		NBDLog("ParseMetaInfoFromTransactionHash failed in EChain_UserIntegrityVerification!\n");	
		return -1;
	}
	
	Json::Value meteJson;
	if (!reader.parse(transactionInfo.c_str(), meteJson))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in parse transactionInfo!\n");	
		return -1;
	}
	ibusinessType = meteJson["dataType"].asInt();	
	strDataHash = meteJson["dataHash"].asString();	
	strDataPath = meteJson["dataPath"].asString();	

	NBDLog("strDataHash = %s, strDataPath = %s!\n", strDataHash.c_str(), strDataPath.c_str());	
	if(strDataHash != dataHash)
	{
		NBDLog("dataHash error in EChain_UserIntegrityVerification!\n");	
		return -1;
	}	

	if(ibusinessType != businessType)
	{
		NBDLog("businessType error in EChain_UserIntegrityVerification!, businessType = %d, ibusinessType = %d!\n", businessType, ibusinessType);	
		return -2;
	}	
	dataPath = strDataPath;
//	NBDLog("blockHash = %s, blockHeight = %d!\n", blockHash.c_str(), blockHeight);
	
	return 1;
}


/*	
????????????????????????????????
?? ?? ????<0,????????  >0 ????????
1??????
          0??json????????????
          -1??????id??????
          -2????????????
          -3??????????????
          -4??????????????
          -5????????????
          -6??????????????
          -7????????????????
          -8????????????????
          -9??????????????
  	      
??????????
          blockAddress:????????????????????
          contractAddress:????????
          businessType: ????????  
          transactionHash: ??????????????????????Hash??
          dataHash: ????????
	  conditionInfo: ??????????????????????????
??????????
	 dataResult: ????????????
	 result:????????????

*/

int EChain_DataSharedResult(std::string blockAddress, std::string contractAddress, int businessType, std::string transactionHash, std::string dataHash, std::string conditionInfo, std::string &dataResult)
{
	int32_t ibusinessType = 0;
	std::string strDataHash;
	std::string strDataPath;
	int32_t iVersion = 0;
	Json::Reader reader;
	std::string transactionInfo;
	if(ParseMetaInfoFromTransactionHash(transactionHash, transactionInfo) < 0)
	{
		NBDLog("ParseMetaInfoFromTransactionHash failed in EChain_DataSharedResult!\n");	
		return -1;
	}

	Json::Value meteJson;
	if (!reader.parse(transactionInfo.c_str(), meteJson))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in parse transactionInfo!\n");	
		return -1;
	}
	ibusinessType = meteJson["dataType"].asInt();	
	strDataHash = meteJson["dataHash"].asString();	
	strDataPath = meteJson["dataPath"].asString();	

	if(strDataHash != dataHash)
	{
		NBDLog("dataHash error in EChain_DataSharedResult! strDataHash = %s, dataHash = %s\n", strDataHash.c_str(), dataHash.c_str());	
		return -1;
	}	

	if(ibusinessType != businessType)
	{
		NBDLog("businessType error in EChain_DataSharedResult!, businessType = %d, ibusinessType = %d!\n", businessType, ibusinessType);	
		return -2;
	}	
	dataResult = strDataPath;
//	NBDLog("blockHash = %s, blockHeight = %d!\n", blockHash.c_str(), blockHeight);
	
	return 1;
}


//??????????????????
/*	
????????????????????????????
?? ?? ????<0,????????  >0 ????????
??????????
		blockAddress: ????????????????
		contractAddress:????????
		startTime:??????????????????
		endTime:??????????????????
		curPage: ??????????
		numsPerPage: ????????????
		
??????????businessDataInfo:????????????????
*/

int EChain_QueryAddressRecord(std::string blockAddress, std::string contractAddress, std::string startTime, std::string endTime, int curPage, int numsPerPage, std::string &businessDataInfo)
{
	char SQLCmd[2048] = {0};
	BusinessDataItem **ppBusinessDataInfo = NULL;
	int i = 0;
	int iRecordItems = 0;
	int iCounts = 0;
	
	char szuserID[64] = {0};
	char szBlockAddress[64] = {0};
	char szuserName[128] = {0};
	
	int iStartRecord = 0;
	int iEndRecord = 0;
	int iTotalPage = 0;
	int iNeedRecord = 0;
	int iRecordSeq = 0;
	
	Json::Value businessJson;
	Json::Value history;	
	Json::Value recorditems;
	Json::Value recordobj;
	
	
	int iResult;

	NBDLog("blockAddress = %s, contractAddress = %s, startTime = %s, endTime = %s in EChain_QueryConditionBusinessData!\n", blockAddress.c_str(), contractAddress.c_str(), startTime.c_str(), endTime.c_str());
	
	g_dbmutex.Lock();

	sprintf(SQLCmd, "select contractaddress, recordtype, businesstype, datahash, transactionhash, createtime from business_data where 1=1 and contractAddress = '%s'", contractAddress.c_str());
	
	char szCondition[256] = {0};

    if(startTime != "" && startTime.size() > 1)
    {
    	memset(szCondition, 0, 256);
		sprintf(szCondition, "(createtime>='%s' and createtime<='%s')",startTime.c_str(), endTime.c_str());
        sprintf(SQLCmd, "%s and %s",SQLCmd, szCondition);
    }
	
	sprintf(SQLCmd, "%s order by createtime desc;",SQLCmd);
	
	
//	sprintf(SQLCmd, "select srcclientid, srcaddress, destclientid, destaddress, tokenname, tokenamount, hash, orderid, createtime from token_transaction where srcaddress='%s' or destaddress='%d' order by createtime desc;", queryAddress.c_str(), queryAddress.c_str());
	mysql_ping(&g_mysql);
	iResult = mysql_real_query(&g_mysql,SQLCmd,(unsigned int)strlen(SQLCmd));
	if(iResult)
	{
		printf("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		NBDLog("??????????????%s,sqlcmd is %s\n",mysql_error(&g_mysql),SQLCmd);
		g_dbmutex.Unlock();
		history["businessInfo"] =	recordobj;
		history["blockAddress"] = blockAddress;
		history["contractAddress"] = contractAddress;
		history["curPage"] = curPage;
		history["numsPerPage"] = numsPerPage;
		history["totalNums"] = 0;
		history["totalPage"] = 0;
		history["result"] = -1;

		businessDataInfo = history.toStyledString();
		return -8;
	}
	else 
	{
		g_res = mysql_store_result(&g_mysql);//????????????????????
		iRecordItems = (int)mysql_num_rows(g_res);
		NBDLog("iRecordItems = %d!\n", iRecordItems);
		if(iRecordItems == 0)
		{
			mysql_free_result(g_res);
			g_dbmutex.Unlock();
			goto RESULT;
		}

		iTotalPage = (iRecordItems-1)/numsPerPage + 1;
		iStartRecord = (curPage-1)*numsPerPage;
		if(iRecordItems < (curPage-1)*numsPerPage + numsPerPage)
		{
			iEndRecord = iRecordItems - 1;
			iNeedRecord = iRecordItems - (curPage-1)*numsPerPage; 
		}
		else
		{
			iEndRecord = (curPage-1)*numsPerPage + numsPerPage - 1;
			iNeedRecord = numsPerPage;
		}
		
		ppBusinessDataInfo = new BusinessDataItem*[iNeedRecord];
		for(i = 0; i < iNeedRecord; i++)
		{
			ppBusinessDataInfo[i] = new BusinessDataItem;
			if (ppBusinessDataInfo[i] == NULL)
			{
				NBDLog("malloc ppBusinessDataInfo failed!\n");
				mysql_free_result(g_res);
				g_dbmutex.Unlock();
				history["businessInfo"] =	recordobj;
				history["blockAddress"] = blockAddress;
				history["contractAddress"] = contractAddress;
				history["curPage"] = curPage;
				history["numsPerPage"] = numsPerPage;
				history["totalNums"] = 0;
				history["totalPage"] = 0;
				history["result"] = -1;
				
				businessDataInfo = history.toStyledString();
			
				return -8;
			}
			memset(ppBusinessDataInfo[i], 0, sizeof(BusinessDataItem));
		}
		iRecordSeq = 0;
		while (g_row = mysql_fetch_row(g_res))
		{
			if(iRecordSeq == 0)
			{
				strcpy(szBlockAddress, g_row[0]);
			}
			if((iRecordSeq >= iStartRecord) && (iRecordSeq <= iEndRecord))
			{
				if (g_row[1] != NULL)
				{
					ppBusinessDataInfo[iCounts]->BusinessType = atoi(g_row[1]);
				}
				if (g_row[2] != NULL)
				{
					ppBusinessDataInfo[iCounts]->RecordType = atoi(g_row[2]);
				}
				if (g_row[3] != NULL)
				{
					strcpy(ppBusinessDataInfo[iCounts]->DataHsh, g_row[3]);
				}
				if (g_row[4] != NULL)
				{
					strcpy(ppBusinessDataInfo[iCounts]->TransactionHash, g_row[4]);
				}
				
				if (g_row[5] != NULL)
				{
					strcpy(ppBusinessDataInfo[iCounts]->DateTime, g_row[5]);
				}
					
		//	NBDLog("SrcAddress = %s, DestAddress = %s, AssetCode = %s, AssetAmount = %d!\n", 
		//					ppHistoryPayInfo[iCounts]->SrcAddress, ppHistoryPayInfo[iCounts]->DestAddress, ppHistoryPayInfo[iCounts]->AssetCode, ppHistoryPayInfo[iCounts]->AssetAmount);
				iCounts++;
			}
			iRecordSeq++;
		}
		
		mysql_free_result(g_res);
		g_dbmutex.Unlock();
	}
	NBDLog("iRecordItems = %d, iCounts = %d, iRecordSeq = %d, iNeedRecord = %d!\n", iRecordItems, iCounts, iRecordSeq, iNeedRecord);
	for(i = 0; i < iNeedRecord; i++)
	{
		recorditems["recordType"] = ppBusinessDataInfo[i]->RecordType;
		recorditems["businessType"] = ppBusinessDataInfo[i]->BusinessType;
		recorditems["dataHash"] = ppBusinessDataInfo[i]->DataHsh;
		recorditems["transactionHash"] = ppBusinessDataInfo[i]->TransactionHash;
		recorditems["txTime"] = ppBusinessDataInfo[i]->DateTime;	
		recordobj.append(recorditems);
	}

RESULT:
	history["blockAddress"] = blockAddress;
	history["contractAddress"] = contractAddress;
	history["curPage"] = curPage;
	history["numsPerPage"] = numsPerPage;
	history["totalNums"] = iRecordItems;
	history["totalPage"] = iTotalPage;
	
	history["businessInfo"] = recordobj;
	
	history["result"] = 1;
	
	businessDataInfo = history.toStyledString();
	
	if(ppBusinessDataInfo!= NULL)
	{
		for(i = 0; i< iNeedRecord; i++)
		{
			delete ppBusinessDataInfo[i];
			ppBusinessDataInfo[i] = NULL;
		}
		delete ppBusinessDataInfo;
		ppBusinessDataInfo = NULL;
	}
	
	return 1;
}

/*	
??????????????????Hash??????????????????
?? ?? ????<0,????????  >0 ????????
??????????
			transactionHash: ????Hash
??????????
			transactionInfo:????????????
			ledgerSeq:??????????
			timeInfo:????????????
*/

int EChain_QueryTransactionHashRecord(std::string transactionHash, std::string &transactionInfo, int &ledgerSeq, std::string &timeInfo)
{
	ECHAINS::HttpClient http;
	ECHAINS::HttpClient::RecvMessage rec;
	Json::Value rep;
	Json::Reader reader;
	std::string metaDataInfo;
	double dCloseTime;
	char szCloseTime[64] = {0};
	
	http.Initialize("127.0.0.1:25000");	
	
	rec= http.http_request(ECHAINS::HttpClient::HTTP_GET, "getTransactionHistory?hash=" + transactionHash, "");
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in EChain_QueryTransactionHashInfo!\n");	
		return -1;
	}
	
	if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
	{
		if (rep.isMember("result") && rep["result"].isMember("transactions"))
		{
			Json::Value val_transactions = rep["result"]["transactions"];   
			int transNum = val_transactions.size();
			for(int j = 0; j < transNum; j++)
			{
				if(val_transactions[j].isMember("transaction") && val_transactions[j]["transaction"].isMember("metadata"))
				{
					std::string szHexString = val_transactions[j]["transaction"]["metadata"].asString();

					if(ParseCreditInfoFromHexString(szHexString, metaDataInfo) < 0)
					{
						NBDLog("ParseCreditInfoFromHexString failed!\n");
						return -1;
					}
					transactionInfo = metaDataInfo;
					ledgerSeq = val_transactions[j]["ledger_seq"].asInt();	
					dCloseTime = val_transactions[j]["close_time"].asDouble();	
					transTimeToString(dCloseTime, szCloseTime);
					timeInfo = szCloseTime;
					
					return 1;
				}
			}
		}	
	}
	else
	{
		NBDLog("error_code is %d in EChain_QueryTransactionHashInfo!\n", rep["error_code"].asInt());	
		return -6;
	}
	
	
//	NBDLog("blockHash = %s, blockHeight = %d!\n", blockHash.c_str(), blockHeight);
	
	return 1;
}



/*	
????????????????????????????????????????????????????????????????????
?? ?? ????<0,????????  >0 ????????
??????????
          curPage: ??????????
		  numsPerPage: ????????????
??????????blockInfo: ????????????????????json????, ????????;
					{"totalCount":60,"blockInfo":[{"ledgerSeq":1000,"blockAge":50,"blockhash":"17663770b8c6744fdd3768e5ff6b35a08b9d285197b3f716f3a2b6d57832b2cd","blockSize":111645},
												  {"ledgerSeq":999,"blockAge":56,"blockhash":"17663770b8c6744fdd3768e5ff6b35a08b9d285197b3f716f3a2b6d57832b2cd","blockSize":111655}
																			 ]}
*/
int EChain_QueryBlockBrowserInfo(int curPage, int numsPerPage, std::string &blockInfo)
{
	int i, j;
	
	int iStartSeq=0;
	int iEndSeq = 0;
	int iTotalPage = 0;
	
	double dCloseTime;
	Json::Value blockHash;
	char szCloseTime[64] = {0};
	std::string strSeq;	
	
	int iBlockSize = 0;
	ECHAINS::HttpClient http;
	Json::Value rep;
	Json::Reader reader;	
	ECHAINS::HttpClient::RecvMessage rec;	
		
	Json::Value blockJson;
	Json::Value transItems;
	Json::Value transObj;
	int iBlockHeight;
	
	http.Initialize("127.0.0.1:25000");
	
	//????????????????????
	rec = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getModulesStatus", "");
	if(ParseBlockHeightFromJson(rec, iBlockHeight) < 0)
	{
		NBDLog("ParseBlockHeightFromJson error!\n");
		return -1;
	}
	
	iStartSeq = iBlockHeight-((curPage-1)*numsPerPage);
	if(iStartSeq >= numsPerPage)
	{
		iEndSeq = iBlockHeight-(curPage*numsPerPage) + 1;
	}
	else
	{
		iEndSeq = 1;
	}
	iTotalPage = (iBlockHeight-1)/numsPerPage + 1;
	
	NBDLog("iStartSeq = %d, iEndSeq = %d!\n", iStartSeq, iEndSeq);
	for(i=iStartSeq; i >= iEndSeq; i--)
	{		
		char szSeq[64] = {0};
		sprintf(szSeq, "%d", i);
		strSeq = szSeq;
	
		rec = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getLedger?seq=" + strSeq, "");
		if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
		{
			NBDLog("reader.parse failed in EChain_QueryBlockInfo!\n");	
			return -6;
		}
		
		if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
		{
			if (rep.isMember("result") && rep["result"].isMember("header"))
			{
				Json::Value val_header = rep["result"]["header"]; 
				blockHash = val_header["hash"].asString();
				dCloseTime = val_header["close_time"].asDouble();	
				transTimeToString(dCloseTime, szCloseTime);
				
			}  	
		}
		else
		{
			NBDLog("rep.isMember error_code = %d in EChain_QueryBlockInfo!\n", rep["error_code"].asInt());	
			return -6;
		}	
	
		rec= http.http_request(ECHAINS::HttpClient::HTTP_GET, "getTransactionHistory?ledger_seq=" + strSeq, "");
	//	printf("rec.context.c_str()	= %s!\n", rec.context.c_str());
		
		if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
		{
			NBDLog("reader.parse failed in EChain_QueryBlockInfo!\n");	
			return -6;
		}
		
		if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
		{
			iBlockSize = rec.context.size()+ BLOCKHEADER;
			transItems["ledgerSeq"] = i;
			transItems["blockAge"] = szCloseTime;
			transItems["blockhash"] = blockHash;	
			transItems["blockSize"] = iBlockSize;
		}
		else
		{
			iBlockSize = BLOCKHEADER;
			transItems["ledgerSeq"] = i;
			transItems["blockAge"] = szCloseTime;
			transItems["blockhash"] = blockHash;	
			transItems["blockSize"] = iBlockSize;
		}
		
		transObj.append(transItems);
	}

	blockJson["curPage"] = curPage;
	blockJson["numsPerPage"] = numsPerPage;
	blockJson["totalNums"] = iBlockHeight;
	blockJson["totalPage"] = iTotalPage;
	blockJson["list"] = transObj;
	blockJson["result"] = 1;
	
	blockInfo = blockJson.toStyledString();
	
	return 1;
	
}


/*	
??????????????????????????
?? ?? ????<0,????????  >0 ????????
??????????
		  blockSeq: ??????????????????
??????????TransactionInfo: ??????????????????json????
*/
int EChain_QueryBlockTransaction(int blockSeq, std::string &transactionInfo)
{
	std::string strSeq;	
	ECHAINS::HttpClient http;
	Json::Value rep;
	Json::Reader reader;	
	ECHAINS::HttpClient::RecvMessage rec; 

	std::string strTransactionInfo;	
		
	http.Initialize("127.0.0.1:25000");

	NBDLog("blockSeq = %d in EChain_QueryBlockTransaction!\n", blockSeq);	

	char szSeq[64] = {0};
	sprintf(szSeq, "%d", blockSeq);
	strSeq = szSeq;
	
	rec = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getTransactionHistory?ledger_seq=" + strSeq, "");

	NBDLog("rec = %s!\n", rec.context.c_str());	
	//transactionInfo = rec.context.c_str();
	
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed in EChain_QueryBlockTransaction!\n");	
		return -6;
	}
	
	if (rep.isMember("error_code") && rep["error_code"].asInt() == 0)
	{
		if (rep.isMember("result") && rep["result"].isMember("transactions"))
		{
			Json::Value val_result = rep["result"]; 
			transactionInfo = val_result.toStyledString();
			return 1;
		}	
	}
	else
	{
		NBDLog("rep.isMember error_code = %d in EChain_QueryBlockTransaction!\n", rep["error_code"].asInt());	
		return -6;
	}	
	
}


/*	
????????????????????????????????
?? ?? ????<0,????????  >0 ????????
??????????
		  ??
??????????chainInfo: ????????????????json????, ????????;
					{"blockHeight":12786,"blockNodes":8,"blockAccounts":678,"blockTransX":121645,"blockAmount":67800000000,"blockAverTime":60000,"blockTxPerDay":68}
*/

int EChain_QueryChainInfo(std::string &chainInfo)
{
	ECHAINS::HttpClient http;
	std::string blockinfo;
	
	
	http.Initialize("127.0.0.1:25000");
	
	//????????????????
	ECHAINS::HttpClient::RecvMessage rev_modules = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getModulesStatus", "");
	
//	NBDLog("getModulesStatus response is \n%s\n", rev_modules.context.c_str());	
	if(ParseModulesStatusInfoFromJson(rev_modules, blockinfo) < 0)
	{
		NBDLog("ParseModulesStatusInfoFromJson error!\n");
		return -1;
	}
	chainInfo = blockinfo;
	return 1;
}


/*	
??????????????????????????????nonce????
?? ?? ??????????????Nonce ???????????????? -1 
??????????
					??
??????????	
					genesAddress: ????????????
*/
int EChain_GetGenesisInfo(std::string &genesAddress)
{
	ECHAINS::HttpClient http;
	std::string genesAddr;
	int iNonceNum = 0;
		
	http.Initialize("127.0.0.1:25000");	
	//????????????????????
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getGenesisAccount", "");
	
//	NBDLog("getGenesisAccount response is \n%s\n", rev_accouts.context.c_str());	
	if(ParseGenesInfoFromJson(rev_accouts, genesAddr, iNonceNum) < 0)
	{
		NBDLog("ParseGenesInfoFromJson error!\n");
		return -1;
	}
	genesAddress = genesAddr;
	iNonceNum += 1;
	return iNonceNum;
}


/*	
????????????????????????Nonce ????
?? ?? ??????????????Nonce ???????????????? -1 
??????????
					blockAddress: ????????
??????????	
					??
*/

int EChain_GetBlockAddressNonceInfo(std::string blockAddress)
{
	ECHAINS::HttpClient http;
	std::string genesAddr;
	int iNonceNum = 0;
		
	http.Initialize("127.0.0.1:25000");	
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getAccount?address=" + blockAddress, "");
//	NBDLog("getGenesisAccount response is \n%s\n", rev_accouts.context.c_str());	
	iNonceNum = ParseNonceNumFromJson(rev_accouts);
	if(iNonceNum < 0)
	{
		NBDLog("ParseNonceNumFromJson error!\n");
		return -1;
	}
	
	return iNonceNum;
}


/*	
??????????????????????????????????????????????
?? ?? ????<0,????????????  >0 ?????????????? ??????????????????
??????????
					blockAddress:????????????????
					issuerAddress:??????????
					hexInfo:????metadata????
					assetAmount??????????????????
					assetCode????????token????????
??????????	
					transferHash:????Hash
*/
int EChain_IssueAssets(std::string blockAddress, std::string issuerAddress, std::string hexInfo, std::string assetCode, int32_t assetAmount, std::string &transferHash)
{
	ECHAINS::HttpClient http;
	std::string blockPrvKey;
	int iNonceNum = 0;
	std::string txHash;
	
	http.Initialize("127.0.0.1:25000");
	
	if(getPrivateKeyFromUserRegisterDB(issuerAddress, blockPrvKey) < 0)
	{
		NBDLog("getPrivateKeyFromUserRegisterDB failed!\n");
		return -1;
	}
//	NBDLog("genesPrvKey = %s in XChain_IssueAssets!\n", genesPrvKey.c_str());
	echain::PrivateKey blockPrv(blockPrvKey);

ReStart:	
	ECHAINS::HttpClient::RecvMessage rev_accouts = http.http_request(ECHAINS::HttpClient::HTTP_GET, "getAccount?address=" + issuerAddress, ""); 
//	NBDLog("getAccount response is \n%s\n", rev_accouts.context.c_str());	
	iNonceNum = ParseNonceNumFromJson(rev_accouts);
	if(iNonceNum < 0)
	{
		NBDLog("ParseNonceNumFromJson error!\n");
		return -6;
	}
	
Loop:
	iNonceNum = iNonceNum + 1;
	Json::Value resq_tx = GetIssueAssetJson(issuerAddress, hexInfo, iNonceNum, assetAmount, assetCode);
		
	NBDLog("IssueAsset request is \n%s\n", resq_tx.toStyledString().c_str());	
  
	ECHAINS::HttpClient::RecvMessage rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "getTransactionBlob",resq_tx.toStyledString());

	NBDLog("getTransactionBlob response is \n%s\n", rec.context.c_str());		
	Json::Value rep;
	Json::Reader reader;
	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{
		NBDLog("reader.parse failed!\n");	
		return -6;
	}
	std::string blob = rep["result"]["transaction_blob"].asString();
	std::string blobde;
	utils::decode_b16(blob, blobde);

	txHash = rep["result"]["hash"].asString();
	transferHash = txHash;
	
//	NBDLog("blob = %s,\nblobde = %s!\n", blob.c_str(), blobde.c_str());
	rec = http.http_request(ECHAINS::HttpClient::HTTP_POST, "submitTransaction",
		SubmintTransaction(blockPrv, utils::encode_b16(blockPrv.Sign(blobde)), blob).toStyledString());

	NBDLog("EChain_IssueAssets submitTransaction response is \n%s\n", rec.context.c_str()); 	

	if (!reader.parse(rec.context.c_str(), rep))  // reader??Json????????????rep2??rep2??????Json????????????	
	{	
		NBDLog("reader.parse failed!\n");	
		return -6;
	}
	if(rep["results"][Json::UInt(0)]["error_code"].asInt() != 0)
	{
		NBDLog("IssueAssets submitTransaction failed!\n");
		return -6;
	}

	int iResult = rep["results"][Json::UInt(0)]["error_code"].asInt();
	if(iResult != 0)
	{
		NBDLog("Payment submitTransaction failed, result = %d!\n", iResult);
		if(iResult == 3) //nonce????????
			goto Loop;
		else if(iResult == 99) //nonce????????
		{
			goto ReStart;
		}
		
		NBDLog("IssueAssets submitTransaction failed, error_code = %d!\n", iResult);
		return -7;
	}

	return 1;

}


#endif

