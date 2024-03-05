#include "reader.h"
#include <fstream>

std::vector<char> Reader::ReadData(const std::wstring& filename)
{
    // バイナリファイルで読み込む
    std::ifstream ifs(filename, std::ios::binary);

    // 読み込みサイズを調べる
    ifs.seekg(0, std::ios::end);
    long long int size = ifs.tellg();
    ifs.seekg(0);

    // 読み込んだデータを出力する
    std::vector<char> data(size);
    ifs.read(data.data(), size);

    return data;
}
