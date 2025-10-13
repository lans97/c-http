#include "http/utils.h"

bool http_version_isValid(http_version *this) {
    if (this->major == 0 && this->minor == 9) return true;
    if (this->major == 1) {
        if (this->minor == 0) return true;
        if (this->minor == 1) return true;
    }
    if (this->major == 2 && this->minor == 0) return true;
    if (this->major == 3 && this->minor == 0) return true;
    return false;
}
