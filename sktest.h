#define MAX_STRING_SIZE 100
typedef struct skstat {
        char ip[100];
        long rc;
        long nl;
        long con;
        long str;
        long tot;
} stats;

extern stats get_http (char header[][MAX_STRING_SIZE], int no_of_header, int no_of_request);
