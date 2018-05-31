#include "stdafx.h"

#include "B64.h"
#include <memory>

char CPNet::B64::encodingTable[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/' };

char CPNet::B64::decodingTable[256];
bool CPNet::B64::init_ = false;
//bool CPNet::B64::isAvailable_ = false;

std::string CPNet::B64::encode(char const* inputData, unsigned int in_len) {

	auto outSize = (in_len / 3) * 4;
	if (in_len % 3 != 0) {
		outSize += 4;
	}

	std::unique_ptr<char[]> buff(new char[outSize]);
	char *tmp = buff.get();

	for (int runs = in_len / 3; runs > 0; runs--) {
		/*int d1 = (*inputData);
		int d2 = (*inputData++);
		int d3 = (*inputData);
		int d4 = (*inputData++);
		int d5 = (*inputData);
		int d6 = (*inputData++);
		int tmp1 = ((d1 & 0xfc) >> 2);
		int tmp2 = (((d2 & 0x03) << 4) + ((d3 & 0xf0) >> 4));
		int tmp3 = (((d4 & 0x0f) << 2) + ((d5 & 0xc0) >> 6));
		int tmp4 = (d6 & 0x3f);*/
		/*int tmp1 = ;
		int tmp2 = ;
		int tmp3 = ;
		int tmp4 = ;*/

		/*(*tmp++) = encodingTable[(((*inputData) & 0xfc) >> 2)];
		auto stupidUndefinedInstructionOrder = (*inputData++);
		(*tmp++) = encodingTable[(((stupidUndefinedInstructionOrder & 0x03) << 4)) + (((*inputData) & 0xf0) >> 4)];
		stupidUndefinedInstructionOrder = (*inputData++);
		(*tmp++) = encodingTable[(((stupidUndefinedInstructionOrder & 0x0f) << 2) + (((*inputData) & 0xc0) >> 6))];
		(*tmp++) = encodingTable[((*inputData++) & 0x3f)];*/

		(*tmp++) = encodingTable[((inputData[0] & 0xfc) >> 2)];
		(*tmp++) = encodingTable[(((inputData[0] & 0x03) << 4)) + ((inputData[1] & 0xf0) >> 4)];
		(*tmp++) = encodingTable[(((inputData[1] & 0x0f) << 2) + ((inputData[2] & 0xc0) >> 6))];
		(*tmp++) = encodingTable[(inputData[2] & 0x3f)];

		inputData += 3;
	}

	if ((in_len % 3) == 1) {
		(*tmp++) = encodingTable[((inputData[0] & 0xfc) >> 2)];
		(*tmp++) = encodingTable[((inputData[0] & 0x03) << 4) + (('\0' & 0xf0) >> 4)];
		(*tmp++) = '=';
		(*tmp++) = '=';
	}
	if ((in_len % 3) == 2) {
		(*tmp++) = encodingTable[(inputData[0] & 0xfc) >> 2];
		(*tmp++) = encodingTable[((inputData[0] & 0x03) << 4) + ((inputData[1] & 0xf0) >> 4)];
		(*tmp++) = encodingTable[((inputData[1] & 0x0f) << 2) + (('\0' & 0xc0) >> 6)];
		(*tmp++) = '=';
	}


	std::string ret(buff.get(), outSize);

	return ret;
}

int CPNet::B64::b64DecodeSize(int b64StringSize) {
	return b64StringSize / 4 * 3 + 1;
}


int CPNet::B64::decode(std::string inputData, char &outBuff, int outBuffSize) {


	if (inputData.size() > 4 && inputData.at(inputData.size() - 1) == '\n') {
		inputData.pop_back();
	}


	int inputLen = inputData.size();
	const char *inputDataC = inputData.c_str();

	if (!init_) {
		init_ = true;
		for (int i = 0; i < 64; i++)
			decodingTable[(uint8_t)encodingTable[i]] = (char)i;
	}

	int outputLen = outBuffSize;
	if ((outputLen) != inputLen / 4 * 3 + 1) {
		return -1;
	}


	if (inputLen % 4 != 0) return -1;

	if (inputDataC[inputLen - 1] == '=') (outputLen)--;
	if (inputDataC[inputLen - 2] == '=') (outputLen)--;

	for (int i = 0, j = 0; i < inputLen;) {

		uint32_t sextet_a = inputDataC[i] == '=' ? 0 & i++ : decodingTable[(uint8_t)inputDataC[i++]];
		uint32_t sextet_b = inputDataC[i] == '=' ? 0 & i++ : decodingTable[(uint8_t)inputDataC[i++]];
		uint32_t sextet_c = inputDataC[i] == '=' ? 0 & i++ : decodingTable[(uint8_t)inputDataC[i++]];
		uint32_t sextet_d = inputDataC[i] == '=' ? 0 & i++ : decodingTable[(uint8_t)inputDataC[i++]];

		uint32_t triple = (sextet_a << 3 * 6)
			+ (sextet_b << 2 * 6)
			+ (sextet_c << 1 * 6)
			+ (sextet_d << 0 * 6);

		if (j < outputLen) (&outBuff)[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < outputLen) (&outBuff)[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < outputLen) (&outBuff)[j++] = (triple >> 0 * 8) & 0xFF;
	}

	//do not remove this line, otherwise the app might crash!
	(&outBuff)[outputLen-1] = '\0';

	return outputLen;
}

void CPNet::B64::init() {
	if (!init_) {
		init_ = true;
		for (int i = 0; i < 64; i++)
			decodingTable[(uint8_t)encodingTable[i]] = (char)i;

		/*isAvailable_ = CPNet::B64::test();
		if (!isAvailable_) {
			CPLOG_ERROR << "B64 TEST ERROR";
		}*/
	}
}

/*bool CPNet::B64::isAvailable() {
	return isAvailable_;
}*/
