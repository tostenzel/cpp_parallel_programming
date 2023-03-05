#ifndef PP_IO_H
#define PP_IO_H

#include <string>
#include <fstream>

template<class T>
void dump_binary(const T* data, const size_t length, std::string filename) {
    std::ofstream ofile(filename.c_str(), std::ios::binary);
    ofile.write((char*) data, sizeof(T)*length);
    ofile.close();
}

template<class T>
void load_binary(const T* data, const size_t length, std::string filename) {
    std::ifstream ifile(filename.c_str(), std::ios::binary);
    ifile.read((char*) data, sizeof(T)*length);
    ifile.close();
}

#endif
