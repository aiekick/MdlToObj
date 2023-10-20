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

#include <string>
#include "MdlToObj.hpp"

int main(int argc, char** argv) {
    std::string file_path_name_to_open;
    std::string file_path_name_to_save;
    if (argc > 1U) {
        file_path_name_to_open = std::string(argv[1]);
        if (argc > 2U) {
            file_path_name_to_save = std::string(argv[2]);
        }
    }

    if (!file_path_name_to_open.empty()) {
        MdlToObj m;
        if (m.openMdlFile(file_path_name_to_open)) {
            if (m.saveObjFile(file_path_name_to_save)) {
                return 0;
            }
        }
    } else {
        std::cout << "MdlToObj Syntax : MdlToObj file_to_convert [file_to_save]" << std::endl;
    }

    return 1;
}
