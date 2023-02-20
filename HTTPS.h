#pragma once
#include <windows.h>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include <vector>
#include <WinInet.h>
#pragma comment(lib, "wininet")

namespace HTTPS {
    std::string DownloadString2(std::string URL) {
        HINTERNET interwebs = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
        HINTERNET urlFile;
        std::string rtn;
        if (interwebs) {
            urlFile = InternetOpenUrlA(interwebs, URL.c_str(), NULL, NULL, NULL, NULL);
            if (urlFile) {
                char buffer[2000];
                DWORD bytesRead;
                do {
                    InternetReadFile(urlFile, buffer, 2000, &bytesRead);
                    rtn.append(buffer, bytesRead);
                    memset(buffer, 0, 2000);
                } while (bytesRead);
                InternetCloseHandle(interwebs);
                InternetCloseHandle(urlFile);
                //std::string p = replaceAll(rtn, "|n", "\n");
                //return p;
            }
            else
            {
     
    
                exit(-1);
            }
        }
        else
        {
            interwebs = InternetOpenA("Mozilla/4.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
            if (interwebs) {
                urlFile = InternetOpenUrlA(interwebs, URL.c_str(), NULL, NULL, NULL, NULL);
                if (urlFile) {
                    char buffer[2000];
                    DWORD bytesRead;
                    do {
                        InternetReadFile(urlFile, buffer, 2000, &bytesRead);
                        rtn.append(buffer, bytesRead);
                        memset(buffer, 0, 2000);
                    } while (bytesRead);
                    InternetCloseHandle(interwebs);
                    InternetCloseHandle(urlFile);
                    //std::string p = replaceAll(rtn, "|n", "\n");
                    //return p;
                }
                else
                {
               
                    exit(-1);
                }
            }
            else
            {
        
                exit(-1);
            }
        }
        InternetCloseHandle(interwebs);
        //std::string p = replaceAll(rtn, "|n", "\n");
        return rtn;
    }





    
}
