#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdexcept>

#include "CgiUtils.h"
#include "FormEntry.h"
#include "FormFile.h"
#include "CgiRequest.h"
#include "Lang.h"

   
  // ============================================================
  // Function copy_if (handy, missing from STL)
  // ============================================================
  // This code taken directly from 
  // "The C++ Programming Language, Third Edition" by Bjarne Stroustrup
  template<class In, class Out, class Pred>
  Out 
  copy_if(In first, 
	  In last, 
	  Out res, 
	  Pred p)
  {
    while(first != last) {
      if(p(*first))
	*res++ = *first;
      ++first;
    }
    return res;
  }
  

using namespace cgicc;

CgiRequest::CgiRequest()
{	
	html_path = "/var/www/html/";
	upload_path = "/updtmp/";
	outputHTTPResponseHeaders = false;
	
	acquireEnv();

	// get the post data
	if(stringsAreEqual(fRequestMethod, "post")) {
 		std::vector<char> data(fContentLength);
    
		if ( getContentLength() )
		{
			std::cin.read( &data[0], getContentLength() );
			
			if ( std::cin.gcount() != getContentLength() )
				throw std::runtime_error("I/O error");

			fPostData = std::string( &data[0], getContentLength() );
			
			parseFormInput(fPostData, fContentType);
		} 
  	} 

	parseFormInput(fQueryString);

}

void CgiRequest::acquireEnv()
{
	  fServerSoftware 		= getEnv("SERVER_SOFTWARE");
	  fServerName 		= getEnv("SERVER_NAME");
	  fGatewayInterface 	= getEnv("GATEWAY_INTERFACE");
	  fServerProtocol 		= getEnv("SERVER_PROTOCOL");

	  std::string port 		= getEnv("SERVER_PORT");
	  fServerPort 			= std::atol(port.c_str());

	  fRequestMethod 		= getEnv("REQUEST_METHOD");
	  fScriptName 			= getEnv("SCRIPT_NAME");
	  fQueryString 		= getEnv("QUERY_STRING");

	  fRemoteAddr 		= getEnv("REMOTE_ADDR");

	  fContentType 		= getEnv("CONTENT_TYPE");

	  std::string length 	= getEnv("CONTENT_LENGTH");
	  fContentLength 		= std::atol(length.c_str());
	  
	  fUserAgent 			= getEnv("HTTP_USER_AGENT");
}


form_iterator 
CgiRequest::getElement(const std::string& name)
{
  return std::find_if(fFormData.begin(), fFormData.end(), FE_nameCompare(name));
}

const_form_iterator 
CgiRequest::getElement(const std::string& name) 		const
{
  return std::find_if(fFormData.begin(), fFormData.end(),FE_nameCompare(name));
}

bool 
CgiRequest::getElement(const std::string& name, 
			 std::vector<FormEntry>& result) 	const
{ 
  return findEntries(name, true, result); 
}

form_iterator 
CgiRequest::getElementByValue(const std::string& value)
{
  return std::find_if(fFormData.begin(), fFormData.end(),
		      FE_valueCompare(value));
}

const_form_iterator 
CgiRequest::getElementByValue(const std::string& value) 	const
{
  return std::find_if(fFormData.begin(), fFormData.end(), 
		      FE_valueCompare(value));
}

bool 
CgiRequest::getElementByValue(const std::string& value, 
				std::vector<FormEntry>& result)	const
{ 
  return findEntries(value, false, result); 
}

std::string 
CgiRequest::getReqValue(const std::string& field_name)
{
	const_form_iterator value = getElement(field_name);

	if(value != getElements().end() && value->getValue().empty() == false)
		return(value->getValue());
	else
		return(std::string(""));

}

bool
CgiRequest::findEntries(const std::string& param, 
			  bool byName,
			  std::vector<FormEntry>& result) 	const
{
  // empty the target vector
  result.clear();

  if(byName) {
    copy_if(fFormData.begin(), fFormData.end(), 
	    std::back_inserter(result),FE_nameCompare(param));
  }
  else {
    copy_if(fFormData.begin(), fFormData.end(), 
	    std::back_inserter(result), FE_valueCompare(param));
  }

  return false == result.empty();
}

