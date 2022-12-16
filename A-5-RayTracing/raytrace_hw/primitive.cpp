#include"primitive.h"
#include<sstream>
#include<cstdio>
#include<string>
#include<cmath>
#include<iostream>
#include<cstdlib>
#define ran() ( double( rand() % 32768 ) / 32768 )

const int BEZIER_MAX_DEGREE = 5;
const int Combination[BEZIER_MAX_DEGREE + 1][BEZIER_MAX_DEGREE + 1] =
{	0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0,
	1, 2, 1, 0, 0, 0,
	1, 3, 3, 1, 0, 0,
	1, 4, 6, 4, 1, 0,
	1, 5, 10,10,5, 1
};

const int MAX_COLLIDE_TIMES = 10;
const int MAX_COLLIDE_RANDS = 10;


std::pair<double, double> ExpBlur::GetXY()
{
	double x,y;
	x = ran();
	x = pow(2, x)-1;
	y = rand();
	return std::pair<double, double>(x*cos(y),x*sin(y));
}

Material::Material() {
	color = absor = Color();
	refl = refr = 0;
	diff = spec = 0;
	rindex = 0;
	drefl = 0;
	texture = NULL;
	blur = new ExpBlur();
}

void Material::Input( std::string var , std::stringstream& fin ) {
	if ( var == "color=" ) color.Input( fin );
	if ( var == "absor=" ) absor.Input( fin );
	if ( var == "refl=" ) fin >> refl;
	if ( var == "refr=" ) fin >> refr;
	if ( var == "diff=" ) fin >> diff;
	if ( var == "spec=" ) fin >> spec;
	if ( var == "drefl=" ) fin >> drefl;
	if ( var == "rindex=" ) fin >> rindex;
	if ( var == "texture=" ) {
		std::string file; fin >> file;
		texture = new Bmp;
		texture->Input( file );
	}
	if ( var == "blur=" ) {
		std::string blurname; fin >> blurname;
		if(blurname == "exp")
			blur = new ExpBlur();
	}
}

Primitive::Primitive() {
	sample = rand();
	material = new Material;
	next = NULL;
}

Primitive::Primitive( const Primitive& primitive ) {
	*this = primitive;
	material = new Material;
	*material = *primitive.material;
}

Primitive::~Primitive() {
	delete material;
}

void Primitive::Input( std::string var , std::stringstream& fin ) {
	material->Input( var , fin );
}

Sphere::Sphere() : Primitive() {
	De = Vector3( 0 , 0 , 1 );
	Dc = Vector3( 0 , 1 , 0 );
}

void Sphere::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "De=" ) De.Input( fin );
	if ( var == "Dc=" ) Dc.Input( fin );
	Primitive::Input( var , fin );
}

CollidePrimitive Sphere::Collide( Vector3 ray_O , Vector3 ray_V ) {
	ray_V = ray_V.GetUnitVector();
	Vector3 P = ray_O - O;
	double b = -P.Dot( ray_V );
	double det = b * b - P.Module2() + R * R;
	CollidePrimitive ret;

	if ( det > EPS ) {
		det = sqrt( det );
		double x1 = b - det  , x2 = b + det;

		if ( x2 < EPS ) return ret;
		if ( x1 > EPS ) {
			ret.dist = x1;
			ret.front = true;
		} else {
			ret.dist = x2;
			ret.front = false;
		} 
	} else 
		return ret;

	ret.C = ray_O + ray_V * ret.dist;
	ret.N = ( ret.C - O ).GetUnitVector();
	if ( ret.front == false ) ret.N = -ret.N;
	ret.isCollide = true;
	ret.collide_primitive = this;
	return ret;
}

Color Sphere::GetTexture(Vector3 crash_C) {
	Vector3 I = ( crash_C - O ).GetUnitVector();
	double a = acos( -I.Dot( De ) );
	double b = acos( std::min( std::max( I.Dot( Dc ) / sin( a ) , -1.0 ) , 1.0 ) );
	double u = a / PI , v = b / 2 / PI;
	if ( I.Dot( Dc * De ) < 0 ) v = 1 - v;
	return material->texture->GetSmoothColor( u , v );
}


