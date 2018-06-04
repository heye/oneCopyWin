#ifndef ONE_CONFIG_H
#define ONE_CONFIG_H

#include <string>
#include <map>

class Config {

public:
  static std::string getServerAddr();
  static void setServerAddr(std::string addr);

  static std::string getAPIKeyOne();
  static void setAPIKey(std::string);


  static std::string getValue(std::string key);
  static bool setValue(std::string key, std::string value);

private:
  Config();

  static Config& getInstance();

  std::map<std::string, std::string> keyValueMap_;

  bool hasRead_;

  bool readFromDisk();
  bool writeToDisk();

  std::wstring getConfigPath();

};

#endif // !ONE_CONFIG_H