void
CgiRequest::parseFormInput(const std::string& data,const std::string &content_type)
{
  
  std::string standard_type		= "application/x-www-form-urlencoded";
  std::string multipart_type 	= "multipart/form-data";

  // Don't waste time on empty input
  if(true == data.empty())
    return;

  // Standard content type = application/x-www-form-urlencoded
  // It may not be explicitly specified
  if(true == content_type.empty() 
     || stringsAreEqual(content_type, standard_type,standard_type.length())) {
    std::string name, value;
    std::string::size_type pos;
    std::string::size_type oldPos = 0;

    // Parse the data in one fell swoop for efficiency
    while(true) {
      // Find the '=' separating the name from its value
      pos = data.find_first_of('=', oldPos);
      
      // If no '=', we're finished
      if(std::string::npos == pos)
	break;
      
      // Decode the name
      name = form_urldecode(data.substr(oldPos, pos - oldPos));
      oldPos = ++pos;
      
      // Find the '&' separating subsequent name/value pairs
      pos = data.find_first_of('&', oldPos);
      
      // Even if an '&' wasn't found the rest of the string is a value
      value = form_urldecode(data.substr(oldPos, pos - oldPos));

      // Store the pair
      fFormData.push_back(FormEntry(name, value));
      
      if(std::string::npos == pos)
	break;

      // Update parse position
      oldPos = ++pos;
    }
  }
  // File upload type = multipart/form-data
  else if(stringsAreEqual(multipart_type, content_type,
			  multipart_type.length())){
    // Find out what the separator is
    std::string 		bType 	= "boundary=";
    std::string::size_type 	pos 	= content_type.find(bType);

    // generate the separators
    std::string sep1 = content_type.substr(pos + bType.length());
    sep1.append("\r\n");
    sep1.insert(0, "--");

    std::string sep2 = content_type.substr(pos + bType.length());
    sep2.append("--\r\n");
    sep2.insert(0, "--");

    // Find the data between the separators
    std::string::size_type start  = data.find(sep1);
    std::string::size_type sepLen = sep1.length();
    std::string::size_type oldPos = start + sepLen;

    while(true) {
      pos = data.find(sep1, oldPos);

      // If sep1 wasn't found, the rest of the data is an item
      if(std::string::npos == pos)
	break;

      // parse the data
      parseMIME(data.substr(oldPos, pos - oldPos));

      // update position
      oldPos = pos + sepLen;
    }

    // The data is terminated by sep2
    pos = data.find(sep2, oldPos);
    // parse the data, if found
    if(std::string::npos != pos) {
      parseMIME(data.substr(oldPos, pos - oldPos));
    }
  }
}

MultipartHeader
CgiRequest::parseHeader(const std::string& data)
{
  std::string disposition;
  disposition = extractBetween(data, "Content-Disposition: ", ";");
  
  std::string name;
  name = extractBetween(data, "name=\"", "\"");
  
  std::string filename;
  filename = extractBetween(data, "filename=\"", "\"");

  std::string cType;
  cType = extractBetween(data, "Content-Type: ", "\r\n\r\n");

  // This is hairy: Netscape and IE don't encode the filenames
  // The RFC says they should be encoded, so I will assume they are.
  filename = form_urldecode(filename);

  //remove the dos path
  std::string::size_type	pos = filename.find("\\", 0);
  while(std::string::npos != pos)
  {
	filename = filename.substr(pos+1, filename.size());
	pos = filename.find("\\", 0);
  }
	
  return MultipartHeader(disposition, name, filename, cType);
}

