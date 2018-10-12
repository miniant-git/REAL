#include "Filesystem.h"

#include <memory>
#include <regex>

using namespace miniant::Windows::Filesystem;

const WindowsString PATH_SEPARATORS(TEXT("\\/"));

bool CanAccess(const WindowsString& path, DWORD accessRights) {
    SECURITY_INFORMATION RequestedInformation = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
    DWORD nLengthNeeded;
    ::GetFileSecurity(path.c_str(), RequestedInformation, NULL, 0, &nLengthNeeded);

    auto securityDescriptorMemory = std::make_unique<uint8_t[]>(nLengthNeeded);
    PSECURITY_DESCRIPTOR pSecurityDescriptor = securityDescriptorMemory.get();
    if (pSecurityDescriptor == nullptr)
        return false;

    if (!::GetFileSecurity(path.c_str(), RequestedInformation, pSecurityDescriptor, nLengthNeeded, &nLengthNeeded))
        return false;

    HANDLE hToken;
    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken))
        return false;

    HANDLE hImpersonationToken;
    if (!::DuplicateToken(hToken, SecurityImpersonation, &hImpersonationToken)) {
        ::CloseHandle(hToken);
        return false;
    }

    GENERIC_MAPPING GenericMapping = {};
    GenericMapping.GenericAll = FILE_ALL_ACCESS;
    GenericMapping.GenericExecute = FILE_GENERIC_EXECUTE;
    GenericMapping.GenericRead = FILE_GENERIC_READ;
    GenericMapping.GenericWrite = FILE_GENERIC_WRITE;

    ::MapGenericMask(&accessRights, &GenericMapping);

    PRIVILEGE_SET PrivilegeSet;
    DWORD PrivelegeSetLength = sizeof(PrivilegeSet);
    DWORD GrantedAccess;
    BOOL AccessStatus;
    bool result = ::AccessCheck(
        pSecurityDescriptor,
        hImpersonationToken,
        accessRights,
        &GenericMapping,
        &PrivilegeSet,
        &PrivelegeSetLength,
        &GrantedAccess,
        &AccessStatus);
    if (!result) {
        ::CloseHandle(hToken);
        ::CloseHandle(hImpersonationToken);
        return false;
    }

    ::CloseHandle(hToken);
    ::CloseHandle(hImpersonationToken);

    return AccessStatus == TRUE;
}

