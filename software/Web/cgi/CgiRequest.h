/*  Class CgiRequest
 *
 *  	The base class to handle the CGI Request 
 *
 */


#ifndef CGI_REQUEST__H
#define CGI_REQUEST__H

#include <vector>
#include <sstream> 
#include <string>

#include "CgiUtils.h"
#include "FormEntry.h"
#include "FormFile.h"

  // ============================================================
  // Iterator typedefs
  // ============================================================
  
  //! A vector of FormEntry objects
  typedef std::vector<cgicc::FormEntry>::iterator 	form_iterator;
  //! A vector of \c const FormEntry objects
  typedef std::vector<cgicc::FormEntry>::const_iterator const_form_iterator;
  
  typedef std::vector<cgicc::FormFile>::iterator 	formfile_iterator;


// ============================================================
// Class MultipartHeader
// ============================================================
class MultipartHeader 
{
public:
  
  MultipartHeader(const std::string& disposition,
		  const std::string& name,
		  const std::string& filename,
		  const std::string& cType)
	:fContentDisposition(disposition),
	fName(name),
	fFilename(filename),
	fContentType(cType)
{ ; }
  
  inline
  MultipartHeader(const MultipartHeader& head)
  { operator=(head); }
  
  ~MultipartHeader()
  {;}

  MultipartHeader&
  operator= (const MultipartHeader& head)
 {
	  fContentDisposition 	= head.fContentDisposition;
	  fName 				= head.fName;
	  fFilename 			= head.fFilename;
	  fContentType 		= head.fContentType;

	  return *this;
}
 
  inline std::string 
  getContentDisposition() 				const
  { return fContentDisposition; }
  
  inline std::string
  getName() 						const
  { return fName; }

  inline std::string 
  getFilename() 					const
  { return fFilename; }

  inline std::string 
  getContentType() 					const
  { return fContentType; }

private:
  std::string fContentDisposition;
  std::string fName;
  std::string fFilename;
  std::string fContentType;
};




class CgiRequest

{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CgiRequest();

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CgiRequest()
    { ; }
	//@}
    
       // ============================================================



	
    /*! \name Server Information
     * Information on the server handling the HTTP/CGI request
     */
    //@{
    
    /*!
     * \brief Get the name and version of the HTTP server software
     *
     * For example, \c Apache/1.3.4
     * \return The name of the server software
     */
    inline std::string 
    getServerSoftware() 			const
    { return fServerSoftware; }
    
    /*!
     * \brief Get the hostname, DNS name or IP address of the HTTP server
     *
     * This is \e not a URL, for example \c www.gnu.org (no leading http://)
     * \return The name of the server
     */
    inline std::string 
    getServerName() 				const
    { return fServerName; }
    
    /*!
     * \brief Get the name and version of the gateway interface.
     *
     * This is usually \c CGI/1.1
     * \return The name and version of the gateway interface
     */
    inline std::string 
    getGatewayInterface() 			const
    { return fGatewayInterface;}
    
    /*!
     * \brief Get the name and revision of the protocol used for this request.
     *
     * This is usually \c HTTP/1.0 or \c HTTP/1.1
     * \return The protocol in use
     */
    inline std::string 
    getServerProtocol() 			const
    { return fServerProtocol; }
    
    /*!
     * \brief Get the port number on the server to which this request was sent.
     *
     * This will usually be 80.
     * \return The port number
     */
    inline unsigned long
    getServerPort() 				const
    { return fServerPort; }
    
     
    /*!
     * \brief Get the request method used for this query.
     *
     * This is usually one of \c GET or \c POST
     * \return The request method
     */
    inline std::string 
    getRequestMethod()   			const
    { return fRequestMethod; }
    

    inline std::string 
    getScriptName() 				const
    { return fScriptName; }
    
    /*!
     * \brief Get the query string for this request.
     *
     * The query string follows the <tt>?</tt> in the URI which called this
     * application. This is usually only valid for scripts called with 
     * the \c GET method. For example, in the string \c foo.cgi?cgicc=yes 
     * the query string is \c cgicc=yes.
     * @return The query string
     */
    inline std::string 
    getQueryString()  				const
    { return fQueryString; }
    