void Plane::Input( std::string var , std::stringstream& fin ) {
	if ( var == "N=" ) N.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Primitive::Input( var , fin );
}

CollidePrimitive Plane::Collide( Vector3 ray_O , Vector3 ray_V ) {
	// 见lecture9-光线跟踪P26
	// 带入求解
	//   1. 计算是否相交
	//   2. 计算交点位置
	ray_V = ray_V.GetUnitVector();
	N = N.GetUnitVector(); // 法线Normal
	double d = N.Dot( ray_V ); // 如果平面的法线N和光线方向ray_V垂直，则dot结果小于精度误差甚至为0，则光线与平面平行，不Collide。
	CollidePrimitive ret;
	if ( fabs( d ) < EPS ) return ret; // 返回默认的isCollide=false
	double l = ( N * R - ray_O ).Dot( N ) / d; // 否则，l是距离distance
	if ( l < EPS ) return ret;

	ret.dist = l; //光源与平面交点的距离
	ret.front = ( d < 0 ); //在正面还是反面
	ret.C = ray_O + ray_V * ret.dist; // 交点，光源位置+光线方向*距离
	ret.N = ( ret.front ) ? N : -N;
	ret.isCollide = true;
	ret.collide_primitive = this; // 撞上我这个平面了
	return ret;
}

Color Plane::GetTexture(Vector3 crash_C) {
	double u = crash_C.Dot( Dx ) / Dx.Module2();
	double v = crash_C.Dot( Dy ) / Dy.Module2();
	return material->texture->GetSmoothColor( u , v );
}

void Square::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Primitive::Input( var , fin );
}

CollidePrimitive Square::Collide( Vector3 ray_O , Vector3 ray_V ) {
	// Square可用：Vector3 O , Dx , Dy
	// O是位置，Dx应该是从O开始的x方向（大小应该也是边长），Dy应该是从O开始的y方向（大小应该也是边长）
	// 判断方法：借鉴平面Plane::Collide那样先判断是否与平面相交，然后由PPT P30确定交点是否在多边形内（这种网上有，弧长法）。长方形的判断更直接简单一点
	CollidePrimitive ret;
	ret.isCollide = false;

	Vector3 rayDirectionUnitVector = ray_V.GetUnitVector();
	Vector3 Normal = this->Dx * this->Dy; // 法线Normal
	Vector3 NormalUnitVector = Normal.GetUnitVector();
	double NDdot = NormalUnitVector.Dot(rayDirectionUnitVector);
	bool isRayPlaneParallel = fabs(NDdot) < EPS;
	if (isRayPlaneParallel)
		return ret;
	double l = (O - ray_O).Dot(NormalUnitVector) / NDdot;
	if ( l < EPS )
		return ret;

	Vector3 CollidePosition = ray_O + rayDirectionUnitVector * l;
	bool overX = fabs(Dx.Dot(CollidePosition - O)) > Dx.Dot(Dx);
	bool overY = fabs(Dy.Dot(CollidePosition - O)) > Dy.Dot(Dy);
	if (overX || overY)
		return ret; // 不在这个四边形内

	ret.dist = l; //光源与平面上的交点的距离
	ret.front = (NDdot < 0); // 交点在正面还是反面
	ret.C = CollidePosition; // 平面上的交点
	ret.N = (ret.front) ? NormalUnitVector : -NormalUnitVector;
	ret.isCollide = true;
	ret.collide_primitive = this;

	return ret;
}

Color Square::GetTexture(Vector3 crash_C) {
	double u = (crash_C - O).Dot( Dx ) / Dx.Module2() / 2 + 0.5;
	double v = (crash_C - O).Dot( Dy ) / Dy.Module2() / 2 + 0.5;
	return material->texture->GetSmoothColor( u , v );
}

