/*
Copyright (c) 2015 Ryan L. Guy

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgement in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "logfile.h"


LogFile::LogFile() : _path("logs"),
                     _filename(getTime() + ".txt"),
                     _startTimeString(getTime()),
                     _separator("------------------------------------------------------------")
{
}

LogFile::LogFile(std::string filename) : _path("logs"), 
                                         _filename(filename),
                                         _startTimeString(getTime()),
                                         _separator("------------------------------------------------------------")
{
}

LogFile::LogFile(std::string filename, std::string extension) : 
                                         _path("logs"),
                                         _filename(filename + "." + extension),
                                         _startTimeString(getTime()),
                                         _separator("------------------------------------------------------------")
{
}

LogFile::~LogFile()
{
}

void LogFile::write() {
    std::ofstream out;
    out.open(_path + "/" + _filename, std::ios::out | std::ios::app);

    out << _stream.str();
    out.close();
    clear();
}

void LogFile::setPath(std::string path) {
    _path = path;
}

void LogFile::setSeparator(std::string sep) {
    _separator = sep;
}

void LogFile::enableConsole() {
    _isWritingToConsole = true;
}

void LogFile::disableConsole() {
    _isWritingToConsole = false;
}

std::string LogFile::getString() {
    return _stream.str();
}

void LogFile::clear() {
    _stream.str(std::string());
}

void LogFile::newline() {
    _stream << std::endl;
    _print("\n");
}

void LogFile::separator() {
    _stream << _separator << std::endl;
    _print(_separator + "\n");
}

void LogFile::timestamp() {
    std::string time = getTime();
    _stream << time << std::endl;
    _print(time + "\n");
}

void LogFile::log(std::string str, int indentLevel) {
    log(str, "", 0);
}

void LogFile::log(std::string str, int value, int indentLevel) {
    std::ostringstream v;
    v << value;
    log(str, v.str(), indentLevel);
}

void LogFile::log(std::string str, double value, int precision, int indentLevel) {
    double scale = (double)pow(10, precision);
    std::ostringstream v;
    v << floor(value*scale) / (scale);
    log(str, v.str(), indentLevel);
}

void LogFile::log(std::string str, std::string value, int indentLevel) {
    std::ostringstream out;
    for (int i = 0; i < indentLevel; i++) {
        out << "\t";
    }

    out << str << value << std::endl;
    _stream << out.str();
    _print(out.str());
}

std::string LogFile::getTime() {
    time_t rawtime;
    struct tm timeinfo;

    char buffer[80];
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    strftime(buffer, 80, "%d-%b-%Y %Hh%Mm%Ss\0", &timeinfo);

    return std::string(buffer);
}

void LogFile::_print(std::string str) {
    if (_isWritingToConsole) {
        std::cout << str;
    }
}