/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "http_request.h"

/* Constants that aren't configurable in menuconfig */
// #define WEB_URL "/dev/put_data"
// #define WEB_SERVER "192.168.0.106"
#define WEB_PORT "9527"
#define WEB_URL "/put_data"
#define WEB_SERVER "47.100.122.195"
static const char *TAG = "http";

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Content-Type: application/json\r\n"
    "content-length: %d"
    "\r\n\r\n" ;
char * Header[100] = {0};
char str_asm[80];
 uint8_t http_run=0;
 void set_back(void);
void ICACHE_FLASH_ATTR  http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];
    char * str_request;
    while(1) {
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        stopLed(LED_RED);http_run=0;
        xEventGroupWaitBits(wifi_event_group, CONNECTING_BIT,
                            false, false, portMAX_DELAY);
                            http_run=1;
        set_back();
        str_request = generate_strforpoint();
        if(str_request==NULL)//数据无变化 无需上传
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }                   
        ESP_LOGI(TAG, "Connecting to AP");

        int err = getaddrinfo(WEB_SERVER, WEB_PORT , &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        char *str_request_all = (char *) malloc(strlen(REQUEST) + strlen(str_request));
        strcpy(str_request_all, REQUEST);
        sprintf(Header,REQUEST,strlen(str_request));
        strcpy(str_request_all, Header);
        strcat(str_request_all, str_request);
        if (write(s, str_request_all, strlen(str_request_all)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            cJSON_free(str_request);
            free(str_request_all);
            continue;
        }
         free(str_request_all);
        cJSON_free(str_request);
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 3;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
                // putchar(recv_buf[i]);
            }
        } while(r > 0);
        if (r>=0 ) {
            startLed(LED_RED ,1000,150);
        }
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        while(1)
        { 
             vTaskDelay(100 / portTICK_PERIOD_MS);
            if(check_update())
            {
                break;
            }
          
        }
    }
}

 