void Cylinder::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O1=" ) O1.Input( fin );
	if ( var == "O2=" ) O2.Input( fin );
	if ( var == "R=" ) fin>>R; 
	Primitive::Input( var , fin );
}

struct DCylinder {
	Vector3 position;
	Vector3 axis;
	double radius;
};

// https://stackoverflow.com/a/9837645/11702338
// 虽然对接了，但没使用。该算法没有提供description
bool d3RayCylinderIntersection(const DCylinder &cylinder,const Vector3 &org,const Vector3 &dir,double &lambda,Vector3 &normal,Vector3 &newPosition)
// Ray and cylinder intersection
// If hit, returns true and the intersection point in 'newPosition' with a normal and distance along
// the ray ('lambda')
{
  Vector3 RC;
  double d;
  double t,s;
  Vector3 n,D,O;
  double ln;
  double in,out;

  RC=org;
  RC -= cylinder.position;
  n = dir * cylinder.axis;

  ln=n.Module();

  // Parallel? (?)
	if ( fabs( ln ) < EPS ) return false;

  n = n.GetUnitVector();

  d=fabs(RC.Dot(n));

  if (d<=cylinder.radius)
  {
	O = RC * cylinder.axis;
	//TVector::cross(RC,cylinder._Axis,O);
	t = -O.Dot(n)/ln;
	//TVector::cross(n,cylinder._Axis,O);
	O = n * cylinder.axis;
	O = O.GetUnitVector();
	s=fabs( sqrtf(cylinder.radius*cylinder.radius-d*d) / O.Dot(dir) );

	in=t-s;
	out=t+s;

	if (in < -EPS)
	{
	  if(out < -EPS)
		return false;
	  else lambda=out;
	} else if(out < -EPS)
	{
	  lambda=in;
	} else if(in<out)
	{
	  lambda=in;
	} else
	{
	  lambda=out;
	}

	// Calculate intersection point
	newPosition=org;
	newPosition.x+=dir.x*lambda;
	newPosition.y+=dir.y*lambda;
	newPosition.z+=dir.z*lambda;
	Vector3 HB;
	HB=newPosition;
	HB -= cylinder.position;

	double scale = HB.Dot(cylinder.axis);
	normal.x=HB.x-cylinder.axis.x*scale;
	normal.y=HB.y-cylinder.axis.y*scale;
	normal.z=HB.z-cylinder.axis.z*scale;
	normal = normal.GetUnitVector();
	return true;
  }

  return false;
}


CollidePrimitive Cylinder::SideFaceCollide( Vector3 ray_O , Vector3 ray_V ) {
	Vector3 V = ray_V.GetUnitVector();
	Vector3 D = (O2 - O1).GetUnitVector();
	Vector3 P = ray_O - O1;

	Vector3 t1 = V - V.Dot(D) * D;
	Vector3 t2 = P - P.Dot(D) * D;
	double a = t1.Module2();
	double b = t1.Dot(t2);
	double c = t2.Module2() - R * R;
	double det = b * b - a * c;

	CollidePrimitive ret;
	ret.isCollide = false;
	if (det <= EPS)
		return ret;

	det = sqrt(det) / a;
	double x1 = - (b / a) - det; // -b +- sqrt(b^2 - 4ac)
	double x2 = - (b / a) + det;
	if (x2 < EPS)
		return ret;

	auto test_back = [&]() {
		bool O1no = D.Dot(ret.C - O1) < EPS;
		bool O2yes = D.Dot(ret.C - O2) > EPS;
		return O1no || O2yes;
	};

	if (x1 > EPS) {
		ret.front = true;
		ret.dist = x1;
		ret.C = ray_O + V * ret.dist;
		if (test_back()) {
			ret.front = false;
			ret.dist = x2;
			ret.C = ray_O + V * ret.dist;
			if (test_back())
				return ret;
		}
	} else {
		ret.front = false;
		ret.dist = x2;
		ret.C = ray_O + V * ret.dist;
		if (test_back())
			return ret;
	}

	Vector3 O1C = ret.C - O1;
	Vector3 Normal = O1C - O1C.Dot(D) * D;
	ret.N = Normal.GetUnitVector();
	if ( ret.front == false )
		ret.N = -ret.N;
	ret.isCollide = true;
	ret.collide_primitive = this;

	return ret;
}

