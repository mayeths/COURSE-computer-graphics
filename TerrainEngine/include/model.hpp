#pragma once
#include <SOIL.h>
#include<fstream>
#include<sstream> 
#include <algorithm>
#include "Vector3D.hpp"
#include <vector>
#include "log.h"

struct Vertex
{
    double x, y, z;
    double tx, ty;
};

struct Face
{
    int vertices[3];
    Vector3D normal;
};

class Model
{
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    Vertex center;
    double scaleD;
public:
    Model()
    {
        vertices.clear();
        faces.clear();
    }
    ~Model() {}

    bool LoadHeighMap(std::string fileName, int xscale, int yscale, int zscale);   //用于读取地形图的灰度值，并将其转换成对应点的高度信息
    void Render();
};

bool Model::LoadHeighMap(std::string fileName, int xscale, int yscale, int zscale)     //加载地形图  用于读取地形图的灰度值，并将其转换成对应点的高度信息
{
    unsigned char* heightData;
    int width, height, channels;

    heightData = SOIL_load_image(fileName.c_str(), &width, &height, &channels, SOIL_LOAD_L);
    if (!heightData) {
        log_error("SOIL error: %s", SOIL_last_result());
        return false;
    }
    channels = 1;

    Vertex curVertex;
    std::stringstream convStream;

    Vertex corner0, corner1;
    corner0.x = -1e30; corner0.y = -1e30; corner0.z = -1e30;
    corner1.x = 1e30; corner1.y = 1e30; corner1.z = 1e30;

    for (int i = 0; i < height; ++i)
    {
        convStream.clear();
        convStream << (i - height / 2)*zscale;
        double curZ;
        convStream >> curZ;
        for (int j = 0; j < width; ++j)
        {
            convStream.clear();
            convStream << (j - width / 2)*xscale;
            double curX;
            convStream >> curX;

            convStream.clear();
            double curY = heightData[channels*(i*width + j)] * 3;                         //顶点Y轴坐标（高度）

            curVertex.x = curX;
            curVertex.y = curY;
            curVertex.z = curZ;
            curVertex.tx = curX / (width*xscale) + 0.5;                                   //顶点纹理坐标
            curVertex.ty = 1 - curZ / (height*zscale) + 0.5;

            corner0.x = std::max(corner0.x, curVertex.x);
            corner0.y = std::max(corner0.y, curVertex.y);
            corner0.z = std::max(corner0.z, curVertex.z);
            corner1.x = std::min(corner1.x, curVertex.x);
            corner1.y = std::min(corner1.y, curVertex.y);
            corner1.z = std::min(corner1.z, curVertex.z);
            vertices.push_back(curVertex);
        }
    }

    for (int i = 0; i < height - 1; ++i)
    {
        for (int j = 0; j < width - 1; ++j)
        {
            int v00 = i*width + j;
            int v10 = v00 + 1;
            int v11 = v10 + width;
            int v01 = v11 - 1;

            Face curFace;
            curFace.vertices[0] = v00;               //网格上三角面顶点索引
            curFace.vertices[1] = v10;
            curFace.vertices[2] = v01;
            Vector3D vec0(vertices[v10].x - vertices[v00].x, vertices[v10].y - vertices[v00].y, vertices[v10].z - vertices[v00].z);
            Vector3D vec1(vertices[v01].x - vertices[v10].x, vertices[v01].y - vertices[v10].y, vertices[v01].z - vertices[v10].z);
            curFace.normal = (vec0 ^ vec1) * (1.0); curFace.normal.normalize();       //面法向量
            faces.push_back(curFace);


            curFace.vertices[0] = v01;              //网格下三角面顶点索引  
            curFace.vertices[1] = v10;
            curFace.vertices[2] = v11;
            Vector3D vec2(vertices[v10].x - vertices[v01].x, vertices[v10].y - vertices[v01].y, vertices[v10].z - vertices[v01].z);
            Vector3D vec3(vertices[v11].x - vertices[v10].x, vertices[v11].y - vertices[v10].y, vertices[v11].z - vertices[v10].z);
            curFace.normal = (vec2 ^ vec3) * (1.0); curFace.normal.normalize();
            faces.push_back(curFace);
        }
    }

    center.x = (corner0.x + corner1.x) / 2.0;
    center.y = (corner0.y + corner1.y) / 2.0;
    center.z = (corner0.z + corner1.z) / 2.0;
    scaleD = corner0.x - corner1.x;
    scaleD = std::max(scaleD, corner0.y - corner1.y);
    scaleD = std::max(scaleD, corner0.z - corner1.z);

    for (int i = 0; i < vertices.size(); ++i)
    {
        vertices[i].x = (vertices[i].x - center.x) / scaleD;
        vertices[i].y = (vertices[i].y - center.y) / scaleD;
        vertices[i].z = (vertices[i].z - center.z) / scaleD;
    }

    return true;
}
