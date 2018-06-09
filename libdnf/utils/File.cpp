#include "File.hpp"
#include "CompressedFile.hpp"

extern "C" {
#   include <solv/solv_xfopen.h>
};

#include <stdlib.h>

namespace libdnf {

std::shared_ptr<File> File::newFile(const std::string & filePath)
{
    if (solv_xfopen_iscompressed(filePath.c_str()) == 1) {
        return std::make_shared<libdnf::CompressedFile>(filePath);
    } else {
        return std::make_shared<libdnf::File>(filePath);
    }
}

File::File(const std::string & filePath)
        : filePath(filePath)
        , file(nullptr)
{}

File::~File()
{
    close();
}

void File::open(const char * mode)
{
    close();
    file = fopen(filePath.c_str(), mode);
    if (!file) {
        throw OpenException(filePath);
    }
}

void File::close()
{
    if (!file)
        return;
    if (fclose(file) != 0) {
        file = nullptr;
        throw CloseException(filePath);
    }
    file = nullptr;
}

size_t File::read(char *buffer, size_t count)
{
    return fread(buffer, sizeof(char), count, file);
}

bool File::readLine(std::string & line)
{
    char *buffer = nullptr;
    size_t size = 0;
    if (getline(&buffer, &size, file) == -1) {
        free(buffer);
        return false;
    }

    line = buffer;
    free(buffer);

    return true;
}

std::string File::getContent()
{
    if (!file)
        throw NotOpenedException(filePath);

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    std::string content(fileSize, '\0');
    auto bytesRead = read(&content.front(), fileSize);
    if (bytesRead != fileSize) {
        throw ShortReadException(filePath);
    }

    return content;
}

}
