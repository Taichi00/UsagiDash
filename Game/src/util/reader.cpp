#include "reader.h"
#include <fstream>

std::vector<char> Reader::ReadData(const std::wstring& filename)
{
    // �o�C�i���t�@�C���œǂݍ���
    std::ifstream ifs(filename, std::ios::binary);

    // �ǂݍ��݃T�C�Y�𒲂ׂ�
    ifs.seekg(0, std::ios::end);
    long long int size = ifs.tellg();
    ifs.seekg(0);

    // �ǂݍ��񂾃f�[�^���o�͂���
    std::vector<char> data(size);
    ifs.read(data.data(), size);

    return data;
}