WindowsString MakePowerShellCommand(const WindowsString& command) {
    static const std::basic_regex<WindowsString::value_type> backslash(TEXT(R"(\\)"), std::regex_constants::optimize);
    static const std::basic_regex<WindowsString::value_type> doubleQuotes(TEXT(R"(")"), std::regex_constants::optimize);

    WindowsString escapedCommand = std::regex_replace(command, backslash, TEXT(R"(\\)"));
    escapedCommand = std::regex_replace(escapedCommand, doubleQuotes, TEXT(R"(\")"));
    return TEXT("powershell -Command \"&{ $ErrorActionPreference = 'Stop'; ") + escapedCommand + TEXT("; trap { exit 1 }}\"");
}

namespace miniant::Windows::Filesystem {

WindowsString WrapInDoubleQuotes(const WindowsString& string) {
    return TEXT("\"") + string + TEXT("\"");
}

bool ExecuteCommand(const WindowsString& command, bool asAdministrator) {
    WindowsString parameters = TEXT("/C ") + WrapInDoubleQuotes(command);

    SHELLEXECUTEINFO info = {};
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.lpVerb = asAdministrator ? TEXT("runas") : TEXT("");
    info.lpFile = TEXT("cmd");
    info.lpParameters = parameters.c_str();
    info.nShow = SW_HIDE;
    if (::ShellExecuteEx(&info) == TRUE) {
        ::WaitForSingleObject(info.hProcess, INFINITE);
        DWORD exitCode;
        bool success = ::GetExitCodeProcess(info.hProcess, &exitCode);
        ::CloseHandle(info.hProcess);
        return success && exitCode == 0;
    }

    return false;
}

WindowsString GetExecutablePath() {
    std::vector<TCHAR> path(128);
    do {
        path.resize(path.size() * 2);
        if (path.size() > DWORD(-1))
            path.resize(DWORD(-1));

        DWORD nPathLength = ::GetModuleFileName(NULL, path.data(), static_cast<DWORD>(path.size()));
        if (nPathLength < path.size())
            break;

    } while (::GetLastError() == ERROR_INSUFFICIENT_BUFFER && path.size() < DWORD(-1));

    return path.data();
}

WindowsString GetTempDirectory() {
    DWORD pathLength = ::GetTempPath(0, NULL);
    std::vector<TCHAR> path(pathLength);
    ::GetTempPath(pathLength, path.data());
    return path.data();
}

std::optional<std::vector<WindowsString>> GetPathComponents(const WindowsString& path) {
    if (path.empty())
        return {};

    std::vector<WindowsString> components;
    size_t pathLength = path.length();
    size_t lastPosition = 0;
    while (lastPosition < pathLength) {
        size_t position = path.find_first_of(PATH_SEPARATORS, lastPosition);
        if (position - lastPosition > 0)
            components.push_back(path.substr(lastPosition, position - lastPosition));

        if (position == WindowsString::npos)
            break;

        lastPosition = position + 1;
    }

    return { std::move(components) };
}

std::optional<WindowsString> GetName(const WindowsString& path) {
    size_t lastSeparator = path.find_last_of(PATH_SEPARATORS);
    if (lastSeparator == WindowsString::npos)
        return path;

    if (lastSeparator == path.length() - 1) {
        size_t secondLastSeparator = path.find_last_of(PATH_SEPARATORS, 1);
        if (secondLastSeparator == WindowsString::npos)
            return path.substr(0, lastSeparator);

        return path.substr(secondLastSeparator + 1, lastSeparator);
    }

    return path.substr(lastSeparator + 1);
}

std::optional<WindowsString> GetParentDirectory(const WindowsString& path) {
    size_t lastSeparator = path.find_last_of(PATH_SEPARATORS);
    if (lastSeparator == WindowsString::npos)
        return {};

    return { path.substr(0, lastSeparator + 1) };
}

bool IsDirectory(const WindowsString& path) {
    DWORD attributes = ::GetFileAttributes(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return false;

    return attributes & FILE_ATTRIBUTE_DIRECTORY;
}

bool IsFile(const WindowsString& path) {
    DWORD attributes = ::GetFileAttributes(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return false;

    return !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

WindowsString GetDeleteCommand(const WindowsString& filepath) {
    return TEXT("del /F /Q ") + WrapInDoubleQuotes(filepath);
}

WindowsString GetMoveCommand(const WindowsString& source, const WindowsString& destination) {
    return TEXT("move /Y ") + WrapInDoubleQuotes(source) + TEXT(" ") + WrapInDoubleQuotes(destination);
}

WindowsString GetRenameCommand(const WindowsString& source, const WindowsString& newName) {
    return TEXT("rename ") + WrapInDoubleQuotes(source) + TEXT(" ") + WrapInDoubleQuotes(newName);
}

WindowsString GetExtractZipCommand(const WindowsString& zipfile, const WindowsString& destination) {
    return MakePowerShellCommand(TEXT("Expand-Archive -Path ") + WrapInDoubleQuotes(zipfile)
        + TEXT(" -DestinationPath ") + WrapInDoubleQuotes(destination));
}

bool CanWriteTo(const WindowsString& path) {
    return CanAccess(path, GENERIC_WRITE);
}

bool DeleteFile(const WindowsString& filepath) {
    return ExecuteCommand(GetDeleteCommand(filepath), !CanWriteTo(filepath));
}

bool MoveFile(const WindowsString& source, const WindowsString& destination) {
    std::optional<WindowsString> parentDirectory = GetParentDirectory(destination);
    return ExecuteCommand(
        GetMoveCommand(source, destination),
        !CanWriteTo(source) || !parentDirectory || !CanWriteTo(parentDirectory.value()));
}

bool RenameFile(const WindowsString& source, const WindowsString& newName) {
    return ExecuteCommand(GetRenameCommand(source, newName), !CanWriteTo(source));
}

bool CreateDirectory(const WindowsString& path) {
    if (IsDirectory(path))
        return true;

    std::optional<std::vector<WindowsString>> components = GetPathComponents(path);
    if (!components)
        return false;

    WindowsString currentPath = TEXT("");
    for (const auto& component : components.value()) {
        currentPath += component + TEXT('\\');
        ::CreateDirectory(currentPath.c_str(), NULL);
    }

    return true;
}

bool ExtractZip(const WindowsString& zipfile, const WindowsString& destination) {
    return ExecuteCommand(GetExtractZipCommand(zipfile, destination), !CanWriteTo(destination));
}

}
