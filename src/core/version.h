#pragma once

#include <string>
#include <cinttypes>

const std::string APP_NAME = "ProteStAr: protein structures archivizer";
const uint32_t APP_VERSION_MAJOR = 0;
const uint32_t APP_VERSION_MINOR = 7;
const uint32_t APP_VERSION_BUGFIX = 0;
const uint32_t APP_VERSION = APP_VERSION_MAJOR * 1000000 + APP_VERSION_MINOR * 1000 + APP_VERSION_BUGFIX;
const std::string APP_VERSION_STR = std::to_string(APP_VERSION_MAJOR) + "." + std::to_string(APP_VERSION_MINOR) + "." + std::to_string(APP_VERSION_BUGFIX);

const uint32_t ARCHIVE_VERSION_MAJOR = 0;
const uint32_t ARCHIVE_VERSION_MINOR = 7;
const uint32_t ARCHIVE_VERSION = ARCHIVE_VERSION_MAJOR * 100 + ARCHIVE_VERSION_MINOR;
const std::string ARCHIVE_VERSION_STR = std::to_string(ARCHIVE_VERSION_MAJOR) + "." + std::to_string(ARCHIVE_VERSION_MINOR);

const uint32_t MAX_SUPPORTED_ARCHIVE_VERSION_MAJOR = 0;
const uint32_t MAX_SUPPORTED_ARCHIVE_VERSION_MINOR = 9;
const uint32_t MAX_SUPPORTED_ARCHIVE_VERSION = MAX_SUPPORTED_ARCHIVE_VERSION_MAJOR * 100 + MAX_SUPPORTED_ARCHIVE_VERSION_MINOR;
const std::string MAX_SUPPORTED_ARCHIVE_VERSION_STR = std::to_string(MAX_SUPPORTED_ARCHIVE_VERSION_MAJOR) + "." + std::to_string(MAX_SUPPORTED_ARCHIVE_VERSION_MINOR);

const uint32_t MIN_SUPPORTED_ARCHIVE_VERSION_MAJOR = 0;
const uint32_t MIN_SUPPORTED_ARCHIVE_VERSION_MINOR = 7;
const uint32_t MIN_SUPPORTED_ARCHIVE_VERSION = MIN_SUPPORTED_ARCHIVE_VERSION_MAJOR * 100 + MIN_SUPPORTED_ARCHIVE_VERSION_MINOR;
const std::string MIN_SUPPORTED_ARCHIVE_VERSION_STR = std::to_string(MIN_SUPPORTED_ARCHIVE_VERSION_MAJOR) + "." + std::to_string(MIN_SUPPORTED_ARCHIVE_VERSION_MINOR);

std::string UNSUPPORTED_ARCHIVE_INFO(uint32_t archive_ver);

// EOF