void
CgiRequest::parseMIME(const std::string& data)
{
  // Find the header
  std::string end = "\r\n\r\n";
  std::string::size_type headLimit = data.find(end, 0);
  
  // Detect error
  if(std::string::npos == headLimit)
    throw std::runtime_error("Malformed input");

  // Extract the value - there is still a trailing CR/LF to be subtracted off
  std::string::size_type valueStart = headLimit + end.length();

  int data_len = data.length() - valueStart - 2;

  // Parse the header - pass trailing CR/LF x 2 to parseHeader
  MultipartHeader head = parseHeader(data.substr(0, valueStart));

  if(head.getFilename().empty())
  {
	std::string value = data.substr(valueStart, data.length() - valueStart - 2);
  	fFormData.push_back(FormEntry(head.getName(), value));
  } else if(!head.getName().empty()) {
  	std::string save_filename = upload_path + head.getFilename();
	std::ofstream save_file(save_filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	if(!save_file)
		throw std::runtime_error("can't save upload file.");
	
	std::cerr << "Got file length:" << data_len << std::endl;

  	save_file.write(data.data() + valueStart, data_len);

	save_file.close();
	 
	fFormFiles.push_back(FormFile(head.getName(), 
				  head.getFilename(), 
				  head.getContentType(), 
				  data_len));
  }
}

void CgiRequest::output()
{
}

void CgiRequest::outputError(const std::string & outstring)
{
	if(outputHTTPResponseHeaders == false) {
		std::cout << "Content-type:text/html\r\n\r\n";
		outputHTTPResponseHeaders = true;
	}
	std::cout << "<html> <head> " << std::endl;
	std::cout << "	<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\";>" << std::endl;
	std::cout << "	<link href=\"/css/normal.css\" type=text/css rel=StyleSheet>" << std::endl;
	std::cout << "</head> <body>" << std::endl;
	std::cout << "<br> <blockquote> " << outstring <<"</blockquote>" << std::endl;
	std::cout << "</body> </html>" << std::endl;
}

int CgiRequest::outputHTMLFile(const std::string & filename, bool replace)
{
	if(outputHTTPResponseHeaders == false) {
		std::cout << "Content-type:text/html\r\n\r\n";
		outputHTTPResponseHeaders = true;
	}
	return(appendHTMLFile(filename, replace));
}


void CgiRequest::redirectHTMLFile(const std::string & filename)
{
	std::cout << "Content-type:text/html\r\n\r\n";

	std::cout << "<html><head>";
	std::cout << "<meta http-equiv=\"refresh\" content=\"0;url=";
	std::cout << filename <<"\">" << std::endl;
	std::cout << "</head></html>";
	
}

void CgiRequest::redirectHTMLFile(const std::string & filename, const std::string & param)
{
	std::cout << "Content-type:text/html\r\n\r\n";

	std::cout << "<html><head>";
	std::cout << "<meta http-equiv=\"refresh\" content=\"0;url=";
	std::cout << filename << "?" << param<< "\">" << std::endl;
	std::cout << "</head></html>";
	
}


int CgiRequest::appendHTMLFile(const std::string & filename, bool replace)
{
	std::string full_name = html_path + filename;
  	std::ifstream html_file( full_name.c_str(), std::ios::in );

	if( !html_file )
		return -1;
	
	while (html_file&&!html_file.eof ())
	{
		char buffer[2048];

		html_file.getline (buffer, sizeof (buffer));

		if(replace) {
			std::string content = std::string(buffer);

			//fill the multi-language first
			fillLabelInHtmlLine(content);
			
			//fill the defined variable
			fillVarInHtmlLine(content);
			std::cout << content << std::endl;
		}
		else
			std::cout << buffer <<std::endl;
	}
	
	html_file.close();

	return 0;
}

std::string 
CgiRequest::getFieldValue(const std::string& field_name)
{
	form_iterator value = getElement(field_name);

	//First try to retrieve from request form data 
	if(value != getElements().end() && value->getValue().empty() == false)
		return(value->getValue());
	else
	{	
		// if not found in form data, then try to retrieve from user defined variables
		value  = std::find_if(userVariables.begin(), userVariables.end(),FE_nameCompare(field_name));

		if(value != userVariables.end() && value->getValue().empty() == false)
			return(value->getValue());
		else			
			return(std::string(""));
	}
}

bool CgiRequest::fillVarInHtmlLine(std::string&data)
{
	std::string var_name;
  	std::string::size_type start, limit;
  
  	start = data.find("${", 0);
  	if(std::string::npos == start) 
		return true; 	//no variable found

	limit = data.find("}", start);
    	if(std::string::npos == limit)
		return false;  //no variable close found
		
      var_name = data.substr(start +2, limit -start -2);

	if(var_name.empty())
		return false; 	//variable is illeagle

	std::string var_value = getFieldValue(var_name);

	data.replace(start, limit -start + 1, var_value);
	
	return(fillVarInHtmlLine(data));
}

bool CgiRequest::fillLabelInHtmlLine(std::string&data)
{
	std::string var_name;
  	std::string::size_type start, limit;
  
  	start = data.find("${_L_", 0);
  	if(std::string::npos == start) 
		return true; 	//no variable found

	limit = data.find("}", start);
    	if(std::string::npos == limit)
		return false;  //no variable close found
		
      var_name = data.substr(start +5, limit -start -5);

	if(var_name.empty())
		return false; 	//variable is illeagle

	data.replace(start, limit -start + 1, multilang::getLabel(var_name.c_str()));
	
	return(fillLabelInHtmlLine(data));
}

void 
CgiRequest::addAdminStateVariable(const std::string& name, unsigned char value)			
{ 
    	std::string data;

	data = "<select class='fixsize' name='";
	data += name + "' id='" + name + "'>";

	if(value == 2) {
		data += "<option value='2' selected>";
		data = data + multilang::getAdminState(0) + "</option>";
		data += "<option value='1'>";
		data = data + multilang::getAdminState(1) + "</option></select>";
	} else {
		data += "<option value='2'>";
		data = data + multilang::getAdminState(0) + "</option>";
		data += "<option value='1' selected>";
		data = data + multilang::getAdminState(1) + "</option></select>";
	}
		
    	userVariables.push_back(cgicc::FormEntry(name, data));
}


std::string 
CgiRequest::getReqUploadFilename(const std::string& name)
{
	formfile_iterator it;

	for (it = fFormFiles.begin(); it < fFormFiles.end(); it++)
	{
		if(stringsAreEqual(it->getName(), name))
			return(it->getFilename());
	}
	return(std::string(""));

}

int CgiRequest::getReqUploadFileLen(const std::string& name)
{
	formfile_iterator it;

	for (it = fFormFiles.begin(); it < fFormFiles.end(); it++)
	{
		if(stringsAreEqual(it->getName(), name))
			return(it->getDataLength());
	}
	return(0);

}


