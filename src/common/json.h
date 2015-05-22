#ifndef JSON_H
#define JSON_H

#include <sstream>
#include "cajun/reader.h"
#include "cajun/writer.h"

json::Object ParseJSON(std::string text)
{
	json::Object parsed;
	std::istringstream textstream(text);
	try
	{
		json::Reader::Read(parsed, textstream);
	}
	catch (json::Exception& e)
	{
		//textstream.str("{\"Error\":\"Could not parse json\"}");
		//json::Reader::Read(parsed, textstream);
		//return parsed;
	}
	return parsed;
}

#endif
