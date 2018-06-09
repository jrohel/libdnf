#include "CompressedFile.hpp"

extern "C" {
#   include <solv/solv_xfopen.h>
};

#include <sstream>

namespace libdnf {

CompressedFile::CompressedFile(const std::string & filePath)
        : File(filePath)
{}

void CompressedFile::open(const char * mode)
{
    close();
    file = solv_xfopen(filePath.c_str(), mode);
    if (!file) {
        throw OpenException(filePath);
    }
}

std::string CompressedFile::getContent()
{
    if (!file)
        throw NotOpenedException(filePath);

    constexpr size_t bufferSize = 4096;
    char buffer[bufferSize];
    std::ostringstream ss;
    size_t bytesRead;
    do {
        bytesRead = read(buffer, bufferSize);
        ss.write(buffer, bytesRead);
    } while (bytesRead == bufferSize);

    return ss.str();
}

}
