/*
MIT License

Copyright (c) 2023 Stephane Cuillerdier (aka Aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <set>
#include <array>
#include <string>
#include <vector>
#include <cassert>
#include <cstdarg>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

#define MDL_TO_OBJ_VERSION "0.1"

// OBJ file format : https://en.wikipedia.org/wiki/Wavefront_.obj_file

typedef std::array<double, 2U> UV;
typedef std::array<double, 3U> Vertex;
typedef std::vector<int32_t> Face;
typedef std::array<double, 3U> Color;

struct Material {
    std::string name;
    double Ns = 0.0;             // specular exponent
    Color Ka = {1.0, 1.0, 1.0};  // ambiant color
    Color Ks = {1.0, 1.0, 1.0};  // specular color
    Color Kd = {0.0, 0.0, 0.0};  // diffuse color
    Color Ke = {1.0, 1.0, 1.0};  // emissive color
    double Ni = 0.0;             // refraction index
    double d = 1.0;              // opacity [0:1]
    std::string ka_texture;      // ambient texture

    // 0. Color on and Ambient off
    // 1. Color on and Ambient on
    // 2. Highlight on
    // 3. Reflection on and Ray trace on
    // 4. Transparency: Glass on, Reflection: Ray trace on
    // 5. Reflection: Fresnel on and Ray trace on
    // 6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
    // 7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
    // 8. Reflection on and Ray trace off
    // 9. Transparency: Glass on, Reflection: Ray trace off
    // 10. Casts shadows onto invisible surfaces
    double illum = 2.0;  // illumination model
};

struct Model {
    std::string name;
    Material mat;
    bool smooth_shading = false;
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<UV> uvs;
};

class MdlToObj {
private:
    std::vector<Model> m_Models;
    std::string m_SourceFilePathName;
    std::set<std::string> m_Components;

public:
    bool openMdlFile(const std::string& vFile) {
        if (m_isFileExist(vFile)) {
            m_SourceFilePathName = vFile;
            auto source = m_loadFileToString(vFile);
            if (!source.empty()) {
                size_t start_line = 0U;
                size_t end_line = source.find('\n');
                std::string component_name;
                bool mesh_found = false;
                bool data_found = false;
                bool uv_found = false;
                bool polys_found = false;
                bool texture_found = false;
                size_t num_uvs = 0U;
                size_t num_verts = 0U;
                size_t num_faces = 0U;
                Model model;
                Vertex vertex;
                UV uv;
                Face face;
                std::string smooth_shading;
                std::string surface_name;
                while (end_line != std::string::npos) {
                    auto line = source.substr(start_line, end_line - start_line);
                    m_replaceString(line, "\n", "");
                    m_replaceString(line, "\r", "");
                    m_replaceString(line, "\t", "");
                    if (mesh_found) {
                        if (m_getValueForKey(line, "NumPolys ", num_faces)) {
                            data_found = false;
                            model.faces.reserve(num_faces);  // for speed up next push_back
                            std::cout << "Faces section found" << std::endl;
                        } else if (m_getValueForKey(line, "NumVerts ", num_verts)) {
                            model.vertices.reserve(num_verts);  // for speed up next push_back
                        } else if (data_found) {
                            if (uv_found) {
                                if (m_getVertexUV(line, vertex, uv)) {
                                    model.vertices.push_back(vertex);
                                    model.uvs.push_back(uv);
                                    ++num_uvs;
                                }
                            } else {
                                if (m_getVertex(line, vertex)) {
                                    model.vertices.push_back(vertex);
                                }
                            }
                        } else if (m_isKeyExist(line, "EndPolygonMesh")) {
                            if (num_verts) {
                                std::cout << "Count Vertex : " << num_verts << std::endl;
                            }
                            if (num_uvs) {
                                std::cout << "Count Uvs : " << num_uvs << std::endl;
                            }
                            if (num_faces) {
                                std::cout << "Count Faces : " << num_faces << std::endl;
                            }
                            mesh_found = false;
                            polys_found = false;
                            m_Models.push_back(model);
                        } else if (m_isKeyExist(line, "EndTexture")) {
                            texture_found = false;
                        } else if (m_isKeyExist(line, "Texture")) {
                            texture_found = true;
                        } else if (polys_found) {
                            /*if (getFace(line, face)) {
                                model.faces.push_back(face);
                            } else */ if (m_getFaces(line, model.faces)) {
                            }
                        } else if (texture_found) {
                            if (m_getValueForKey(line, "FRGB ", model.mat.ka_texture)) {
                                std::cout << "Ka texture found : " << model.mat.ka_texture << std::endl;
                            }
                        } else if (m_getValueForKey(line, "FaceColor %", model.mat.Ka)) {
                        } else if (m_getValueForKey(line, "FaceEmissionColor %", model.mat.Ke)) {
                        } else if (m_getValueForKey(line, "SmoothShading ", smooth_shading)) {
                            model.smooth_shading = (smooth_shading != "No");
                        } else if (m_getValueForKey(line, "Shininess ", model.mat.Ns)) {
                        } else if (m_getValueForKey(line, "Translucency ", model.mat.d)) {
                        } else if (m_getValueForKey(line, "Specularity ", model.mat.Ks[0])) {
                            model.mat.Ks[1] = model.mat.Ks[0];
                            model.mat.Ks[2] = model.mat.Ks[0];
                        } else if (m_isKeyExist(line, "Polys")) {
                            polys_found = true;
                        } else if (m_isKeyExist(line, "DataTx")) {
                            data_found = true;
                            uv_found = true;
                            std::cout << "Vertices/Uvs section found" << std::endl;
                        } else if (m_isKeyExist(line, "Data")) {
                            data_found = true;
                            std::cout << "Vertices section found" << std::endl;
                        }
                    } else if (m_getValueForKey(line, "Component ", component_name)) {
                    } else if (m_getValueForKey(line, "Surface: ", surface_name)) {
                    } else if (m_isKeyExist(line, "PolygonMesh")) {
                        auto name = component_name;
                        if (!surface_name.empty() && surface_name != component_name) {
                            name += "_" + surface_name;
                        }
                        uint32_t idx = 0U;
                        while (m_Components.find(name) != m_Components.end()) {
                            name = m_toStr("%s_%u", component_name.c_str(), idx++);
                        }
                        m_Components.emplace(name);
                        mesh_found = true;
                        data_found = false;
                        uv_found = false;
                        polys_found = false;
                        model = {};
                        model.name = name;
                        model.mat.name = name;
                        std::cout << "Mesh found : " << name << std::endl;
                    }

                    start_line = end_line + 1;
                    end_line = source.find('\n', start_line);
                }

                return (!model.vertices.empty() && !model.faces.empty());
            }
        } else {
            std::cout << "Fail to open the file " << vFile << std::endl;
        }
        return false;
    }

    bool saveObjFile(const std::string& vFile) {
        bool res = false;
        std::string filePathNames[2];  // 0: obj file, 1: mtl file
        if (vFile.empty()) {
            filePathNames[0] = m_replaceFileNameExt(m_SourceFilePathName, ".obj");
            filePathNames[1] = m_replaceFileNameExt(m_SourceFilePathName, ".mtl");
        } else {
            filePathNames[0] = m_replaceFileNameExt(vFile, ".obj");
            filePathNames[1] = m_replaceFileNameExt(vFile, ".mtl");
        }

        ///////////////////////////////////
        //// write mtl file ///////////////
        ///////////////////////////////////

        std::string mtl_header = m_toStr(u8R"(# MTL File generated with MdlToObj from a STK/MDL file
# MdlToObj : https://github.com/aiekick/MdlToObj
)");
        auto mtl_file_output = mtl_header;
        for (const auto& model : m_Models) {
            mtl_file_output += m_getObjMaterialString(model.mat);
        }
        m_saveStringToFile(mtl_file_output, filePathNames[1]);

        ///////////////////////////////////
        //// write obj file ///////////////
        ///////////////////////////////////

        std::string obj_header = m_toStr(u8R"(# OBJ File generated with MdlToObj from a STK/MDL file
# MdlToObj : https://github.com/aiekick/MdlToObj
)");
        auto obj_file_output = obj_header;
        uint32_t vertices_offset = 0U;
        uint32_t uvs_offset = 0U;
        for (const auto& model : m_Models) {
            obj_file_output += m_getObjModelString(model, m_getFileName(filePathNames[1]), vertices_offset, uvs_offset);
            vertices_offset += (uint32_t)model.vertices.size();
            uvs_offset += (uint32_t)model.uvs.size();
        }
        m_saveStringToFile(obj_file_output, filePathNames[0]);

        return res;
    }

