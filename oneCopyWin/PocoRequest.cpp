

#include "stdafx.h"

#include "PocoRequest.h"

//#include "Poco/StreamCopier.h"

//#include "Poco/URIStreamOpener.h"
//#include "Poco/Net/HTTPStreamFactory.h"

#include "Poco/Net/Context.h"
//#include "Poco/Net/SSLManager.h"
//#include "Poco/Net/SSLException.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"

#include "Poco/JSON/Parser.h"


#include <iostream>

CPNet::PocoRequest::PocoRequest(
  std::string serverAddr,
  bool secure,
  bool noverify) :

  serverAddr_(serverAddr),
  noverify_(noverify)
{
}

CPNet::PocoRequest::~PocoRequest()
{
}

std::string CPNet::PocoRequest::post(std::string payload, std::string user, std::string passwd) {
  jsonReply_ = "";

  try {

    if (payload.empty()) {
      return "";
    }


    auto meth = Poco::Net::Context::VERIFY_RELAXED;
    if (noverify_) {
      meth = Poco::Net::Context::VERIFY_NONE;
    }

    const std::string winCertStore = "";
    const Poco::URI uri(serverAddr_);
    Poco::Net::Context::Ptr context(
      new Poco::Net::Context(
        Poco::Net::Context::TLSV1_2_CLIENT_USE,
        "",
        meth
#ifdef _WIN32
        //,Poco::Net::Context::Options::OPT_PERFORM_REVOCATION_CHECK | Poco::Net::Context::Options::OPT_USE_STRONG_CRYPTO
        //,""
#else
        , 9
        , true
#endif
      ));

    //std::istringstream certPem("");
    //auto cert = Poco::Net::X509Certificate(certPem);
    //context->addTrustedCert(cert);

#ifdef CP_SERVER
    CPlanServerLog() << "host: " << uri.getHost() << ":" << uri.getPort();
#endif

    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, "/");

    req.setContentType("application/json");
    //req.setKeepAlive(true);
    req.setContentLength(payload.length());
    session.sendRequest(req) << payload;

    if (!user.empty() && !passwd.empty()) {
      Poco::Net::HTTPBasicCredentials cred(user, passwd);
      cred.authenticate(req);
    }

    //execute request
    Poco::Net::HTTPResponse res;
    std::istream& rs = session.receiveResponse(res);
    std::string resp;

    //format response
    auto readSize = res.getContentLength();
    if (readSize > 0) {
      auto readBuff = std::auto_ptr<char>(new char[readSize]);
      rs.read(readBuff.get(), readSize);
      //readBuff.get()[readSize] = '\0'; //make it a c string
      jsonReply_ = std::string(readBuff.get(), readSize);
    }
  }
  catch (const std::exception& e) {
  }

  return jsonReply_;
}

std::string CPNet::PocoRequest::getString(std::string key) {

  try {
    Poco::JSON::Parser parser;
    auto parsed = parser.parse(jsonReply_);

    auto object = parsed.extract<Poco::JSON::Object::Ptr>();

    auto value = object->getValue<std::string>(key);

    return value;
  }
  catch (std::exception &e) {

  }
  return "";
}