#define NUMBER_SIGN(x) ((x) > 0) - ((x) < 0)

CollidePrimitive Cylinder::BaseFaceCollide( Vector3 ray_O , Vector3 ray_V, Cylinder::Face face) {
	CollidePrimitive ret;
	ret.isCollide = false;

	ray_V = ray_V.GetUnitVector();
	bool isTopFace = face == Cylinder::Face::TOP_FACE;
	Vector3 O = isTopFace ? O1 : O2;
	Vector3 D = (O2 - O1).GetUnitVector();

	double d = D.Dot(ray_V);
	if (fabs(d) < EPS)
		return ret;

	ret.dist = D.Dot(O - ray_O) / d;
	if (ret.dist < EPS)
		return ret;

	ret.C = ray_O + ret.dist * ray_V;
	if ((ret.C - O).Module2() - R * R > EPS)
		return ret;

	int front = NUMBER_SIGN(d) * (isTopFace ? 1 : -1);
	ret.front = (front > 0);
	ret.N = front * D;

	ret.isCollide = true;
	ret.collide_primitive = this;
	
	return ret;
}


CollidePrimitive Cylinder::Collide( Vector3 ray_O , Vector3 ray_V ) {
	CollidePrimitive p0 = this->SideFaceCollide(ray_O, ray_V);
	CollidePrimitive p1 = this->BaseFaceCollide(ray_O, ray_V, Cylinder::Face::TOP_FACE);
	CollidePrimitive p2 = this->BaseFaceCollide(ray_O, ray_V, Cylinder::Face::BOTTOM_FACE);

	CollidePrimitive ret;
	if (p0.isCollide && p0.dist < ret.dist) ret = p0;
	if (p1.isCollide && p1.dist < ret.dist) ret = p1;
	if (p2.isCollide && p2.dist < ret.dist) ret = p2;
	return ret;
}

Color Cylinder::GetTexture(Vector3 crash_C) {
	Vector3 delta = crash_C - O1;
	Vector3 N = (O2 - O1).GetUnitVector();
	Vector3 Nx = N.GetAnVerticalVector();
	Vector3 Ny = N * Nx;
	double u = (delta.Dot(N)) / (O2 - O1).Module();
	double theta = atan2(delta.Dot(Ny), delta.Dot(Nx));
	return material->texture->GetSmoothColor(std::fmod(theta, 2*PI) , u);
}

void Bezier::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O1=" ) O1.Input( fin );
	if ( var == "O2=" ) O2.Input( fin );
	if ( var == "P=" ) {
		degree++;
		double newR, newZ;
		fin>>newZ>>newR;
		R.push_back(newR);
		Z.push_back(newZ);
	}
	if ( var == "Cylinder" ) {
		double maxR = 0;
		for(int i=0;i<R.size();i++)
			if(R[i] > maxR)
				maxR = R[i];
		boundingCylinder = new Cylinder(O1, O2, maxR);
		N = (O1 - O2).GetUnitVector();
		Nx = N.GetAnVerticalVector();
		Ny = N * Nx;
	}
	Primitive::Input( var , fin );
}

// 找不到简单易懂的资料
CollidePrimitive Bezier::Collide( Vector3 ray_O , Vector3 ray_V ) {
	CollidePrimitive ret;
	//NEED TO IMPLEMENT
	return ret;
}

Color Bezier::GetTexture(Vector3 crash_C) {
	double u = 0.5 ,v = 0.5;
	//NEED TO IMPLEMENT
	return material->texture->GetSmoothColor( u , v );
}

