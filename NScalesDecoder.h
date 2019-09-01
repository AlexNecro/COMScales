#pragma once
#include "base.h"
#include <vector>

/*
	this is java-style enum implementation:
*/
struct ScalesSettings {
	int messageSize;
	DeviceType deviceType;
	ATL::CString name;

	ScalesSettings(DeviceType deviceType, ATL::CString name, int messagesize);

	static ScalesSettings& getByType(DeviceType deviceType);
	static ScalesSettings& getByName(ATL::CString name);
	static ATL::CString getAllNames();

	//constants:
	static ScalesSettings BU4263M1;
	static ScalesSettings CAS6000;
	static ScalesSettings FT11;
	static ScalesSettings HBT9;
	static ScalesSettings IND310;
	static ScalesSettings VT220;
	static ScalesSettings Test;

	static std::vector<ScalesSettings*>* all;//non-pointer value accindently disappears =-O
	static int count;
};

/*
object-style was redunant here;
all scale-depended classes was merget to this one because they had almost no code
*/
class NScalesDecoder
{
public:
	NScalesDecoder();
	~NScalesDecoder();
	static Measurement DecodeWeight(DeviceType deviceType, byte* buffer, int bytesRead);//entry point
	static Measurement DecodeWeightTest(byte* buffer, int bytesRead);
	static Measurement DecodeWeightBU4263M1(byte* buffer, int bytesRead, const ScalesSettings& settings);
	static Measurement DecodeWeightCAS6000(byte* buffer, int bytesRead, const ScalesSettings& settings);
	static Measurement DecodeWeightFT11(byte* buffer, int bytesRead, const ScalesSettings& settings);
	static Measurement DecodeWeightHBT9(byte* buffer, int bytesRead, const ScalesSettings& settings);
	static Measurement DecodeWeightIND310(byte* buffer, int bytesRead, const ScalesSettings& settings);
	static Measurement DecodeWeightVT220(byte* buffer, int bytesRead, const ScalesSettings& settings);
};

