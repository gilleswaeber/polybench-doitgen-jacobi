#define ABORT_ON_ERROR(call) \
if (int status = (call)) { \
std::cerr << "Error at " << __FILE__ << ":" << __LINE__ << ": status is " << status << std::endl; \
abort(); \
}
