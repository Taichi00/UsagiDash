#include "GLTFLoader.h"
#include "SharedStruct.h"
#include "streamreader.h"
#include <experimental/filesystem>
#include <memory>

bool GLTFLoader::Load(ImportSettings settings)
{
    auto filepath = std::experimental::filesystem::path(settings.filename);
    auto& meshes = settings.meshes;
    auto inverseU = settings.inverseU;
    auto inverseV = settings.inverseV;

    if (filepath.is_relative())
    {
        // 相対パスを絶対パスに直す
        auto current = std::experimental::filesystem::current_path();
        current /= filepath;
        current.swap(filepath);
    }

    auto reader = std::make_unique<StreamReader>(filepath.parent_path());
    auto glbStream = reader->GetInputStream(filepath.filename().u8string());
    auto glbResourceReader = std::make_shared<Microsoft::glTF::GLBResourceReader>(std::move(reader), std::move(glbStream));
    auto document = Microsoft::glTF::Deserialize(glbResourceReader->GetJson());

    meshes.clear();
    meshes.resize(document.meshes.Elements().size());

    LoadMesh(document, glbResourceReader);
    LoadMaterial(document, glbResourceReader);

    return true;
}

void GLTFLoader::LoadMesh(const Microsoft::glTF::Document& doc, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader)
{
    using namespace Microsoft::glTF;

    for (const auto& mesh : doc.meshes.Elements())
    {
        for (const auto& meshPrimitive : mesh.primitives)
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            // 各アクセッサの取得
            auto& idPos = meshPrimitive.GetAttributeAccessorId(ACCESSOR_POSITION);
            auto& accPos = doc.accessors.Get(idPos);

            auto& idNrm = meshPrimitive.GetAttributeAccessorId(ACCESSOR_NORMAL);
            auto& accNrm = doc.accessors.Get(idNrm);

            auto& idUV = meshPrimitive.GetAttributeAccessorId(ACCESSOR_TEXCOORD_0);
            auto& accUV = doc.accessors.Get(idUV);

            auto& idTan = meshPrimitive.GetAttributeAccessorId(ACCESSOR_TANGENT);
            auto& accTan = doc.accessors.Get(idTan);

            auto& idColor = meshPrimitive.GetAttributeAccessorId(ACCESSOR_COLOR_0);
            auto& accColor = doc.accessors.Get(idColor);

            auto& idIndex = meshPrimitive.indicesAccessorId;
            auto& accIndex = doc.accessors.Get(idIndex);

            // アクセッサからデータ列を取得
            auto vertPos = reader->ReadBinaryData<float>(doc, accPos);
            auto vertNrm = reader->ReadBinaryData<float>(doc, accNrm);
            auto vertUV = reader->ReadBinaryData<float>(doc, accUV);
            auto vertTan = reader->ReadBinaryData<float>(doc, accTan);
            auto vertColor = reader->ReadBinaryData<float>(doc, accColor);

            auto vertexCount = accPos.count;
            for (uint32_t i = 0; i < vertexCount; ++i)
            {
                // 頂点データの構築
                int id40 = 4 * i, id41 = 4 * i + 1, id42 = 4 * i + 2, id43 = 4 * i + 3;
                int id30 = 3 * i, id31 = 3 * i + 1, id32 = 3 * i + 2;
                int id20 = 2 * i, id21 = 2 * i + 1;

                vertices.emplace_back(
                    Vertex{
                        DirectX::XMFLOAT3(vertPos[id30], vertPos[id31], vertPos[id32]),
                        DirectX::XMFLOAT3(vertNrm[id30], vertNrm[id31], vertNrm[id32]),
                        DirectX::XMFLOAT2(vertUV[id20], vertUV[id21]),
                        DirectX::XMFLOAT3(vertTan[id30], vertTan[id31], vertTan[id32]),
                        DirectX::XMFLOAT4(vertColor[id40], vertColor[id41], vertColor[id42], vertColor[id43]),
                    }
                );
            }

            // インデックスデータ
            indices = reader->ReadBinaryData<uint32_t>(doc, accIndex);


        }
    }
}
