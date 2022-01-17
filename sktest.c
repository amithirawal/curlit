#include "helpers.h"
#include "sktest.h"

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
  char *data;
  size_t len = size*nitems;
  if (len>2) {
      data  = (char *)calloc(len-1,size); 
      sprintf (data, "%s", buffer);
      data[len-2] = 0;
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
    fprintf (stderr, "not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
 
  return realsize;
}

stats get_http (char header[][MAX_STRING_SIZE], int no_of_header, int no_of_request)  {
    CURL *curl;
    int i=0, reqs=0;
    struct memory chunk;
    stats ret_val;
    chunk.response = malloc(1);
    chunk.size = 0;
    Mediator* m_n = MediatorNew(no_of_request);
    Mediator* m_c = MediatorNew(no_of_request);
    Mediator* m_s = MediatorNew(no_of_request);
    Mediator* m_t = MediatorNew(no_of_request);
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
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        for (; reqs<no_of_request; reqs++) {
            response = curl_easy_perform(curl);
            if (response != CURLE_OK) {
                fprintf (stderr, "Request failed: %s\n", curl_easy_strerror(response));
                strcpy(ret_val.ip, "0");
                ret_val.rc = -1;
                ret_val.nl = -1;
                ret_val.con = -1;
                ret_val.str = -1;
                ret_val.tot = -1;
                return ret_val;
            } else {
                curl_off_t namelookup = 0;
                curl_off_t connect = 0;
                curl_off_t start = 0;
                curl_off_t total = 0;

                if (!curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME_T, &namelookup) && (namelookup>0)) {
                    MediatorInsert(m_n,namelookup);
                }
                if (!curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME_T, &connect) && (connect>0)) {
                    MediatorInsert(m_c,connect);
                }
                if (!curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME_T, &start) && (start>0)) {
                    MediatorInsert(m_s,start);
                }
                if (!curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total) && (total>0)) {
                    MediatorInsert(m_t,total);
                }
            }
        }
        s_namelookup = MediatorMedian(m_n);
        s_connect = MediatorMedian(m_c);
        s_start = MediatorMedian(m_s);
        s_total = MediatorMedian(m_t);
        if (!curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code)) {}
        if (!curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip) && ip) {}
        strcpy(ret_val.ip, ip);
        ret_val.rc = response_code;
        ret_val.nl = s_namelookup;
        ret_val.con = s_connect;
        ret_val.str = s_start;
        ret_val.tot = s_total;
        curl_slist_free_all(list);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    free(m_n);
    free(m_c);
    free(m_s);
    free(m_t);
    return ret_val;
}

