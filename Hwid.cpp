#include "HTTPS.h"
#include "Hwid.h"
namespace HWID {

    bool ConnectedToInternet() {
        bool Connected = false;
        try
        {
            Connected = (InternetCheckConnectionA((LPCSTR)"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0));
        }
        catch (const std::exception&)
        {
            Connected = false;
        }
        return Connected;
    }
    void AuthenticateInjection() {
        if (!InternetCheckConnectionA((LPCSTR)"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0))
        {

            exit(-1);

        }

        static bool InitializedHwid = false;
        if (!InitializedHwid)
        {
            HW_PROFILE_INFO hwProfileInfo;
            if (GetCurrentHwProfile(&hwProfileInfo)) {
                char* GUID_ = hwProfileInfo.szHwProfileGuid;
                std::string HWID;
                HWID += GUID_;
                std::string HWIDS = HTTPS::DownloadString2("https://pastebin.com/raw/4pVCNa5D");
                bool Safe = false;
                if (HWIDS.find(HWID) != std::string::npos) {
                    Safe = true;
                    HWIDS.clear();
                    //GUID_ = "";
                    HWID.clear();
                }

                if (!Safe) {
                    HWID.clear();
                    HWIDS.clear();
                    exit(-1);
                }

            }
            else
            {

                exit(-1);
            }
            InitializedHwid = true;
        }
    }
}