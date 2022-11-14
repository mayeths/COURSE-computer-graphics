#pragma once
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include<cmath>
//定义Vertex顶点结构及Face三角面片结构
//********************Vector3D类用于定义向量结构及其有关的函数，其中定义了向量的加减，点乘，叉乘，除法，求长度及单位化等运算************************//

class Vector3D
{
public:
	double x, y, z;

	Vector3D() { x = 0; y = 0; z = 0; }
	Vector3D(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }

	Vector3D operator+ (Vector3D v)
	{
		return Vector3D(x + v.x, y + v.y, z + v.z);
	}
	Vector3D operator- (Vector3D v)
	{
		return Vector3D(x - v.x, y - v.y, z - v.z);
	}
	Vector3D operator* (double v)
	{
		return Vector3D(x*v, y*v, z*v);
	}
	Vector3D operator/(double v)
	{
		return Vector3D(x / v, y / v, z / v);
	}
	Vector3D operator^(Vector3D v)
	{
		Vector3D res;
		double xx = y*v.z - z*v.y;
		double yy = z*v.x - x*v.z;
		double zz = x*v.y - y*v.x;
		return Vector3D(xx, yy, zz);
	}
	Vector3D& operator+=(Vector3D v)
	{
		x += v.x; y += v.y; z += v.z;
		return *this;
	}
	Vector3D& operator*=(double v)
	{
		x *= v; y *= v; z *= v;
		return *this;
	}
	Vector3D& operator=(Vector3D v)
	{
		x = v.x; y = v.y; z = v.z;
		return *this;
	}
	double& operator[](unsigned int i)
	{
		switch (i)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		default: return x;
		}
	}
	double length()
	{
		return sqrt(x*x + y*y + z*z);
	}
	void normalize()
	{
		double len = length();
		x /= len;
		y /= len;
		z /= len;
	}
};

#endif