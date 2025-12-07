#include "Header.h"

void PostScoreToServer(const char* id, int score) {
    HINTERNET hInternet = InternetOpenA("WinInet Example", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hInternet) {
        HINTERNET hConnect = InternetConnectA(hInternet, "ninerang.cafe24.com", INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (hConnect) {
            const char* headers = "Content-Type: application/x-www-form-urlencoded";
            
            // POST 데이터 생성 (C++ string 사용)
            std::ostringstream postData;
            postData << "id=" << id << "&point=" << score;
            std::string dataStr = postData.str();
            
            HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/CookieRun/pointend.php", NULL, NULL, NULL, 0, 0);
            if (hRequest) {
                BOOL bRequestSent = HttpSendRequestA(hRequest, headers, (DWORD)strlen(headers), 
                                                   (LPVOID)dataStr.c_str(), (DWORD)dataStr.length());
                if (bRequestSent) {
                    // 응답 읽기 (선택적)
                    DWORD dwBytesRead = 0;
                    char buffer[1024];
                    BOOL bReadSuccess = InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &dwBytesRead);
                    if (bReadSuccess && dwBytesRead > 0) {
                        buffer[dwBytesRead] = '\0';
                        std::cout << "서버 응답: " << buffer << std::endl;
                    }
                    std::cout << "점수 전송 성공! ID: " << id << ", 점수: " << score << std::endl;
                }
                else {
                    DWORD error = GetLastError();
                    std::cerr << "점수 전송 실패! 오류 코드: " << error << std::endl;
                }
                InternetCloseHandle(hRequest);
            }
            else {
                std::cerr << "HTTP 요청 생성 실패!" << std::endl;
            }
            InternetCloseHandle(hConnect);
        }
        else {
            std::cerr << "서버 연결 실패!" << std::endl;
        }
        InternetCloseHandle(hInternet);
    }
    else {
        std::cerr << "인터넷 초기화 실패!" << std::endl;
    }
}