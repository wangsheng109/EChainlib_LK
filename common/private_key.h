/*
Copyright echain Technologies Co., Ltd. 2017 All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef PRIVATE_KEY_H_
#define PRIVATE_KEY_H_

#include <utils/headers.h>
#include <3rd/ed25519.h>
#include <ec/ecc_sm2.h>

namespace echain {
	typedef unsigned char sm2_public_key[65];
	typedef const unsigned char* puchar;
	enum SignatureType {
		SIGNTYPE_NONE,
		SIGNTYPE_ED25519 = 1,
		SIGNTYPE_CFCASM2 = 2,
		SIGNTYPE_RSA = 3,
		SIGNTYPE_CFCA = 4
	};

	enum PrivateKeyPrefix {
		ADDRESS_PREFIX = 0xea, //0xa0
		PUBLICKEY_PREFIX = 0xeb, //0xb0
		PRIVATEKEY_PREFIX = 0xec  //0xc0
	};

	enum Ed25519KeyLength {
		ED25519_ADDRESS_LENGTH = 20, // 1+1+20+1
		ED25519_PUBLICKEY_LENGTH = 32, //1+1+32+1
		ED25519_PRIVATEKEY_LENGTH = 32, //1+1+32+1
	};

	enum Sm2KeyLength {
		SM2_ADDRESS_LENGTH = 20, //1+1+20+1
		SM2_PUBLICKEY_LENGTH = 65, //1+1+65+1
		SM2_PRIVATEKEY_LENGTH = 32 //1+1+32+1
	};

	bool GetKeyElement(const std::string &base16_pub_key, PrivateKeyPrefix &prefix, SignatureType &sign_type, std::string &raw_data);
	std::string GetSignTypeDesc(SignatureType type);
	SignatureType GetSignTypeByDesc(const std::string &desc);

	class PublicKey {
		DISALLOW_COPY_AND_ASSIGN(PublicKey);
		friend class PrivateKey;

	public:
		PublicKey();
		PublicKey(const std::string &base16_pub_key);
		~PublicKey();

		void Init(std::string rawpkey);

		//返回base58编码之后的地址
		std::string GetBase16Address() const;

		//返回公钥的base58编码
		std::string GetBase16PublicKey() const;

		std::string GetRawPublicKey() const;

		bool IsValid() const { return valid_; }

		SignatureType GetSignType() { return type_; };
		std::string CalcHash(const std::string &value) const;

		static bool Verify(const std::string &data, const std::string &signature, const std::string &public_key_base16);
		static bool IsAddressValid(const std::string &public_key_base16);
	private:
		std::string raw_pub_key_;
		bool valid_;
		SignatureType type_;
	};

	class PrivateKey {
		DISALLOW_COPY_AND_ASSIGN(PrivateKey);
	public:
		PrivateKey(SignatureType type);
		PrivateKey(const std::string &base16_private_key);
		bool From(const std::string &base16_private_key);
		~PrivateKey();


		std::string	Sign(const std::string &input) const;
		std::string GetBase16PrivateKey() const;
		std::string GetBase16Address() const;
		std::string GetBase16PublicKey() const;
		std::string GetRawPublicKey() const;
		bool IsValid() const { return valid_; }
		std::string GetRawPrivateKey() {
			return utils::String::BinToHexString(raw_priv_key_);
		}
		SignatureType GetSignType() { return type_; };
		std::string CalcHash(const std::string &value) const;

	private:
		std::string raw_priv_key_;
		bool valid_;
		SignatureType type_;
		PublicKey pub_key_;
		static utils::Mutex lock_;
	};
};

#endif
