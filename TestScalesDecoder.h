#pragma once
#include "NScalesDecoder.h"
#include "NLog.h"
class TestScalesDecoder
{
public:
	static bool Test(NLog *log, ScalesSettings scales) {//entry point
		Measurement w;
		double weight = 0.0;
		switch (scales.deviceType) {
		case DeviceType::CAS6000:
		{
			weight = 1330.0;			
			byte buffer[] = {0,0,0,0,0,0,0,0,0,0,0,0,
				0x53 ,0x54 ,0x2C ,0x47 ,0x53 ,0x2C ,0x44 ,0xC4 ,0x2C ,0x20 ,0x20 ,0x20 ,0x20 ,0x31 ,0x2E ,0x33 ,0x33 ,0x00 ,0x20 ,0x74 ,0x0D ,0x0A,
				0,0,0,0,0,0,0,0,0,0,0,0 }; //1330
			w = NScalesDecoder::DecodeWeight(scales.deviceType, buffer, scales.messageSize * 2);
		}
		break;
		case DeviceType::BU4263M1:
		{			
			weight = 1480.0;			
			byte buffer[] = { 0,0,0,
				0x23, 0x4E, 0x94, 0x00, 0x00, 0x24,
				0,0,0};
			w = NScalesDecoder::DecodeWeight(scales.deviceType, buffer, scales.messageSize * 2);
		}
		break;
		case DeviceType::IND310:
		{
			weight = 1380.0;
			byte buffer[] = { 0,0,0, 0,0,0, 0,0,0,
				0x0, 0x01, 0x29, 0x30, 0x21, 0x20, 0x20, 0x31, 0x33, 0x38, 0x30, 0x20, 0x20, 0x20, 0x20, 0x30, 0x30, 0x0D,
				0,0,0, 0,0,0, 0,0,0 };
			w = NScalesDecoder::DecodeWeight(scales.deviceType, buffer, scales.messageSize * 2);
		}
		break;
		case DeviceType::HBT9:
		{
			weight = 12.146;
			//wn0012.146kg\r\n
			byte buffer[] = {".......wn0012.146kg\r\nwn0012.146kg\r\n......."};
			w = NScalesDecoder::DecodeWeight(scales.deviceType, buffer, scales.messageSize * 2);
		}
		break;
		case DeviceType::FT11:
		{
			weight = 1360.0;
			byte buffer[] = { 
				0,0,0, 0,0,0, 0,0,0,
				0x0E  ,0x02  ,0x69  ,0x70  ,0x30  ,0x20  ,0x20  ,0x31  ,0x33  ,0x36  ,0x30  ,0x20  ,0x20  ,0x20  ,0x20  ,0x20  ,0x30  ,0x0D,
				0,0,0, 0,0,0, 0,0,0 };
			w = NScalesDecoder::DecodeWeight(scales.deviceType, buffer, scales.messageSize * 2);
		}
		break;
		case DeviceType::VT220: //????????????????
		{
			//T+123.45
			weight = 123450.0;//kgs
			byte buffer[] = {".....T+123.45\r...."};
			w = NScalesDecoder::DecodeWeight(scales.deviceType, buffer, scales.messageSize * 2);
		}
		break;
		};

		log->LogF(TEXT("(%s) TestScalesDecoder(%s): %f decoded as %f"), (w.weight == weight)?TEXT("PASSED"):TEXT("  BAD "), scales.name, weight, w.weight);
		if (w.weight != weight) return false;
		else return true;		
	}
};

