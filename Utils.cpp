//
// Created by acmery on 2020/7/11.
//

#include "Utils.hpp"

std::string Utils::getCityByIp() {
    std::string buffer="";
    std::string location="";
    try
    {
        CURL *pCurl = NULL;
        CURLcode res;

        curl_global_init(CURL_GLOBAL_ALL);    // In windows, this will init the winsock stuff
        std::cout << "city requesting..." << std::endl;
        std::string url_str = "http://pv.sohu.com/cityjson?ie=utf-8";//http://ip.ws.126.net/ipquery";

        pCurl = curl_easy_init();        // get a curl handle
        if (NULL != pCurl)
        {
            curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10);
            curl_easy_setopt(pCurl, CURLOPT_URL, url_str.c_str());
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writer);
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &buffer);
            res = curl_easy_perform(pCurl);

            if (res != CURLE_OK)
            {
                printf("curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
            }

            curl_easy_cleanup(pCurl);
        }
        curl_global_cleanup();
    }
    catch (std::exception &ex)
    {
        printf("curl exception %s.\n", ex.what());
    }
    if(buffer.empty())
    {
        std::cout<< "!!! ERROR The sever response NULL" << std::endl;
    }
    else
    {
        location = parseJsonLocation(buffer);
    }

    std::cout << "get location: " << location << std::endl;

    return location;
}

std::string Utils::getOS() {
    std::fstream os_info_file;
    std::string m_release_version;
    os_info_file.open("/etc/os-release", std::ios_base::in);
    if(!os_info_file.is_open())
        printf("failed to open osinfo\n");
    std::string version;
    getline(os_info_file, version);
    m_release_version = version.substr(6, version.length()-7);
    std::cout<<m_release_version<<std::endl;
    os_info_file.close();
    return m_release_version;
}

std::string Utils::parseJsonLocation(std::string input) {
    Json::Value root;
    Json::Reader reader;

    if("" != input)
    {
        input = input.substr((int)input.find("{"));
    }

    bool parsingSuccessful = reader.parse(input, root);
    if(!parsingSuccessful)
    {
        std::cout<<"!!! Failed to parse the location data"<< std::endl;
        return "";
    }

    std::string cip =  root["cip"].asString();
    std::string cname =  root["cname"].asString();

    return cname;
}

int Utils::writer(char *data, size_t size, size_t nmemb, std::string *writerData) {
    unsigned long sizes = size * nmemb;
    if (writerData == NULL)
        return -1;

    writerData->append(data, sizes);

    return sizes;
}