//
// Created by 55241 on 2023/4/4.
//

#ifndef HW3_VOXELGRID_H
#define HW3_VOXELGRID_H
#include "definitions.h"

struct VoxelGrid {

    std::vector<int> voxels;
    std::vector<unsigned int> ex_voxels;
    std::vector<unsigned int> in_voxels;
    std::vector<unsigned int> buildings;
    unsigned int max_x, max_y, max_z;

    float resolution;


    VoxelGrid(unsigned int x, unsigned int y, unsigned int z, const float &reso) {
        max_x = x;
        max_y = y;
        max_z = z;
        resolution = reso;
        unsigned int total_voxels = x*y*z;
        voxels.reserve(total_voxels);

        for (unsigned int i = 0; i < total_voxels; ++i) voxels.push_back(0);
    }

    // overloaded function, enabling different types input arguments to call one function of the same name
    int &operator()(const unsigned int &x, const unsigned int &y, const unsigned int &z) {
        assert(x >= 0 && x < max_x);
        assert(y >= 0 && y < max_y);
        assert(z >= 0 && z < max_z);
        return voxels[x + y*max_x + z*max_x*max_y];
    }

    int operator()(const unsigned int &x, const unsigned int &y, const unsigned int &z) const {
        assert(x >= 0 && x < max_x);
        assert(y >= 0 && y < max_y);
        assert(z >= 0 && z < max_z);
        return voxels[x + y*max_x + z*max_x*max_y];
    }

    Point3 center(const unsigned int &x, const unsigned int &y, const unsigned int &z, const Point3& origin) const {
        double xc = (x+0.5) * resolution + origin.x();
        double yc = (y+0.5) * resolution + origin.y();
        double zc = (z+0.5) * resolution + origin.z();
        Point3 cp(xc,yc,zc);
        return cp;
    }


    unsigned int voxel_index(const unsigned int &x, const unsigned int &y, const unsigned int &z) const {
        assert(x >= 0 && x < max_x);
        assert(y >= 0 && y < max_y);
        assert(z >= 0 && z < max_z);
        unsigned int idx = x + y*max_x + z*max_x*max_y;
        return idx;
    }

    std::vector<unsigned int> voxel_coordinates(const unsigned int &idx) const {
        assert(idx >= 0 && idx < max_x*max_y*max_z);
        unsigned int z = idx / (max_x*max_y);
        unsigned int y = (idx - z*max_x*max_y) / max_x;
        unsigned int x = idx - z*max_x*max_y - y*max_x;
        assert(x >= 0 && x < max_x);
        assert(y >= 0 && y < max_y);
        assert(z >= 0 && z < max_z);
        return {x, y, z};
    }


    std::vector<unsigned int> get_neighbour(const unsigned int &x, const unsigned int &y, const unsigned int &z
    , const int& v) const{
        std::vector<unsigned int> neighbours;
        assert(x >= 0 && x < max_x);
        assert(y >= 0 && y < max_y);
        assert(z >= 0 && z < max_z);
        if (x-1 >= 0) {
            unsigned int i1 = voxel_index(x-1, y, z);
            if (voxels[i1]==v) neighbours.push_back(i1);
        }
        if (x+1 < max_x) {
            unsigned int i2 = voxel_index(x+1, y, z);
            if (voxels[i2]==v) neighbours.push_back(i2);
        }
        if (y-1 >= 0) {
            unsigned int i3 = voxel_index(x, y-1, z);
            if (voxels[i3]==v) neighbours.push_back(i3);
        }
        if (y+1 < max_y) {
            unsigned int i4 = voxel_index(x, y+1, z);
            if (voxels[i4]==v) neighbours.push_back(i4);
        }
        if (z-1 >= 0) {
            unsigned int i5 = voxel_index(x, y, z-1);
            if (voxels[i5]==v) neighbours.push_back(i5);
        }
        if (z+1 < max_z) {
            unsigned int i6 = voxel_index(x, y, z+1);
            if (voxels[i6]==v) neighbours.push_back(i6);
        }
        return neighbours;
    }


    bool get_exterior_neighbour(unsigned int x,unsigned int y,unsigned int z,const VoxelGrid &voxel_grid)const{

        unsigned int i = x + 1;
        unsigned int j = y + 1;
        unsigned int k = z + 1;
        //check its former three connected neighbour,if one of them is exterior,return true
        if(i<=max_x-1){
            if(voxel_grid(i,y,z) == -1) { return true;}
        }
        if(j<=max_y-1){
            if(voxel_grid(x,j,z) == -1) {return true;}
        }
        if(k<=max_z-1){
            if(voxel_grid(x,y,k) == -1) { return true; }
        }
        return false;
    }


