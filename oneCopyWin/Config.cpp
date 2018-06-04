#include "stdafx.h"
#include "Config.h"
#include <Shlobj.h>
#include <fstream>


std::string Config::getServerAddr() {
  auto addr = getValue("server_addr_one");

  //default address
  if (addr.empty()) {
    addr = "https://azenix.io";
  }

  return addr;
}

void Config::setServerAddr(std::string addr) {
  setValue("server_addr_one", addr);
}

std::string Config::getAPIKeyOne() {
  auto key = getValue("api_key_one");

  //default key
  if (key.empty()) {
    key = "test-key";
  }

  return key;
}

void Config::setAPIKey(std::string apiKey) {
  setValue("api_key_one", apiKey);
}

std::string Config::getValue(std::string key) {
  if (!getInstance().hasRead_) {
    Config::getInstance().readFromDisk();
  }

  if (getInstance().keyValueMap_.find(key) == getInstance().keyValueMap_.end()) {
    return "";
  }
    
  return Config::getInstance().keyValueMap_.at(key);
}

bool Config::setValue(std::string key, std::string value) {
  getInstance().keyValueMap_.erase(key);
  getInstance().keyValueMap_.emplace(key, value);
  getInstance().writeToDisk();
  return true;
}

Config::Config() {
  hasRead_ = readFromDisk();
}

Config & Config::getInstance() {
  static Config configInstance;
  return configInstance;
}

bool Config::readFromDisk() {
  auto path = getConfigPath();
  std::ifstream configFile(path);

  if (!configFile.is_open()) {
    return true;
  }

  keyValueMap_.clear();

  std::string line;
  while (std::getline(configFile, line)) {
    auto split = line.find_first_of('=');

    if (split +1 >= line.length()) {
      continue;
    }
    if (split < 1) {
      continue;
    }

    auto key = line.substr(0, split);
    auto value = line.substr(split+1, line.length() - split);

    keyValueMap_.emplace(key, value);
  }
  
  configFile.close();

  return true;
}

bool Config::writeToDisk() {
  auto path = getConfigPath();
  std::ofstream configFile(path);

  //TODO: try to create the file?

  for (auto onePair : keyValueMap_) {
    configFile.write(onePair.first.c_str(), onePair.first.length());
    configFile.put('=');
    configFile.write(onePair.second.c_str(), onePair.second.length());
    configFile.put('\n');
  }

  configFile.close();
  
  return true;
}

std::wstring Config::getConfigPath() {

  std::wstring configPath;

  wchar_t *path = nullptr;
  auto result = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &path);
  if (result == S_OK) {
    configPath = path;
    CoTaskMemFree(path);
  }
  else {
    configPath = L"C:\\ProgramData";
  }
  configPath.append(L"\\oneCopyWin");

  if (!CreateDirectory(configPath.c_str(),NULL)){
    if (GetLastError() != ERROR_ALREADY_EXISTS) {
      return L"";
    }
  }

  configPath.append(L"\\config.cfg");


  return configPath;
}
