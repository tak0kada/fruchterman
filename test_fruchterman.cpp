#include "fruchterman.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <utility>
#include <boost/format.hpp>

std::pair<std::vector<std::array<double, 3>>, std::vector<std::array<std::size_t, 3>>>
    read_obj(const std::string& path)
{
    std::vector<std::array<double, 3>> vertex;
    std::vector<std::array<std::size_t, 3>> face;

    std::ifstream ifs{path};
    if (ifs.fail())
    {
        throw std::runtime_error("ERROR: cannot open obj file: " + path + ".");
    }

    std::string buf;
    while (ifs >> buf)
    {
        if (buf == "v")
        {
            double x, y, z;
            ifs >> x >> y >> z;
            vertex.push_back(std::array<double, 3>{x, y, z});
        }
        else if (buf == "f")
        {
            std::size_t v[3] = {};
            for (int i = 0; i < 3; ++i)
            {
                std::string s;
                ifs >> s;
                std::size_t pos{s.find_first_of("/")};
                if (pos == std::string::npos)
                {
                    pos = s.size();
                }
                std::sscanf(s.substr(0, pos).c_str(), "%zu", &v[i]);
            }
            assert (v[0] != 0 && v[1] != 0 && v[2] != 0); // each face has three vetices
            face.push_back(std::array<std::size_t, 3>{--v[0], --v[1], --v[2]});
        }
        // not implemeted
        else if (buf == "#") {}
        else if (buf == "vt") {}
        else if (buf == "vn") {}
        else if (buf == "o") {}
        else if (buf == "g") {}
        else if (buf == "s") {}
        else if (buf == "mtllib") {}
        else if (buf == "usemtl") {}
        else {}

        // discard the rest of the line
        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::size_t nV{vertex.size()};
    std::size_t nF{face.size()};
    std::size_t nE = nF * 3 / 2; // triangular mesh
    if (nV - nE + nF - 2 != 0)
    {
        std::cerr << boost::format("nV: %zu, nE: %zu, nF: %zu, g = 1 - 0.5 * (nV - nE + nF) = %d")
                         % nV % nE % nF % (1 - (nV - nE + nF) * 0.5)
                  << std::endl;
        throw std::runtime_error("ERROR: input mesh is not 2-manifold with genus 0: " + path + ".");
    }

    return {vertex, face};
}

void write_obj(const std::vector<std::array<double, 3>>& vertex,
    const std::vector<std::array<std::size_t, 3>> face, const std::string& path)
{
    std::ofstream ofs;
    try
    {
        ofs.open(path);
    }
    catch (std::ios_base::failure& e)
    {
        std::cerr << e.what() << std::endl;
    }

    for (const auto& v : vertex)
    {
        ofs << "v " << v[0]
            << " "  << v[1]
            << " "  << v[2] << "\n";
    }
    for (const auto& f : face)
    {
        ofs << "f " << f[0] + 1
            << " "  << f[1] + 1
            << " "  << f[2] + 1 << "\n";
    }
    ofs.close();
}

int main()
{
    const auto [v, f] = read_obj("bunny.obj");
    write_obj(layout_with_fr_3d(v, f, .5, .1, 1), f, "test.obj");
    return 0;
}