private:
    static std::string m_replaceFileNameExt(const std::string& vFilePathName, const std::string& vNewExt) {
        assert(!vFilePathName.empty());
        auto lastDot = vFilePathName.find_last_of('.');
        if (lastDot == std::string::npos) {
            return vFilePathName;
        }
        auto pathWithoutExtension = vFilePathName.substr(0, lastDot);
        return pathWithoutExtension + vNewExt;
    }
    static std::string m_getFileName(const std::string& vFilePathName) {
        assert(!vFilePathName.empty());
        auto lastSlash = vFilePathName.find_last_of("/\\");
        if (lastSlash == std::string::npos) {
            return vFilePathName;
        }
        return vFilePathName.substr(lastSlash + 1);
    }
    static bool m_isFileExist(const std::string& vFilePathName) {
        std::ifstream docFile(vFilePathName, std::ios::in);
        if (docFile.is_open()) {
            docFile.close();
            return true;
        }
        return false;
    }
    static bool m_strToIntT(const std::string& vStr, int32_t& outValue) {
        try {
            outValue = std::stoi(vStr);
            return true;
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return false;
    }
    static bool m_strToSizeT(const std::string& vStr, size_t& outValue) {
        try {
            outValue = (size_t)std::stoul(vStr);
            return true;
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return false;
    }
    static bool m_strToDouble(const std::string& vStr, double& outValue) {
        try {
            outValue = std::stod(vStr);
            return true;
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return false;
    }
    static bool m_getValueForKey(const std::string& vSrc, const std::string& vKey, std::string& vOutStrValue) {
        if (!vSrc.empty()) {
            auto key_pos = vSrc.find(vKey);
            if (key_pos != std::string::npos) {
                vOutStrValue = vSrc.substr(key_pos + vKey.size());
                auto end_line = vOutStrValue.find_first_of("\n\r");
                if (end_line != std::string::npos) {
                    vOutStrValue = vOutStrValue.substr(0U, end_line);
                }
                return true;
            }
        }
        return false;
    }
    static bool m_getValueForKey(const std::string& vSrc, const std::string& vKey, double& vOutDoubleValue) {
        std::string strValue;
        if (m_getValueForKey(vSrc, vKey, strValue)) {
            return m_strToDouble(strValue, vOutDoubleValue);
        }
        return false;
    }
    static bool m_getValueForKey(const std::string& vSrc, const std::string& vKey, size_t& vOutSizeTValue) {
        std::string strValue;
        if (m_getValueForKey(vSrc, vKey, strValue)) {
            return m_strToSizeT(strValue, vOutSizeTValue);
        }
        return false;
    }
    static bool m_getValueForKey(const std::string& vSrc, const std::string& vKey, Color& vOutColorValue) {
        std::string strValue;
        if (m_getValueForKey(vSrc, vKey, strValue)) {
            double v = 0.0;
            if (m_strToDouble(strValue.substr(0, 3), v)) {
                auto r = v / 255.0;
                if (m_strToDouble(strValue.substr(3, 3), v)) {
                    auto g = v / 255.0;
                    if (m_strToDouble(strValue.substr(6), v)) {
                        auto b = v / 255.0;
                        vOutColorValue[0] = r;
                        vOutColorValue[1] = g;
                        vOutColorValue[2] = b;
                        return true;
                    }
                }
            }
        }
        return false;
    }
    static bool m_isKeyExist(const std::string& vSrc, const std::string& vKey) {
        if (!vSrc.empty()) {
            return (vSrc.find(vKey) != std::string::npos);
        }
        return false;
    }
    static bool m_replaceString(::std::string& str, const ::std::string& oldStr, const ::std::string& newStr) {
        bool found = false;
        size_t pos = 0;
        while ((pos = str.find(oldStr, pos)) != ::std::string::npos) {
            found = true;
            str.replace(pos, oldStr.length(), newStr);
            pos += newStr.length();
        }
        return found;
    }
    static std::vector<std::string> m_splitStringToVector(const std::string& vText, const std::string& vDelimiters, const bool& vPushEmpty) {
        std::vector<std::string> arr;
        if (!vText.empty()) {
            size_t start = 0;
            size_t end = vText.find_first_of(vDelimiters, start);
            while (end != std::string::npos) {
                std::string token = vText.substr(start, end - start);
                if (!token.empty() || (token.empty() && vPushEmpty)) {
                    arr.emplace_back(token);
                }
                start = end + 1;
                end = vText.find_first_of(vDelimiters, start);
            }
            std::string token = vText.substr(start);
            if (!token.empty() || (token.empty() && vPushEmpty)) {
                arr.emplace_back(token);
            }
        }
        return arr;
    }
    static std::vector<int32_t> m_splitStringToIntVector(const std::string& vText, const std::string& vDelimiters, const bool& vPushEmpty) {
        std::vector<int32_t> arr;
        int32_t tmp;
        if (!vText.empty()) {
            size_t start = 0;
            size_t end = vText.find_first_of(vDelimiters, start);
            while (end != std::string::npos) {
                std::string token = vText.substr(start, end - start);
                if (!token.empty() || (token.empty() && vPushEmpty)) {
                    if (m_strToIntT(token, tmp)) {
                        arr.emplace_back(tmp);
                    }
                }
                start = end + 1;
                end = vText.find_first_of(vDelimiters, start);
            }
            std::string token = vText.substr(start);
            if (!token.empty() || (token.empty() && vPushEmpty)) {
                if (m_strToIntT(token, tmp)) {
                    arr.emplace_back(tmp);
                }
            }
        }
        return arr;
    }
    static bool m_getVertex(const std::string& vSrc, Vertex& vOutVertexValue) {
        if (!vSrc.empty()) {
            auto tokens = m_splitStringToVector(vSrc, " ", false);
            if (tokens.size() == 3U) {
                if (m_strToDouble(tokens[0], vOutVertexValue[0])) {
                    if (m_strToDouble(tokens[1], vOutVertexValue[1])) {
                        if (m_strToDouble(tokens[2], vOutVertexValue[2])) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    static bool m_getVertexUV(const std::string& vSrc, Vertex& vOutVertexValue, UV& vOutUVValue) {
        if (!vSrc.empty()) {
            auto tokens = m_splitStringToVector(vSrc, " ", false);
            if (tokens.size() == 5U) {
                if (m_strToDouble(tokens[0], vOutVertexValue[0])) {
                    if (m_strToDouble(tokens[1], vOutVertexValue[1])) {
                        if (m_strToDouble(tokens[2], vOutVertexValue[2])) {
                            if (m_strToDouble(tokens[3], vOutUVValue[0])) {
                                if (m_strToDouble(tokens[4], vOutUVValue[1])) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }
    static bool m_getFaces(const std::string& vSrc, std::vector<Face>& vOutFacesValue) {
        if (!vSrc.empty()) {
            // [-1] N i0 i1 i2 i3 i4 i5 i6 ...
			// -1 => triangle strip / nothing => triangle fan
            // N => count faces
            auto tokens = m_splitStringToIntVector(vSrc, " ", false);
            if (!tokens.empty()) {
                int32_t offset = 1;
                int32_t mode = 1; // triangle fan
                int32_t count_faces;
                if (tokens[0] < 0) {
                    mode = -1;  // triangle strip
                    count_faces = tokens[1];
                    offset = 2;
                } else {
                    count_faces = tokens[0];
                }
                if (count_faces == (tokens.size() - offset)) {  // check than face is ok
                    Face face;
                    size_t count = tokens.size() - 1U;
                    size_t face_id = 0U;
                    for (size_t idx = offset + 1U; idx < count; ++idx) {
                        if (mode == 1) {  // triangle fan
                            /*
                            * [0 1 2 3] :
                            * 3 -- 2
                            * |  / |
                            * | /  |
                            * 0 -- 1
                            * f0 [0 1 2]
                            * f1 [0 2 3]
                            */
                            face.push_back(tokens[offset]);
                            face.push_back(tokens[idx]);
                            face.push_back(tokens[idx + 1]);
                            vOutFacesValue.push_back(face);
                            face.clear();
                        } else /*if (mode == -1)*/ {  // triangle strip
                            /*
                            * [0 1 3 2 5 4 7] :
                            * 0 -- 3 -- 5 -- 7
                            * |  / |  / |  /
                            * | /  | /  | /
                            * 1 -- 2 -- 4
                            * f0 [0 1 3]
                            * f1 [1 2 3]
                            * f2 [3 2 5]
                            * f3 [2 4 5]
                            * f4 [5 4 7]
                            */
                            if (face_id && (face_id + 1U) % 2U == 0) {
                                face.push_back(tokens[idx - 1]);
                                face.push_back(tokens[idx + 1]);
                                face.push_back(tokens[idx]);
                                vOutFacesValue.push_back(face);
                                face.clear();
                                ++face_id;
                            } else {
                                face.push_back(tokens[idx - 1]);
                                face.push_back(tokens[idx]);
                                face.push_back(tokens[idx + 1]);
                                vOutFacesValue.push_back(face);
                                face.clear();
                                ++face_id;
                            }
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }
    static std::string m_loadFileToString(const std::string& vFile) {
        std::string res;
        std::ifstream docFile(vFile, std::ios::in);
        if (docFile.is_open()) {
            std::stringstream strStream;
            strStream << docFile.rdbuf();  // read the file
            res = strStream.str();
            docFile.close();
        }
        return res;
    }
    static void m_saveStringToFile(const std::string& vString, const std::string& vFilePathName) {
        std::ofstream fileWriter(vFilePathName, std::ios::out);
        if (!fileWriter.bad()) {
            fileWriter << vString;
            fileWriter.close();
        }
    }
    static std::string m_toStr(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char TempBuffer[3072 + 1];  // 3072 = 1024 * 3
        const int w = vsnprintf(TempBuffer, 3072, fmt, args);
        va_end(args);
        if (w) {
            return std::string{TempBuffer, (size_t)w};
        }
        return std::string{};
    }
    static std::string m_getObjMaterialString(const Material& vMaterial) {
        auto res = m_toStr(u8R"(
newmtl %s
Ns %.6f
Ka %.6f %.6f %.6f
Kd %.6f %.6f %.6f
Ks %.6f %.6f %.6f
Ke %.6f %.6f %.6f
Ni %.6f
d %.6f
illum %u
)",
                         vMaterial.name.c_str(),                             //
                         vMaterial.Ns,                                       //
                         vMaterial.Ka[0], vMaterial.Ka[1], vMaterial.Ka[2],  //
                         vMaterial.Kd[0], vMaterial.Kd[1], vMaterial.Kd[2],  //
                         vMaterial.Ks[0], vMaterial.Ks[1], vMaterial.Ks[2],  //
                         vMaterial.Ke[0], vMaterial.Ke[1], vMaterial.Ke[2],  //
                         vMaterial.Ni,                                       //
                         vMaterial.d,                                        //
                         (uint32_t)vMaterial.illum);                         //
        if (!vMaterial.ka_texture.empty()) {
            res += m_toStr(u8R"(map_Ka %s\n
)",
                         vMaterial.ka_texture.c_str());
        }
        return res;
    }
    static uint32_t m_getFaceItemId(const int32_t& vIdx, const uint32_t& vOffset, const uint32_t& vItemCount) {
        if (vIdx < 0) {
            return (uint32_t)(vIdx + 1U + vOffset + vItemCount);
        }
        return (uint32_t)(vIdx + 1U + vOffset);
    }
    static std::string m_getObjModelString(const Model& vModel, const std::string& vMTLFile, const uint32_t& vVerticeOffset,
                                         const uint32_t& vUvsOffset) {
        bool have_uvs = !vModel.uvs.empty();
        std::string res;
        res += m_toStr("mtllib %s\no %s\n", vMTLFile.c_str(), vModel.name.c_str());
        for (const auto& vertex : vModel.vertices) {
            res += m_toStr("v %.6f %.6f %.6f\n", vertex[0], vertex[1], vertex[2]);
        }
        for (const auto& uv : vModel.uvs) {
            res += m_toStr("vt %.6f %.6f\n", uv[0], uv[1]);
        }
        res += m_toStr("s %i\n", vModel.smooth_shading ? 1 : 0);
        res += m_toStr("usemtl %s\n", vModel.mat.name.c_str());
        auto vertices_count = (uint32_t)vModel.vertices.size();
        auto uvs_count = (uint32_t)vModel.uvs.size();
        for (const auto& face : vModel.faces) {
            if (face.size() == 3U) {
                const auto& iv0 = m_getFaceItemId(face[0], vVerticeOffset, vertices_count);
                const auto& iv1 = m_getFaceItemId(face[1], vVerticeOffset, vertices_count);
                const auto& iv2 = m_getFaceItemId(face[2], vVerticeOffset, vertices_count);
                const auto& iuv0 = m_getFaceItemId(face[0], vUvsOffset, uvs_count);
                const auto& iuv1 = m_getFaceItemId(face[1], vUvsOffset, uvs_count);
                const auto& iuv2 = m_getFaceItemId(face[2], vUvsOffset, uvs_count);
                if (have_uvs) {
                    res += m_toStr("f %u/%u %u/%u %u/%u\n", iv0, iuv0, iv1, iuv1, iv2, iuv2);
                } else {
                    res += m_toStr("f %u %u %u\n", iv0, iv1, iv2);
                }
            } else if (face.size() == 4U) {
                const auto& iv0 = m_getFaceItemId(face[0], vVerticeOffset, vertices_count);
                const auto& iv1 = m_getFaceItemId(face[1], vVerticeOffset, vertices_count);
                const auto& iv2 = m_getFaceItemId(face[2], vVerticeOffset, vertices_count);
                const auto& iv3 = m_getFaceItemId(face[3], vVerticeOffset, vertices_count);
                const auto& iuv0 = m_getFaceItemId(face[0], vUvsOffset, uvs_count);
                const auto& iuv1 = m_getFaceItemId(face[1], vUvsOffset, uvs_count);
                const auto& iuv2 = m_getFaceItemId(face[2], vUvsOffset, uvs_count);
                const auto& iuv3 = m_getFaceItemId(face[3], vUvsOffset, uvs_count);
                if (have_uvs) {
                    res += m_toStr("f %u/%u %u/%u %u/%u %u/%u\n", iv0, iuv0, iv1, iuv1, iv2, iuv2, iv3, iuv3);
                } else {
                    res += m_toStr("f %u %u %u %u\n", iv0, iv1, iv2, iv3);
                }
            } else {
                res += "f ";
                for (const auto& i : face) {
                    const auto& iv = m_getFaceItemId(i, vVerticeOffset, vertices_count);
                    const auto& iuv = m_getFaceItemId(i, vUvsOffset, uvs_count);
                    if (have_uvs) {
                        res += m_toStr("%u/%u ", iv, iuv);
                    } else {
                        res += m_toStr("%u ", iv);
                    }
                }
                // possible because the alst char is always a space
                // so we replace it by a \n
                res.back() = '\n';
            }
        }
        return res;
    }
};
