#ifndef CPNET_POCO_REQUEST
#define CPNET_POCO_REQUEST

#include <string>

class PocoRequest
{
public:
  PocoRequest(std::string serverAddr,
    bool secure = true,
    bool noverify = false);
  ~PocoRequest();



  std::string post(std::string request, std::string user = "", std::string passwd = "");
  std::string post(const char* buffer, int bufferSize, std::string user = "", std::string passwd = "");

  std::string getString(std::string key);
  std::string getBool(std::string key);


private:

  std::string jsonReply_;

  std::string serverAddr_;
  bool noverify_;
};



#endif