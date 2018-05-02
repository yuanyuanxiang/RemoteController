#include "tinyxml.h"
#include "../Xmldef.h"

int main()
{
	char sip[256], buffer[1024], *ptr = buffer + 256;
	sprintf(ptr, VDU_VduRegister("1234567890ABCDEF", 
		"VDU001", 
		"123456", 
		"127.0.0.1", 
		5000));
	int nLen = strlen(buffer);
	sprintf(sip, SIP_RequestHeader("192.168.1.1", "192.168.1.2", 
		CreateCallId(), CreateCSeq(), nLen));
	ptr -= strlen(sip);
	strncpy(ptr, sip, strlen(sip));
	const char* xml = strstr(ptr, "<?xml");

	TiXmlDocument * xmlDocument = new TiXmlDocument();
	xmlDocument->Parse(xml, 0, TIXML_DEFAULT_ENCODING);
	TiXmlElement * rootElement = xmlDocument->RootElement();
	TiXmlAttribute* attri = rootElement->FirstAttribute();
	const char *response = rootElement->Value();
	const char *command = attri->Name();
	const char *cmdType = attri->Value();
	printf("%s %s = \"%s\"\n", response, command, cmdType);

	const char buf2[] = XMLHEADER\
		"<response command=\"VduRegister\">\r\n"\
		"<result code=\"0\">success</result>\r\n"\
		"<parameters>\r\n"\
		"<keepAlivePeriod>XXXXXXX</keepAlivePeriod>\r\n"\
		"<currentTime>yyyy-MM-dd hh:mm:ss</currentTime>\r\n"\
		"</parameters>\r\n"\
		"</response >\r\n";

	TiXmlDocument * xmlDocument2 = new TiXmlDocument;
	xmlDocument2->Parse(buf2, 0, TIXML_DEFAULT_ENCODING);
	rootElement = xmlDocument2->RootElement();
	const char *response2 = rootElement->Value();
	TiXmlElement* result = rootElement->FirstChildElement("result");
	const char *value = result->FirstAttribute()->Value();
	TiXmlElement* parameters = rootElement->FirstChildElement("parameters");
	TiXmlElement* keepAlivePeriod = parameters->FirstChildElement("keepAlivePeriod");
	const char *text = keepAlivePeriod->GetText();

	system("PAUSE");
	delete xmlDocument;
	delete xmlDocument2;

	return 0;
}