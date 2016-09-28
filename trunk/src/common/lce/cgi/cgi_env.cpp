#include "cgi_env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace lce
{
	namespace cgi
	{
		std::string CCgiEnv::m_sNull = "";

		CCgiEnv::CCgiEnv()
		{

		}

		CCgiEnv::~CCgiEnv()
		{

		}

		bool CCgiEnv::Init()
		{
			if ( LoadCgiEnv() 
				&& LoadPostData()
				&& parseCookies()
				&& parseValues() )
			{
				return true;
			}
			return false;
		}

		bool CCgiEnv::LoadPostData()
		{
			if ( strncasecmp(this->GetRequestMethod().c_str(), "post",sizeof("post")) == 0 )
			{
				m_sPostData.resize(m_dwContentLen);
				if (fread((char*)m_sPostData.data(),sizeof(char),m_dwContentLen,stdin) != m_dwContentLen) {
					return false;
				}
			}
			else if ( GetRequestMethod().empty() )
			{
				LoadDebugData();
			}
			return true;
		}

		bool CCgiEnv::LoadDebugData()
		{
			//m_sPostData.clear();
			//m_sCookie.clear();
			m_sPostData = "";
			m_sCookie="";
			

			char ch = 0;


			fprintf(stderr,"\n--- cgihtml Interactive Mode ---\n");
			fprintf(stderr,"Enter CGI input string.  Remember to encode appropriate ");
			fprintf(stderr,"characters.\nPress ENTER when done:\n\n");
			while ( ((ch = getc(stdin)) != '\n') ) {
				m_sPostData.append(1, ch);
			}

			fprintf(stderr,"Enter CGI cookie string.  Remember to encode appropriate ");
			fprintf(stderr,"characters.\nPress ENTER when done:\n\n");
			while ( ((ch = getc(stdin)) != '\n') ) {
				m_sCookie.append(1, ch);
			}

			fprintf(stderr,"\n Input string: %s\nString length: %zd\n",m_sPostData.c_str(), m_sPostData.size());
			fprintf(stderr,"\n Cookie string: %s\nString length: %zd\n",m_sCookie.c_str(), m_sCookie.size());
			fprintf(stderr,"--- end cgihtml Interactive Mode ---\n\n");
			return true;
		}



		CCgiEnv::SMIMEHeader CCgiEnv::parseMIMEHeader(const std::string& data)
		{
			SMIMEHeader stHeader;
			stHeader.sDisposition	= extractBetween(data, "Content-Disposition: ", ";");
			stHeader.sName = extractBetween(data, "name=\"", "\"");
			stHeader.sFileName = FormUrlDecode(extractBetween(data, "filename=\"", "\""));
			stHeader.sContentType = extractBetween(data, "Content-Type: ", "\r\n\r\n");
		//	printf("sDisposition=%s, sName=%s, sFileName=%s, sContentType=%s.", stHeader.sDisposition.c_str(), stHeader.sName.c_str(), stHeader.sFileName.c_str(), stHeader.sContentType.c_str());
			return stHeader;
		}

		void CCgiEnv::parseMIME(const std::string& data)
		{
			// Find the header
			std::string end = "\r\n\r\n";
			std::string::size_type headLimit = data.find(end, 0);

			// Detect error
			if(headLimit == std::string::npos)
				return;

			// Extract the value - there is still a trailing CR/LF to be subtracted off
			std::string::size_type valueStart = headLimit + end.length();
			std::string value = data.substr(valueStart, data.length() - valueStart - 2);

			// Parse the header - pass trailing CR/LF x 2 to parseHeader
			SMIMEHeader stHead = parseMIMEHeader(data.substr(0, valueStart));
			if ( stHead.sFileName.empty() ) 
			{
				m_mapValues[FormUrlDecode(stHead.sName)] = FormUrlDecode(value);
			}
			else
			{
				CCgiEnv::SFile stFile;
				stFile.sData = value;
				stFile.sName = stHead.sFileName;
				stFile.sContentType = stHead.sContentType;
				m_mapFiles[stHead.sName] = stFile;
			}
		}

		bool CCgiEnv::parseValues()
		{
			m_mapValues.clear();
			m_mapFiles.clear();
			static std::string sType 	= "multipart/form-data";
			if ( strncasecmp(this->GetContentType().c_str(), sType.c_str(), sType.size()) == 0 )
			{
				//file
				// Find out what the separator is
				std::string 		sBoundaryType 	= "boundary=";
				std::string::size_type 	pos 	= GetContentType().find(sBoundaryType);

				// generate the separators
				std::string sep = GetContentType().substr(pos + sBoundaryType.length());
				sep.append("\r\n");
				sep.insert(0, "--");

				std::string sep2 = GetContentType().substr(pos + sBoundaryType.length());
				sep2.append("--\r\n");
				sep2.insert(0, "--");

				// Find the data between the separators
				std::string::size_type start  = m_sPostData.find(sep);
				std::string::size_type sepLen = sep.length();
				std::string::size_type oldPos = start + sepLen;

				while(true) {
					pos = m_sPostData.find(sep, oldPos);

					// If sep wasn't found, the rest of the data is an item
					if(pos == std::string::npos)
						break;

					// parse the data
					parseMIME(m_sPostData.substr(oldPos, pos - oldPos));

					// update position
					oldPos = pos + sepLen;
				}

				// The data is terminated by sep2
				pos = m_sPostData.find(sep2, oldPos);
				// parse the data, if found
				if(pos != std::string::npos) {
					parseMIME(m_sPostData.substr(oldPos, pos - oldPos));
				}
			}
			else
			{
				//no file
				std::string::size_type pos1 = 0;
				std::string::size_type pos2 = 0;
				std::string::size_type equalPos = 0;
				std::string sTmp;
				std::string sValues;
				if ( strncasecmp(this->GetRequestMethod().c_str(), "get",sizeof("get")) == 0 )
				{
					sValues = m_sQueryString;
				}
				else
				{
					sValues = m_sPostData;
				}
				

				if (sValues.empty())
					return true;

				while ( pos2 != std::string::npos )
				{
					pos2 = sValues.find('&',pos1);
					sTmp = sValues.substr(pos1, pos2-pos1);
					pos1 = pos2+1;
					equalPos = sTmp.find('=');
					if (equalPos == std::string::npos)
						continue;
					m_mapValues[FormUrlDecode(sTmp.substr(0, equalPos))] = FormUrlDecode(sTmp.substr(1+equalPos));
				}

			}
			return true;
		}


		bool CCgiEnv::parseCookies()
		{
			m_mapCookies.clear();
			std::string::size_type pos1 = 0;
			std::string::size_type pos2 = 0;
			std::string::size_type equalPos = 0;
			std::string sTmp;

			if (m_sCookie.empty())
				return true;

			while ( pos2 != std::string::npos )
			{
				pos2 = m_sCookie.find(';',pos1);

				sTmp = m_sCookie.substr(pos1, pos2-pos1);
				pos1 = pos2+1;
				equalPos = sTmp.find('=');
				if (equalPos == std::string::npos)
					continue;

				// skip leading whitespace - " \f\n\r\t\v"
				std::string::size_type wscount = 0;
				std::string::const_iterator data_iter;
				for(data_iter = sTmp.begin(); data_iter != sTmp.end(); ++data_iter,++wscount)
					if(isspace(*data_iter) == 0)
						break;	

				m_mapCookies[sTmp.substr(wscount, equalPos - wscount)] = FormUrlDecode(sTmp.substr(1+equalPos));
			}

			return true;
		}


		bool CCgiEnv::LoadCgiEnv()
		{
			m_wPort      = atol(get_env("SERVER_PORT").c_str());
			m_dwContentLen   = (uint32_t)atoll(get_env("CONTENT_LENGTH").c_str());

			m_sServerSoftware       = get_env("SERVER_SOFTWARE");
			m_sServerName           = get_env("SERVER_NAME");
			m_sGatewayInterface     = get_env("GATEWAY_INTERFACE");
			m_sServerProtocol       = get_env("SERVER_PROTOCOL");
			m_sRequestMethod        = get_env("REQUEST_METHOD");
			m_sPathInfo             = get_env("PATH_INFO");
			m_sPathTranslated       = get_env("PATH_TRANSLATED");
			m_sScriptName           = get_env("SCRIPT_NAME");
			m_sQueryString          = get_env("QUERY_STRING");
			m_sRemoteHost           = get_env("REMOTE_HOST");
			m_sRemoteAddr           = get_env("REMOTE_ADDR");
			m_sAuthType             = get_env("AUTH_TYPE");
			m_sRemoteUser           = get_env("REMOTE_USER");
			m_sRemoteIdent          = get_env("REMOTE_IDENT");
			m_sContentType          = get_env("CONTENT_TYPE");
			m_sAccept               = get_env("HTTP_ACCEPT");
			m_sUserAgent            = get_env("HTTP_USER_AGENT");
			m_sRedirectRequest      = get_env("REDIRECT_REQUEST");
			m_sRedirectURL          = get_env("REDIRECT_URL");
			m_sRedirectStatus       = get_env("REDIRECT_STATUS");
			m_sReferrer             = get_env("HTTP_REFERER");
			m_sCookie               = get_env("HTTP_COOKIE");

			return true;
		}

	}
}


