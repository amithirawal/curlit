#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#define MAX_STRING_SIZE 100
#define MAX_HEADERS 15

struct memory {
    char *response;
    size_t size;
};

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
  char *data;
  size_t len = size*nitems;
  if (len>2) {
      data  = (char *)calloc(len-1,size); 
      sprintf (data, "%s", buffer);
      data[len-2] = 0;
      //printf ( "\n[%ld] [%s]", len-2, data);
      free (data);
  }
  return len;
}

static size_t response_payload_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)userp;
 
  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(!ptr) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
 
  return realsize;
}

int get_http (char header[][MAX_STRING_SIZE], int no_of_header, int no_of_request) {
    CURL *curl;
    int i=0, reqs=0;
    struct memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;
    char result[1000] = {0};

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) {
        CURLcode response;
        struct curl_slist *list = NULL;
        long response_code;
        char *ip;
        curl_off_t s_namelookup = 0;
        curl_off_t s_connect = 0;
        curl_off_t s_start = 0;
        curl_off_t s_total = 0;
                
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com/?[1-20]");
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_payload_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        for (; i<no_of_header; i++) {
            list = curl_slist_append(list, header[i]);
            //printf ( "\n[%s]", header[i]);
        }
        //printf ( "\n\n");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        for (; reqs<no_of_request; reqs++) {
            response = curl_easy_perform(curl);
            if (response != CURLE_OK) {
                fprintf (stderr, "Request failed: %s\n", curl_easy_strerror(response));
                return -1;
            } else {
                curl_off_t namelookup;
                curl_off_t connect;
                curl_off_t start;
                curl_off_t total;

                if (!curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME_T, &namelookup) && (namelookup>0)) {
                    s_namelookup += namelookup;
                    //printf("\nLookup : %lu.%06lu sec.", (unsigned long)(namelookup / 1000000), (unsigned long)(namelookup % 1000000));
                }
                if (!curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME_T, &connect) && (connect>0)) {
                    s_connect += connect;
                    //printf("\nConnect : %lu.%06lu sec.", (unsigned long)(connect / 1000000), (unsigned long)(connect % 1000000));
                }
                if (!curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME_T, &start) && (start>0)) {
                    s_start += start;
                    //printf("\nStart Transfer : %lu.%06lu sec.", (unsigned long)(start / 1000000), (unsigned long)(start % 1000000));
                }
                if (!curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total) && (total>0)) {
                    s_total += total;
                    //printf("\nTotal : %lu.%06lu sec.", (unsigned long)(total / 1000000), (unsigned long)(total % 1000000));
                }
            }
        }
        s_namelookup /= no_of_request;
        s_connect /= no_of_request;
        s_start /= no_of_request;
        s_total /= no_of_request;
        if (!curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code)) {
            printf("\nResponse Code: %ld", response_code);
        }
        if(!curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip) && ip) {
            printf("\nIP: %s", ip);
        }
        printf("\nLookup Time: %lu.%06lu sec.", (unsigned long)(s_namelookup / 1000000), (unsigned long)(s_namelookup % 1000000));
        printf("\nConnect Time: %lu.%06lu sec.", (unsigned long)(s_connect / 1000000), (unsigned long)(s_connect % 1000000));
        printf("\nStart Transfer Time: %lu.%06lu sec.", (unsigned long)(s_start / 1000000), (unsigned long)(s_start % 1000000));
        printf("\nTotal Time: %lu.%06lu sec.", (unsigned long)(s_total / 1000000), (unsigned long)(s_total % 1000000));
        sprintf ( result, "SKTEST;%s;%ld;%lu.%06lu;%lu.%06lu;%lu.%06lu;%lu.%06lu", ip, response_code,(unsigned long)(s_namelookup / 1000000), (unsigned long)(s_namelookup % 1000000),(unsigned long)(s_connect / 1000000), (unsigned long)(s_connect % 1000000),(unsigned long)(s_start / 1000000), (unsigned long)(s_start % 1000000),(unsigned long)(s_total / 1000000), (unsigned long)(s_total % 1000000));
        curl_slist_free_all(list);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    printf ("Result is  %s",result);
    return 0;
}

int main(int argc, char *argv[]) {
    char header[MAX_HEADERS][MAX_STRING_SIZE] = {0,};
    int no_of_req = 0;
    int i=0, opt;
    if( argc < 3 ) {
        printf("usage: try './SKTEST [-H \"Header-Name: Header-Value\"]' to make a get request. [%d]\n", argc);
        return 1;
    }
    while ((opt = getopt(argc, argv, "H:n:")) != -1) {
        switch(opt) {
            case 'H':
                if (optarg) {
                    strcpy(header[i++], optarg);
                    //printf("\nInput option value=(%s)", header[i-1]);
                }
                break;

            case 'n':
                no_of_req = atoi(optarg);
                //printf("\n%s, Output option value=%d", optarg, no_of_req);
                break;

            case '?':
                if (optopt == 'H') {
                    printf("\nMissing mandatory input option");
                } else if (optopt == 'n') {
                   printf("\nMissing mandatory output option");
                } else {
                   printf("\nInvalid option received");
                }
                break;
        }
    }
    get_http(header,i,no_of_req);

}

/*
SKTEST;
<IP address of HTTP server>;
<HTTP response code>;
<median of CURLINFO_NAMELOOKUP_TIME>;
<median of CURLINFO_CONNECT_TIME>;
<median of CURLINFO_STARTTRANSFER_TIME>;
<median of CURLINFO_TOTAL_TIME>
*/