    void mark_exterior(VoxelGrid &voxel_grid) {
        for(int i = max_x-1; i>=0; i--){
            for(int j = max_y-1; j>=0; j--){
                for(int k = max_z-1; k>=0; k--){
                    if(i==max_x-1 && j==max_y-1 && k==max_z-1){ // mark the extra starting origin as exterior
                        voxel_grid(max_x-1,max_y-1,max_z-1) = -1;
                        ex_voxels.emplace_back(voxel_index(i, j, k));
                    }
                    else{
                        if(voxel_grid(i,j,k)==0) {//make sure this is not building voxel
                            if (get_exterior_neighbour(i, j, k, voxel_grid)) {
                                voxel_grid(i, j, k) = -1;//if one of the neighbour is exterior,it's connected and is exterior too
                                ex_voxels.emplace_back(voxel_index(i, j, k));
                            }
                            else{
                                in_voxels.emplace_back(voxel_index(i, j, k));// this is interior, the value will be changed in mark_room
                            }
                        }
                        else{
                            buildings.emplace_back(voxel_index(i, j, k)); // face_id
                        }
                    }
                }
            }
        }
    }

    void mark_room() {
        std::list<unsigned int> marking;
        marking.push_back(max_x*max_y*max_z-1);
        while (!marking.empty()) {
            unsigned int idx = marking.front();
            if (voxels[idx] != -1){
                voxels[idx] = -1;
            }
            std::vector<unsigned int> coordinate = voxel_coordinates(idx);
            std::vector<unsigned int> neighbours = get_neighbour(coordinate[0], coordinate[1], coordinate[2], 0);
            for (auto const &neighbour: neighbours) {
                if (neighbour < voxels.size()) { // check if neighbour is within bounds
                    voxels[neighbour] = -1;
//                    std::cout << neighbour << std::endl;
                    marking.push_back(neighbour);
                } else {
                    std::cout << "Neighbour index " << neighbour << " is out of bounds!" << std::endl;
//                    continue;
                }
            }
            marking.pop_front();
        }
    }


    void voxel_to_obj(std::vector<unsigned int>& voxel_coords, const Point3& origin,
                      const std::string& filename) {

        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cout << "Error opening file " << "'" << filename << "'.";
        }
        else {
            out << "# File generated by voxel_to_obj." << std::endl;
            for (int i = 0; i < voxel_coords.size(); i++) {
                Point3 vc = center(voxel_coordinates(voxel_coords[i])[0],
                                   voxel_coordinates(voxel_coords[i])[1],
                                   voxel_coordinates(voxel_coords[i])[2], origin);
                auto offet = resolution / 2;
                double vc_x = vc[0];
                double vc_y = vc[1];
                double vc_z = vc[2];
                Point3 v1 = Point3(vc_x - offet, vc_y + offet, vc_z - offet);
                Point3 v2 = Point3(vc_x - offet, vc_y - offet, vc_z - offet);
                Point3 v3 = Point3(vc_x + offet, vc_y - offet, vc_z - offet);
                Point3 v4 = Point3(vc_x + offet, vc_y + offet, vc_z - offet);
                Point3 v5 = Point3(vc_x + offet, vc_y + offet, vc_z + offet);
                Point3 v6 = Point3(vc_x - offet, vc_y + offet, vc_z + offet);
                Point3 v7 = Point3(vc_x - offet, vc_y - offet, vc_z + offet);
                Point3 v8 = Point3(vc_x + offet, vc_y - offet, vc_z + offet);
                out << "v " << v1.x() << " " << v1.y() << " " << v1.z() << std::endl;
                out << "v " << v2.x() << " " << v2.y() << " " << v2.z() << std::endl;
                out << "v " << v3.x() << " " << v3.y() << " " << v3.z() << std::endl;
                out << "v " << v4.x() << " " << v4.y() << " " << v4.z() << std::endl;
                out << "v " << v5.x() << " " << v5.y() << " " << v5.z() << std::endl;
                out << "v " << v6.x() << " " << v6.y() << " " << v6.z() << std::endl;
                out << "v " << v7.x() << " " << v7.y() << " " << v7.z() << std::endl;
                out << "v " << v8.x() << " " << v8.y() << " " << v8.z() << std::endl;
                out << "f " << 8*i + 1 << " " << 8*i + 4 << " " << 8*i + 3 << " " << 8*i + 2 << std::endl;
                out << "f " << 8*i + 1 << " " << 8*i + 2 << " " << 8*i + 7 << " " << 8*i + 6 << std::endl;
                out << "f " << 8*i + 2 << " " << 8*i + 3 << " " << 8*i + 8 << " " << 8*i + 7 << std::endl;
                out << "f " << 8*i + 1 << " " << 8*i + 6 << " " << 8*i + 5 << " " << 8*i + 4 << std::endl;
                out << "f " << 8*i + 3 << " " << 8*i + 4 << " " << 8*i + 5 << " " << 8*i + 8 << std::endl;
                out << "f " << 8*i + 5 << " " << 8*i + 6 << " " << 8*i + 7 << " " << 8*i + 8 << std::endl;
            }
            out.close();
        }
    }
};


#endif //HW3_VOXELGRID_H
