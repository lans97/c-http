#include <logger.h>
#include <sds.h>
#include <http.h>

int main(int argc, char* argv[]) {
    LOG_DEBUG("Starting HTTP server...");
    tutorial();
    return 0;
}
