//
// Created by acmery on 2020/7/11.
//

#ifndef EPOLLSRV_UTILS_HPP
#define EPOLLSRV_UTILS_HPP

#include <jsoncpp/json/json.h>
#include <curl/curl.h>
#include <fstream>
#include <iostream>


class Utils {
public:
    std::string getCityByIp();

    std::string getOS();
private:
    std::string parseJsonLocation(std::string input);
    static int writer(char *data, size_t size, size_t nmemb, std::string *writerData);
};


#endif //EPOLLSRV_UTILS_HPP
