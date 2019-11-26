#pragma once

#include <array>
#include <vector>
#include <set>
#include <cmath>
#include <cstddef>
#include <cassert>

// attractive force
inline double fa(const double x, const double k)
{
    assert(k > 0);
    return x*x / k;
}

// repulsive force
inline double fr(const double x, const double k)
{
    assert(x > 0);
    return k*k / x;
}

/**
 * Fruchterman, T. M., & Reingold, E. M. (1991). Graph drawing by force‐directed placement. Software: Practice and experience, 21(11), 1129-1164.
 * @param adjM Adjacency Matrix which represents triangular mesh
 * @param vertex
 * @param dist_opt Optimal distance between vertices. Denoted by k in the paper.
 * @param temp_start Upper limit of the displacement of vertex which decreases linearly for each step.
 * @param n_iter
 */
std::vector<std::array<double, 3>> layout_with_fr_3d(const std::vector<std::array<double, 3>>& vertex,
    const std::vector<std::array<std::size_t, 3>>& face, const double dist_opt,
    const double temp_start, const int n_iter)
{
    // extract edges from faces
    std::set<std::array<std::size_t, 2>> E;
    for (std::size_t fi = 0; fi < face.size(); ++fi)
    {
        E.insert({std::min(face[fi][0], face[fi][1]), std::max(face[fi][0], face[fi][1])});
        E.insert({std::min(face[fi][1], face[fi][2]), std::max(face[fi][1], face[fi][2])});
        E.insert({std::min(face[fi][2], face[fi][0]), std::max(face[fi][2], face[fi][0])});
    }
    assert(E.size() == face.size() * 3 / 2);
    assert(vertex.size() - E.size() + face.size() == 2);

    //initialization
    std::vector<std::array<double, 3>> ret{vertex};
    double temp = temp_start;

    for (int i = 0; i < n_iter; ++i)
    {
        // displacement in this step
        std::vector<std::array<double, 3>> disp(vertex.size());

        // calculate repulsive forces
        for (std::size_t vi = 0; vi < vertex.size(); ++vi)
        {
            for (std::size_t ui = vi + 1; ui < vertex.size(); ++ui)
            {
                const double dx = vertex[vi][0] - vertex[ui][0];
                const double dy = vertex[vi][1] - vertex[ui][1];
                const double dz = vertex[vi][2] - vertex[ui][2];
                const double D = std::sqrt(dx*dx + dy*dy + dz*dz);

                if (D != 0)
                {
                    disp[vi][0] += dx / D * fr(D, dist_opt);
                    disp[vi][1] += dy / D * fr(D, dist_opt);
                    disp[vi][2] += dz / D * fr(D, dist_opt);
                    disp[ui][0] -= dx / D * fr(D, dist_opt);
                    disp[ui][1] -= dy / D * fr(D, dist_opt);
                    disp[ui][2] -= dz / D * fr(D, dist_opt);
                }
            }
        }

        // calculate attractive forces
        for (auto it = E.begin(); it != E.end(); ++it)
        {
            const double vi = (*it)[0];
            const double ui = (*it)[1];
            const double dx = vertex[vi][0] - vertex[ui][0];
            const double dy = vertex[vi][1] - vertex[ui][1];
            const double dz = vertex[vi][2] - vertex[ui][2];
            const double D = std::sqrt(dx*dx + dy*dy + dz*dz);

            if (D != 0)
            {
                disp[vi][0] -= dx / D * fa(D, dist_opt);
                disp[vi][1] -= dy / D * fa(D, dist_opt);
                disp[vi][2] -= dz / D * fa(D, dist_opt);
                disp[ui][0] += dx / D * fa(D, dist_opt);
                disp[ui][1] += dy / D * fa(D, dist_opt);
                disp[ui][2] += dz / D * fa(D, dist_opt);
            }
        }

        // move
        for (std::size_t vi = 0; vi < vertex.size(); ++vi)
        {
            const double d = std::sqrt(std::pow(disp[vi][0], 2)
                    + std::pow(disp[vi][1], 2) + std::pow(disp[vi][2], 2));

            if (d != 0)
            {
                ret[vi][0] += disp[vi][0] / d * std::min(temp, std::abs(disp[vi][0]));
                ret[vi][1] += disp[vi][1] / d * std::min(temp, std::abs(disp[vi][1]));
                ret[vi][2] += disp[vi][2] / d * std::min(temp, std::abs(disp[vi][2]));
            }
        }

        temp = temp_start - i * temp_start / n_iter;
    }

    return ret;
}