    /*!
     * \brief Get the length of the data read from standard input, in chars.
     *
     * This is usually only valid for scripts called with the POST method.
     * \return The data length
     */
    inline unsigned long
    getContentLength() 				const
    { return fContentLength; }
    
    /*!
     * \brief Get the content type of the submitted information.
     *
     * For applications called via the GET method, this information is
     * irrelevant.  For applications called with the POST method, this is
     * specifies the MIME type of the information, 
     * usually \c application/x-www-form-urlencoded or as specified by
     * getContentType().
     * \return The content type
     * \see getContentType
     */
    inline std::string 
    getContentType() 				const
    { return fContentType; }
    
    /*!
     * \brief Get the data passed to the CGI application via standard input.
     *
     * This data is of MIME type \c getContentType().
     * \return The post data.
     */
    inline std::string
    getPostData() 				const
    { return fPostData; }
    //@}
    

    inline std::string 
    getRemoteAddr() 				const
    { return fRemoteAddr; }
    
    inline std::string 
    getUserAgent() 				const
    { return fUserAgent; }
    //@}


	// add user define variables
    inline void 
    addUserVariable(const std::string& name, const std::string& value) 				
    {       
    	userVariables.push_back(cgicc::FormEntry(name, value));
     }

	// add user define --admin state  variable
    void 
    addAdminStateVariable(const std::string& name, unsigned char value);	
	
    std::string getReqUploadFilename(const std::string& name);
    int getReqUploadFileLen(const std::string& name);

	// retrieve the form data
    form_iterator 
    getElement(const std::string& name);
    
    const_form_iterator 
    getElement(const std::string& name) 		const;
    
    bool 
    getElement(const std::string& name,
	       std::vector<cgicc::FormEntry>& result) 		const;
    
    form_iterator 
    getElementByValue(const std::string& value);
    
     const_form_iterator 
    getElementByValue(const std::string& value) 	const;
    
    bool 
    getElementByValue(const std::string& value,
		      std::vector<cgicc::FormEntry>& result) 	const;

    inline const std::vector<cgicc::FormEntry>&
    getElements() 					const
    { return fFormData; }


	//directly get the HTML form field value by name. 
	//It will retreive the field and value from query string or post data.  If not found, it will return "" .
	std::string getReqValue(const std::string& field_name);

	//HTML output function, can be overide by child class
	virtual void output();

	//Other HTML output functions 
	void outputError(const std::string & outstring);
	int outputHTMLFile(const std::string & filename, bool replace = false);
	int appendHTMLFile(const std::string & filename, bool replace = false);

	void redirectHTMLFile(const std::string & filename);
	void redirectHTMLFile(const std::string & filename, const std::string & param);
	
	
  private:
    	inline std::string	getEnv(const char *varName)
	{
  		char *var = std::getenv(varName);
  		return (0 == var) ? std::string("") : var;
	}

	// get the cgi environment variables
	void acquireEnv();

 	// Convert query string into a list of FormEntries
	void parseFormInput(const std::string& data, const std::string& content_type = "application/x-www-form-urlencoded");
	MultipartHeader parseHeader(const std::string& data);
	void	parseMIME(const std::string& data);

 	// Find elements in the list of entries
	bool findEntries(const std::string& param, 
		bool byName,
		std::vector<cgicc::FormEntry>& result) 	const;

	bool fillVarInHtmlLine(std::string&data);
	bool fillLabelInHtmlLine(std::string&data);

	std::string getFieldValue(const std::string& field_name);
	
  protected:
    
    std::stringstream 	m_debug;	
     std::string		upload_path;
 
   // ============================================================
    
  private:

    std::vector<cgicc::FormEntry> 	fFormData;
    std::vector<cgicc::FormFile> 		fFormFiles;
    std::vector<cgicc::FormEntry> 	userVariables;
	
    bool			outputHTTPResponseHeaders;

    std::string		html_path;
    unsigned long 	fServerPort;
    unsigned long 	fContentLength;
    std::string 		fServerSoftware;
    std::string 		fServerName;
    std::string 		fGatewayInterface;
    std::string 		fServerProtocol;
    std::string 		fRequestMethod;
    std::string 		fScriptName;
    std::string 		fQueryString;
    std::string 		fRemoteAddr;
    std::string 		fContentType;
    std::string 		fUserAgent;
    std::string 		fPostData;

	
  };

#endif

